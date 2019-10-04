#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <utility>

#include "src/rcl/rcls/rcls_aligned_containers.hxx"
#include "src/rcl/rclx/rclx_gason_util.hxx"
#include "src/rml/rmlm/rmlm_mat4.hxx"
#include "src/rgl/rglv/rglv_mesh.hxx"
#include "src/viewer/compile.hxx"
#include "src/viewer/shaders.hxx"
#include "src/viewer/node/base.hxx"
#include "src/viewer/node/i_gl.hxx"
#include "src/viewer/node/i_material.hxx"

namespace rqdq {
namespace {

using namespace rqv;

class Impl final : public IGl {
public:
	Impl(std::string_view id, InputList inputs, const rglv::Mesh& mesh)
		:IGl(id, std::move(inputs)) {
		std::tie(meshVAO_, meshIndices_) = rglv::make_indexed_vao_F3F3F3(mesh); }

	bool Connect(std::string_view attr, NodeBase* other, std::string_view slot) override {
		if (attr == "material") {
			materialNode_ = dynamic_cast<IMaterial*>(other);
			if (materialNode_ == nullptr) {
				TYPE_ERROR(IMaterial);
				return false; }
			return true; }
		return IGl::Connect(attr, other, slot); }

	void AddDeps() override {
		AddDep(materialNode_); }

	void Draw(rglv::GL* _dc, const rmlm::mat4* pmat, const rmlm::mat4* mvmat, rclmt::jobsys::Job* link, int depth) override {
		using namespace rglv;
		auto& dc = *_dc;
		std::lock_guard<std::mutex> lock(dc.mutex);
		if (materialNode_ != nullptr) {
			materialNode_->Apply(_dc); }

		auto [id, ptr] = dc.glReserveUniformBuffer<rglv::BaseProgram::UniformsSD>();
		ptr->pm = *pmat;
		ptr->mvm = *mvmat;
		ptr->nm = transpose(inverse(ptr->mvm));
		ptr->mvpm = ptr->pm * ptr->mvm;
		dc.glUniforms(id);

		dc.glUseArray(meshVAO_);
		dc.glDrawElements(GL_TRIANGLES, meshIndices_.size(), GL_UNSIGNED_SHORT, meshIndices_.data());
		if (link != nullptr) {
			rclmt::jobsys::run(link); }}

private:
	// config
	rglv::VertexArray_F3F3F3 meshVAO_;
	rcls::vector<uint16_t> meshIndices_;

	// inputs
	IMaterial* materialNode_{nullptr}; };


class Compiler final : public NodeCompiler {
	void Build() override {
		if (!Input("material", /*required=*/true)) { return; }

		std::string_view name{"notfound.obj"};
		if (auto jv = rclx::jv_find(data_, "name", JSON_STRING)) {
			name = jv->toString(); }
		const auto& mesh = meshStore_->get(name);

		out_ = std::make_shared<Impl>(id_, std::move(inputs_), mesh); }};


struct init { init() {
	NodeRegistry::GetInstance().Register("$fxFoo", [](){ return std::make_unique<Compiler>(); });
}} init{};


}  // namespace
}  // namespace rqdq
