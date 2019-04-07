#include "camera.hxx"

#include <memory>
#include <string_view>

#include "src/rcl/rclx/rclx_gason_util.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"
#include "src/viewer/compile.hxx"
#include "src/viewer/node/value.hxx"


namespace rqdq {
namespace {

using namespace rqv;

class ManCamNode : public CameraNode {
public:
	ManCamNode(std::string_view id, InputList inputs, float ha, float va, float fov, rmlv::vec2 origin)
		:CameraNode(id, std::move(inputs)), ha(ha), va(va), fov(fov), origin(origin) {}

	void Connect(std::string_view attr, NodeBase* other, std::string_view slot) override;

	rmlm::mat4 GetProjectionMatrix(float aspect) const override{
		rmlm::mat4 m = rglv::make_gluPerspective(fov, aspect, 1, 1000);
		m = rmlm::mat4::translate(origin.x, origin.y, 0) * m;
		return m; }

	rmlm::mat4 GetModelViewMatrix() const override{
		rmlv::vec3 position = positionNode_->Get(positionSlot_).as_vec3();
		rmlv::vec3 dir = MakeDir();
		rmlv::vec3 right = MakeRight();
		rmlv::vec3 up = cross(right, dir);
		return rglv::look_at(position, position + dir, up); }

private:
	rmlv::vec3 MakeDir() const {
		return { cosf(va)*sinf(ha), sinf(va), cosf(va)*cosf(ha) }; }

	rmlv::vec3 MakeRight() const {
		return { sinf(ha-3.14F/2.0F), 0.0F, cosf(ha-3.14F/2.0F) }; }

private:
	ValuesBase* positionNode_{nullptr};
	std::string positionSlot_{};
	float ha;
	float va;
	float fov;
	rmlv::vec2 origin; };


class OrthographicNode final : public CameraNode {
public:
	OrthographicNode(std::string_view id, InputList inputs)
		:CameraNode(id, std::move(inputs)) {}

	rmlm::mat4 GetProjectionMatrix(float aspect) const override {
		return rglv::make_glOrtho(-1.0F, 1.0F, -1.0F, 1.0F, 1.0F, -1.0F); }

	rmlm::mat4 GetModelViewMatrix() const override {
		return rmlm::mat4::ident(); }};

void ManCamNode::Connect(std::string_view attr, NodeBase* other, std::string_view slot) {
	if (attr == "position") {
		positionNode_ = static_cast<ValuesBase*>(other);
		positionSlot_ = slot; }
	else {
		CameraNode::Connect(attr, other, slot); }}


class PerspectiveCompiler final : public NodeCompiler {
	void Build() override {
		using rclx::jv_find;
		if (!Input("ValuesBase", "position", /*required=*/true)) { return; }

		float ha{3.14F};
		if (auto jv = jv_find(data_, "h", JSON_NUMBER)) {
			ha = static_cast<float>(jv->toNumber()); }

		float va{0.0F};
		if (auto jv = jv_find(data_, "v", JSON_NUMBER)) {
			va = static_cast<float>(jv->toNumber()); }

		float fov{45.0F};
		if (auto jv = jv_find(data_, "fov", JSON_NUMBER)) {
			fov = static_cast<float>(jv->toNumber()); }

		rmlv::vec2 origin{0.0F};
		if (auto jv = jv_find(data_, "originX", JSON_NUMBER)) {
			origin.x = static_cast<float>(jv->toNumber()); }
		if (auto jv = jv_find(data_, "originY", JSON_NUMBER)) {
			origin.y = static_cast<float>(jv->toNumber()); }
		out_ = std::make_shared<ManCamNode>(id_, std::move(inputs_), ha, va, fov, origin); }};


class OrthographicCompiler final : public NodeCompiler {
	void Build() override {
		out_ = std::make_shared<OrthographicNode>(id_, std::move(inputs_)); }};

PerspectiveCompiler perspectiveCompiler{};

OrthographicCompiler orthographicCompiler{};

struct init { init() {
	NodeRegistry::GetInstance().Register(NodeInfo{
		"ICamera",
		"CameraNode",
		[](NodeBase* node) { return dynamic_cast<CameraNode*>(node) != nullptr; },
		nullptr });
	NodeRegistry::GetInstance().Register(NodeInfo{
		"$perspective",
		"Perspective",
		[](NodeBase* node) { return dynamic_cast<ManCamNode*>(node) != nullptr; },
		&perspectiveCompiler });
	NodeRegistry::GetInstance().Register(NodeInfo{
		"$orthographic",
		"Orthographic",
		[](NodeBase* node) { return dynamic_cast<OrthographicNode*>(node) != nullptr; },
		&orthographicCompiler });
}} init{};


}  // namespace
}  // namespace rqdq
