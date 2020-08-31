#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <utility>

#include "src/rcl/rcls/rcls_aligned_containers.hxx"
#include "src/rcl/rclx/rclx_gason_util.hxx"
#include "src/rml/rmlm/rmlm_mat4.hxx"
#include "src/rgl/rglv/rglv_mesh.hxx"
#include "src/rgl/rglv/rglv_mesh_util.hxx"
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
		rglv::MakeArray(mesh, "PND", meshVAO_, meshIndices_); }

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

	void DrawDepth(rglv::GL* _dc, const rmlm::mat4* pmat, const rmlm::mat4* mvmat) override {
		using namespace rglv;
		auto& dc = *_dc;
		std::lock_guard lock(dc.mutex);

		dc.ViewMatrix(*mvmat);
		dc.ProjectionMatrix(*pmat);

		dc.UseBuffer(0, meshVAO_);
		dc.DrawElements(GL_TRIANGLES, static_cast<int>(meshIndices_.size()), GL_UNSIGNED_SHORT, meshIndices_.data()); }

	void Draw(int pass, const LightPack& lights [[maybe_unused]], rglv::GL* _dc, const rmlm::mat4* pmat, const rmlm::mat4* vmat, const rmlm::mat4* mmat) override {
		using namespace rglv;
		auto& dc = *_dc;
		if (pass != 1) return;
		std::lock_guard lock(dc.mutex);
		/*if (materialNode_ != nullptr) {
			materialNode_->Apply(_dc); }*/

		dc.UseProgram(9);

		dc.ViewMatrix(*vmat * *mmat);
		dc.ProjectionMatrix(*pmat);
		auto [id, ptr] = dc.AllocUniformBuffer<OBJ2SProgram::UniformsSD>();
		dc.UseUniforms(id);

		/*const rmlm::mat4 ndcToUV{
			.5F,   0, 0, 0,
			  0, .5F, 0, 0,
			  0,   0, 1, 0,
			.5F, .5F, 0, 1
		};
		ptr->modelToShadow = (ndcToUV * lights.pmat[0] * lights.vmat[0] * *mmat);
		*/

		ptr->modelToShadow = (lights.pmat[0] * lights.vmat[0] * *mmat);
		ptr->ldir = lights.dir[0];
		ptr->lpos = lights.pos[0];
		ptr->lcos = lights.cos[0];

		dc.BindTexture3(lights.map[0], lights.size[0]);

		dc.UseBuffer(0, meshVAO_);
		dc.DrawElements(GL_TRIANGLES, static_cast<int>(meshIndices_.size()), GL_UNSIGNED_SHORT, meshIndices_.data()); }

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
