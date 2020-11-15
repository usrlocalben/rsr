#include "src/rcl/rclx/rclx_gason_util.hxx"
#include "src/rgl/rglv/rglv_math.hxx"
#include "src/rml/rmlm/rmlm_mat4.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"
#include "src/viewer/compile.hxx"
#include "src/viewer/node/i_camera.hxx"
#include "src/viewer/node/i_value.hxx"

#include <memory>
#include <string>
#include <string_view>
#include <utility>

namespace {

using namespace rqdq;
using namespace rqv;
namespace jobsys = rclmt::jobsys;

class Impl : public ICamera {

	IValue* positionNode_{nullptr};
	std::string positionSlot_{};
	float ha;
	float va;
	float fov;
	rmlv::vec2 origin;

public:
	Impl(std::string_view id, InputList inputs, float ha, float va, float fov, rmlv::vec2 origin) :
		ICamera(id, std::move(inputs)),
		ha(ha),
		va(va),
		fov(fov),
		origin(origin) {}

	auto Connect(std::string_view attr, NodeBase* other, std::string_view slot) -> bool override {
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

	auto ProjectionMatrix(float aspect) const -> rmlm::mat4 override {
		rmlm::mat4 m = rglv::Perspective2(fov, aspect, 10, 1000);
		m = rmlm::mat4::translate(origin.x, origin.y, 0) * m;
		return m; }

	auto ViewMatrix() const -> rmlm::mat4 override {
		rmlv::vec3 position = positionNode_->Eval(positionSlot_).as_vec3();
		rmlv::vec3 dir = MakeDir();
		rmlv::vec3 right = MakeRight();
		rmlv::vec3 up = cross(right, dir);
		return rglv::LookAt(position, position + dir, up); }

private:
	auto MakeDir() const -> rmlv::vec3 {
		return { cosf(va)*sinf(ha), sinf(va), cosf(va)*cosf(ha) }; }

	auto MakeRight() const -> rmlv::vec3 {
		return { sinf(ha-3.14F/2.0F), 0.0F, cosf(ha-3.14F/2.0F) }; }};


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


}  // close unnamed namespace
