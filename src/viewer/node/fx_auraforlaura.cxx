#include <iostream>
#include <memory>
#include <mutex>
#include <string_view>
#include <utility>

#include "src/rcl/rclx/rclx_gason_util.hxx"
#include "src/rgl/rglv/rglv_gl.hxx"
#include "src/rgl/rglv/rglv_icosphere.hxx"
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
	Impl(std::string_view id, InputList inputs, int divs, int hunk) :
		IGl(id, std::move(inputs)),
		mesh_(divs, hunk),
		numVertices_(mesh_.GetNumVertices()),
		numRenderIndices_(mesh_.GetNumRenderIndices()) {

		for (auto& vbo : vbos_) {
			vbo.resize(numVertices_);
			vbo.pad(); }

		indices_.resize(mesh_.GetNumIndices());
		for (int i=0; i<indices_.size(); ++i) {
			indices_[i] = mesh_.GetIndices()[i]; }}

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
		if (attr == "amp") {
			ampNode_ = dynamic_cast<IValue*>(other);
			ampSlot_ = slot;
			if (ampNode_ == nullptr) {
				TYPE_ERROR(IValue);
				return false; }
			return true; }
		return IGl::Connect(attr, other, slot); }

	void AddDeps() override {
		AddDep(materialNode_);
		AddDep(freqNode_);
		AddDep(phaseNode_);
		AddDep(ampNode_); }

	void Main() override {
		rclmt::jobsys::run(Compute());}

	void Draw(rglv::GL* _dc, const rmlm::mat4* pmat, const rmlm::mat4* mvmat, rclmt::jobsys::Job* link, int depth) override {
		using namespace rglv;
		auto& dc = *_dc;
		std::lock_guard<std::mutex> lock(dc.mutex);
		if (materialNode_ != nullptr) {
			materialNode_->Apply(_dc); }

		dc.ViewMatrix(*mvmat);
		dc.ProjectionMatrix(*pmat);
		auto [id, ptr] = dc.AllocUniformBuffer<EnvmapXProgram::UniformsSD>();
		dc.UseUniforms(id);

		dc.UseBuffer(0, *vbo_);
		dc.DrawElements(GL_TRIANGLES, numRenderIndices_, GL_UNSIGNED_SHORT, indices_.data(), RGL_HINT_DENSE|RGL_HINT_READ4);
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

		mod2_ = (mod2_+1)%2;
		vbo_ = &vbos_[mod2_];
		auto& vbo = *vbo_;

		rmlv::vec3 freq{ 0.0F };
		if (freqNode_ != nullptr) {
			freq = freqNode_->Eval(freqSlot_).as_vec3();

		rmlv::vec3 phase{ 0.0F };
		if (phaseNode_ != nullptr) {
			phase = phaseNode_->Eval(phaseSlot_).as_vec3(); }

		float amp{ 0.5F };
		if (ampNode_ != nullptr) {
			amp = ampNode_->Eval(ampSlot_).as_float(); }

		//const float t = float(ttt.time());
		/*
		const float angle = t;
		const float freqx = sin(angle) * 0.75 + 0.75;
		const float freqy = freqx * 0.61283476f; // sin(t*1.3f)*4.0f;
		const float freqz = 0;  //sin(t*1.1f)*4.0f;
		*/

		for (int i=0; i<numVertices_; i++) {
			rmlv::vec3 position = mesh_.GP(i);
			rmlv::vec3 normal = position;

			float f = (sin(position.x*freq.x + phase.x) + 0.5F);// * sin(vvn.y*freqy+t) * sin(vvn.z*freqz+t)
			position += normal * amp * f; // normal*amp*f
			float ff = (sin(position.y*freq.y + phase.y) + 0.5F);
			position += normal * amp * ff;

			vbo.a0.set(i, position); }

		vbo.a1.zero();
		for (int i{0}; i<mesh_.GetNumIndices(); i+=3) {
			int i0=indices_[i+0];
			int i1=indices_[i+1];
			int i2=indices_[i+2];
			auto p0 = vbo.a0.at(i0);
			auto p1 = vbo.a0.at(i1);
			auto p2 = vbo.a0.at(i2);
			auto dir = cross(p1-p0, p2-p0);

			// dir will be normalized in vertex shader
			vbo.a1.add(i0, dir);
			vbo.a1.add(i1, dir);
			vbo.a1.add(i2, dir); }

		auto postSetup = rclmt::jobsys::make_job(rclmt::jobsys::noop);
		AddLinksTo(postSetup);
		materialNode_->AddLink(postSetup);
		materialNode_->Run();}}

private:
	const rglv::Icosphere mesh_;
	const int numVertices_;
	const int numRenderIndices_;

	// state
	int mod2_{0};
	std::vector<uint16_t> indices_;
	std::array<rglv::VertexArray_F3F3F3, 2> vbos_{};
	rglv::VertexArray_F3F3F3* vbo_;

	// inputs
	IMaterial* materialNode_{nullptr};
	IValue* freqNode_{nullptr};
	std::string freqSlot_{};
	IValue* phaseNode_{nullptr};
	std::string phaseSlot_{};
	IValue* ampNode_{nullptr};
	std::string ampSlot_{}; };


class Compiler final : public NodeCompiler {
	void Build() override {
		if (!Input("material", /*required=*/true)) { return; }
		if (!Input("freq", /*required=*/true)) { return; }
		if (!Input("phase", /*required=*/true)) { return; }
		if (!Input("amp", /*required=*/true)) { return; }

		int divs = 0;
		if (auto jv = rclx::jv_find(data_, "divs", JSON_NUMBER)) {
			divs = (int)jv->toNumber(); }

		int hunk = 0;
		if (auto jv = rclx::jv_find(data_, "hunk", JSON_NUMBER)) {
			hunk = (int)jv->toNumber(); }

		out_ = std::make_shared<Impl>(id_, std::move(inputs_), divs, hunk); }};


struct init { init() {
	NodeRegistry::GetInstance().Register("$fxAuraForLaura", [](){ return std::make_unique<Compiler>(); });
}} init{};


}  // namespace
}  // namespace rqdq
