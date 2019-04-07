#include <memory>
#include <string_view>

#include "src/rcl/rclx/rclx_gason_util.hxx"
#include "src/rgl/rglv/rglv_mesh.hxx"
#include "src/viewer/compile.hxx"
#include "src/viewer/node/gl.hxx"
#include "src/viewer/node/base.hxx"
#include "src/viewer/node/material.hxx"
#include "src/viewer/shaders.hxx"

namespace rqdq {
namespace {

using namespace rqv;

class FxFoo final : public GlNode {
public:
	FxFoo(std::string_view id, InputList inputs, const rglv::Mesh& mesh)
		:GlNode(id, std::move(inputs)) {
		std::tie(meshVAO_, meshIndices_) = rglv::make_indexed_vao_F3F3F3(mesh); }

	void Connect(std::string_view /*attr*/, NodeBase* other, std::string_view slot) override;

	void AddDeps() override;

	void Draw(rglv::GL* _dc, const rmlm::mat4* pmat, const rmlm::mat4* mvmat, rclmt::jobsys::Job* link, int depth) override;

private:
	// config
	rglv::VertexArray_F3F3F3 meshVAO_;
	rcls::vector<uint16_t> meshIndices_;

	// inputs
	MaterialNode* materialNode_{nullptr}; };


void FxFoo::Connect(std::string_view attr, NodeBase* other, std::string_view slot) {
	if (attr == "material") {
		materialNode_ = static_cast<MaterialNode*>(other); }
	else {
		GlNode::Connect(attr, other, slot); }}


void FxFoo::AddDeps() {
	AddDep(materialNode_); }


void FxFoo::Draw(rglv::GL* _dc, const rmlm::mat4* const pmat, const rmlm::mat4* const mvmat, rclmt::jobsys::Job* link, int depth) {
	using namespace rglv;
	auto& dc = *_dc;
	std::lock_guard<std::mutex> lock(dc.mutex);
	if (materialNode_ != nullptr) {
		materialNode_->Apply(_dc); }
	dc.glMatrixMode(GL_PROJECTION);
	dc.glLoadMatrix(*pmat);
	dc.glMatrixMode(GL_MODELVIEW);
	dc.glLoadMatrix(*mvmat);
	dc.glUseArray(meshVAO_);
	dc.glDrawElements(GL_TRIANGLES, meshIndices_.size(), GL_UNSIGNED_SHORT, meshIndices_.data());
	if (link != nullptr) {
		rclmt::jobsys::run(link); }}


class Compiler final : public NodeCompiler {
	void Build() override {
		if (!Input("MaterialNode", "material", /*required=*/true)) { return; }

		std::string_view name{"notfound.obj"};
		if (auto jv = rclx::jv_find(data_, "name", JSON_STRING)) {
			name = jv->toString(); }
		const auto& mesh = meshStore_->get(name);

		out_ = std::make_shared<FxFoo>(id_, std::move(inputs_), mesh); }};

Compiler compiler{};

struct init { init() {
	NodeRegistry::GetInstance().Register(NodeInfo{
		"$fxFoo",
		"FxFoo",
		[](NodeBase* node) { return dynamic_cast<FxFoo*>(node) != nullptr; },
		&compiler });
}} init{};


}  // namespace
}  // namespace rqdq
