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

	// config
	const bool takeShadows_;
	const bool makeShadows_;
	rglv::VertexArray_F3F3F3 meshVAO_;
	rcls::vector<uint16_t> meshIndices_;

	// inputs
	IMaterial* materialNode_{nullptr};

public:
	Impl(std::string_view id, InputList inputs, const rglv::Mesh& mesh, bool ts, bool ms) :
		IGl(id, std::move(inputs)),
		takeShadows_(ts),
		makeShadows_(ms) {
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
		if (!makeShadows_) {
			return; }
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
		if (materialNode_ != nullptr) {
			materialNode_->Apply(_dc); }
		else if (takeShadows_ && (lights.cnt > 0)) {
			dc.UseProgram(9);  // obj color w/per-pixel light w/shadow-map

			auto [id, vptr] = dc.AllocUniformBuffer();
			auto* ptr = static_cast<OBJ2SProgram::UniformsSD*>(vptr);
			dc.UseUniforms(id);

			/* XXX should be able to put all of the shadow-map calc in this matrix
			rmlm::mat4 ndcToUV{
				.5F,   0, 0, 0,
				  0, .5F, 0, 0,
				  0,   0, 1, 0,
				.5F, .5F, 0, 1 };
			ptr->modelToShadow = (ndcToUV * lights.pmat[0] * lights.vmat[0] * *mmat); */

			ptr->modelToShadow = (lights.pmat[0] * lights.vmat[0] * *mmat);
			ptr->ldir = lights.dir[0];
			ptr->lpos = lights.pos[0];
			ptr->lcos = lights.cos[0];
			dc.BindTexture3(lights.map[0], lights.size[0]);
		}
		else {
			// obj color w/per-pixel light, no shadow-map
			dc.UseProgram(8); }

		dc.ViewMatrix(*vmat * *mmat);
		dc.ProjectionMatrix(*pmat);

		dc.UseBuffer(0, meshVAO_);
		dc.DrawElements(GL_TRIANGLES, static_cast<int>(meshIndices_.size()), GL_UNSIGNED_SHORT, meshIndices_.data()); }};


class Compiler final : public NodeCompiler {
	void Build() override {
		if (!Input("material", /*required=*/true)) { return; }

		std::string_view name{"notfound.obj"};
		if (auto jv = rclx::jv_find(data_, "name", JSON_STRING)) {
			name = jv->toString(); }
		const auto& mesh = meshStore_->get(name);

		bool takeShadows{true};
		if (auto jv = rclx::jv_find(data_, "takeShadows", JSON_FALSE)) {
			takeShadows = false; }
		bool makeShadows{true};
		if (auto jv = rclx::jv_find(data_, "makeShadows", JSON_FALSE)) {
			makeShadows = false; }

		out_ = std::make_shared<Impl>(id_, std::move(inputs_), mesh, takeShadows, makeShadows); }};


struct init { init() {
	NodeRegistry::GetInstance().Register("$mesh", [](){ return std::make_unique<Compiler>(); });
}} init{};


}  // namespace
}  // namespace rqdq
