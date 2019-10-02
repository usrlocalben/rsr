#include <iostream>
#include <memory>
#include <mutex>
#include <string_view>
#include <utility>

#include "src/rcl/rclx/rclx_gason_util.hxx"
#include "src/rgl/rglv/rglv_gl.hxx"
#include "src/rgl/rglv/rglv_mesh.hxx"
#include "src/rml/rmlm/rmlm_mat4.hxx"
#include "src/viewer/compile.hxx"
#include "src/viewer/shaders.hxx"
#include "src/viewer/node/base.hxx"
#include "src/viewer/node/i_gl.hxx"
#include "src/viewer/node/i_material.hxx"
#include "src/viewer/node/i_value.hxx"

namespace rqdq {
namespace {

using namespace rqv;

//PixelToaster::Timer ttt;

class Impl final : public IGl {
public:
	Impl(std::string_view id, InputList inputs, const rglv::Mesh& mesh) :
		IGl(id, std::move(inputs)),
		src_(mesh),
		dst_(mesh) {}

	bool Connect(std::string_view attr, NodeBase* other, std::string_view slot) override {
		if (attr == "material") {
			materialNode_ = dynamic_cast<IMaterial*>(other);
			if (materialNode_ == nullptr) {
				TYPE_ERROR(IMaterial);
				return false; }
			return true; }
		if (attr == "freq") {
			freqNode_ = dynamic_cast<IValue*>(other);
			freqSlot_ = slot;
			if (freqNode_ == nullptr) {
				TYPE_ERROR(IValue);
				return false; }
			return true; }
		if (attr == "phase") {
			phaseNode_ = dynamic_cast<IValue*>(other);
			phaseSlot_ = slot;
			if (phaseNode_ == nullptr) {
				TYPE_ERROR(IValue);
				return false; }
			return true; }
		return IGl::Connect(attr, other, slot); }

	void AddDeps() override {
		AddDep(materialNode_);
		AddDep(freqNode_);
		AddDep(phaseNode_); }

	void Main() override {
		rclmt::jobsys::run(Compute());}

	void Draw(rglv::GL* _dc, const rmlm::mat4* pmat, const rmlm::mat4* mvmat, rclmt::jobsys::Job* link, int depth) override {
		using namespace rglv;
		auto& dc = *_dc;
		std::lock_guard<std::mutex> lock(dc.mutex);
		if (materialNode_ != nullptr) {
			materialNode_->Apply(_dc); }

		auto [id, ptr] = dc.AllocUniformBuffer<EnvmapXProgram::UniformsSD>();
		ptr->pm = *pmat;
		ptr->mvm = *mvmat;
		ptr->nm = transpose(inverse(ptr->mvm));
		ptr->mvpm = ptr->pm * ptr->mvm;
		dc.UseUniforms(id);

		dc.UseBuffer(0, buffers_[activeBuffer_]);
		dc.DrawElements(GL_TRIANGLES, meshIndices_.size(), GL_UNSIGNED_SHORT, meshIndices_.data());
		if (link != nullptr) {
			rclmt::jobsys::run(link); } }

public:
	rclmt::jobsys::Job* Compute(rclmt::jobsys::Job* parent = nullptr) {
		if (parent != nullptr) {
			return rclmt::jobsys::make_job_as_child(parent, Impl::ComputeJmp, std::tuple{this}); }
		
			return rclmt::jobsys::make_job(Impl::ComputeJmp, std::tuple{this}); }

private:
	static void ComputeJmp(rclmt::jobsys::Job* job, unsigned threadId, std::tuple<Impl*>* data) {
		auto&[self] = *data;
		self->ComputeImpl(); }
	void ComputeImpl() {
		using rmlv::ivec2;

		const int numPoints = src_.points.size();

		rmlv::vec3 freq{ 0.0F };
		if (freqNode_ != nullptr) {
			freq = freqNode_->Eval(freqSlot_).as_vec3();

		rmlv::vec3 phase{ 0.0F };
		if (phaseNode_ != nullptr) {
			phase = phaseNode_->Eval(phaseSlot_).as_vec3(); }

		//const float t = float(ttt.time());
		/*
		const float angle = t;
		const float freqx = sin(angle) * 0.75 + 0.75;
		const float freqy = freqx * 0.61283476f; // sin(t*1.3f)*4.0f;
		const float freqz = 0;  //sin(t*1.1f)*4.0f;
		*/

		const float amp = 1.0F; //sin(t*1.4)*30.0f;

		for (int i=0; i<numPoints; i++) {
			rmlv::vec3 position = src_.points[i];
			rmlv::vec3 normal = src_.vertex_normals[i];

			float f = (sin(position.x*freq.x + phase.x) + 0.5F);// * sin(vvn.y*freqy+t) * sin(vvn.z*freqz+t)
			position += normal * amp * f; // normal*amp*f
			float ff = (sin(position.y*freq.y + phase.y) + 0.5F);
			position += normal * amp * ff;

			dst_.points[i] = position; }
		dst_.compute_face_and_vertex_normals();

		if (meshIndices_.empty()) {
			for (const auto& face : src_.faces) {
				for (auto idx : face.point_idx) {
					meshIndices_.push_back(idx); }}}

		activeBuffer_ = (activeBuffer_+1)%3;
		auto& vao = buffers_[activeBuffer_];
		vao.clear();
		for (int i=0; i<numPoints; i++) {
			vao.append(dst_.points[i], dst_.vertex_normals[i], 0); }

		auto postSetup = rclmt::jobsys::make_job(rclmt::jobsys::noop);
		AddLinksTo(postSetup);
		materialNode_->AddLink(postSetup);
		materialNode_->Run();}}

private:
	// state
	std::array<rglv::VertexArray_F3F3F3, 3> buffers_{};
	int activeBuffer_{0};
	rcls::vector<uint16_t> meshIndices_;
	rglv::Mesh src_;
	rglv::Mesh dst_;

	// inputs
	IMaterial* materialNode_{nullptr};
	IValue* freqNode_{nullptr};
	std::string freqSlot_{};
	IValue* phaseNode_{nullptr};
	std::string phaseSlot_{}; };


class Compiler final : public NodeCompiler {
	void Build() override {
		if (!Input("material", /*required=*/true)) { return; }
		if (!Input("freq", /*required=*/true)) { return; }
		if (!Input("phase", /*required=*/true)) { return; }

		std::string_view meshPath{"notfound.obj"};
		if (auto jv = rclx::jv_find(data_, "mesh", JSON_STRING)) {
			meshPath = jv->toString(); }
		const auto& mesh = meshStore_->get(meshPath);

		out_ = std::make_shared<Impl>(id_, std::move(inputs_), mesh); }};


struct init { init() {
	NodeRegistry::GetInstance().Register("$fxAuraForLaura", [](){ return std::make_unique<Compiler>(); });
}} init{};


}  // namespace
}  // namespace rqdq
