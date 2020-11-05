#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include "src/rcl/rclx/rclx_gason_util.hxx"
#include "src/rgl/rglv/rglv_math.hxx"
#include "src/rml/rmlm/rmlm_mat4.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"
#include "src/viewer/compile.hxx"
#include "src/viewer/node/i_camera.hxx"
#include "src/viewer/node/i_value.hxx"

namespace rqdq {
namespace {

using namespace rqv;

class Impl : public ICamera {
public:
	Impl(std::string_view id, InputList inputs, float ha, float va, float fov, rmlv::vec2 origin)
		:ICamera(id, std::move(inputs)), ha(ha), va(va), fov(fov), origin(origin) {}

	bool Connect(std::string_view attr, NodeBase* other, std::string_view slot) override {
		if (attr == "position") {
			positionNode_ = dynamic_cast<IValue*>(other);
			positionSlot_ = slot;
			if (positionNode_ == nullptr) {
				TYPE_ERROR(IValue);
				return false; }
			return true; }
		return ICamera::Connect(attr, other, slot); }

	void DisconnectAll() override {
		ICamera::DisconnectAll();
		positionNode_ = nullptr; }

	rmlm::mat4 ProjectionMatrix(float aspect) const override {
		rmlm::mat4 m = rglv::Perspective2(fov, aspect, 10, 1000);
		m = rmlm::mat4::translate(origin.x, origin.y, 0) * m;
		return m; }

	rmlm::mat4 ViewMatrix() const override {
		rmlv::vec3 position = positionNode_->Eval(positionSlot_).as_vec3();
		rmlv::vec3 dir = MakeDir();
		rmlv::vec3 right = MakeRight();
		rmlv::vec3 up = cross(right, dir);
		return rglv::LookAt(position, position + dir, up); }

private:
	rmlv::vec3 MakeDir() const {
		return { cosf(va)*sinf(ha), sinf(va), cosf(va)*cosf(ha) }; }

	rmlv::vec3 MakeRight() const {
		return { sinf(ha-3.14F/2.0F), 0.0F, cosf(ha-3.14F/2.0F) }; }

private:
	IValue* positionNode_{nullptr};
	std::string positionSlot_{};
	float ha;
	float va;
	float fov;
	rmlv::vec2 origin; };


class Compiler final : public NodeCompiler {
	void Build() override {
		using rclx::jv_find;
		if (!Input("position", /*required=*/true)) { return; }

		auto ha = DataReal("h", 3.14F);
		auto va = DataReal("v", 0.0F);
		auto fov = DataReal("fov", 45.0F);

		auto ox = DataReal("originX", 0.0F);
		auto oy = DataReal("originY", 0.0F);
		rmlv::vec2 origin{ ox, oy };

		out_ = std::make_shared<Impl>(id_, std::move(inputs_), ha, va, fov, origin); }};



struct init { init() {
	NodeRegistry::GetInstance().Register("$perspective", [](){ return std::make_unique<Compiler>(); });
}} init{};


}  // namespace
}  // namespace rqdq
