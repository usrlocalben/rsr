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

#include <iostream>
#include <memory>
#include <mutex>
#include <string_view>
#include <utility>

namespace {

using namespace rqdq;
using namespace rqv;

//PixelToaster::Timer ttt;

class Impl final : public IGl {

	const rglv::Icosphere mesh_;
	const int numVertices_;
	const int numRenderIndices_;

	// state
	int mod2_{0};
	std::vector<uint16_t> indices_;
	std::array<rglv::VertexArray_F3F3, 2> vbos_{};
	rglv::VertexArray_F3F3* vbo_{nullptr};

	// inputs
	IMaterial* materialNode_{nullptr};
	IValue* freqNode_{nullptr};
	std::string freqSlot_{};
	IValue* phaseNode_{nullptr};
	std::string phaseSlot_{};
	IValue* ampNode_{nullptr};
	std::string ampSlot_{};

public:
	Impl(std::string_view id, InputList inputs, int divs, int hunk) :
		NodeBase(id, std::move(inputs)),
		IGl(),
		mesh_(divs, hunk),
		numVertices_(mesh_.GetNumVertices()),
		numRenderIndices_(mesh_.GetNumRenderIndices()) {

		for (auto& vbo : vbos_) {
			vbo.resize(numVertices_);
			vbo.pad(); }

		indices_.resize(mesh_.GetNumIndices());
		for (int i=0; i<int(indices_.size()); ++i) {
			auto idx = mesh_.GetIndices()[i];
			assert(0 <= idx && idx < 65536);
			indices_[i] = static_cast<uint16_t>(idx); }}

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

	void DisconnectAll() override {
		IGl::DisconnectAll();
		materialNode_ = nullptr;
		freqNode_ = nullptr;
		phaseNode_ = nullptr;
		ampNode_ = nullptr; }

	void AddDeps() override {
		IGl::AddDeps();
		AddDep(materialNode_); }

	void Main() override {
		rclmt::jobsys::run(Compute());}

	void DrawDepth(rglv::GL* _dc, const rmlm::mat4* pmat, const rmlm::mat4* mvmat) override {
		using namespace rglv;
		auto& dc = *_dc;
		std::lock_guard lock(dc.mutex);
		dc.Enable(GL_CULL_FACE);
		dc.CullFace(GL_BACK);
		dc.Enable(GL_DEPTH_TEST);
		dc.DepthFunc(GL_LESS);

		dc.ViewMatrix(*mvmat);
		dc.ProjectionMatrix(*pmat);

		dc.UseBuffer(0, vbo_->a0);
		dc.UseBuffer(3, vbo_->a1);
		dc.DrawElements(GL_TRIANGLES, numRenderIndices_, GL_UNSIGNED_SHORT, indices_.data(), RGL_HINT_DENSE|RGL_HINT_READ4);
		dc.ResetX(); }

	void Draw(int pass, const LightPack& lights [[maybe_unused]], rglv::GL* _dc, const rmlm::mat4* pmat, const rmlm::mat4* vmat, const rmlm::mat4* mmat) override {
		using namespace rglv;
		auto& dc = *_dc;
		if (pass != 1) return;
		std::lock_guard lock(dc.mutex);
		if (materialNode_ != nullptr) {
			materialNode_->Apply(_dc); }

		dc.DepthWriteMask(true);
		dc.ColorWriteMask(true);
		dc.Enable(GL_CULL_FACE);
		dc.CullFace(GL_BACK);
		dc.Enable(GL_DEPTH_TEST);
		dc.DepthFunc(GL_LESS);
		dc.Disable(GL_BLEND);

		dc.ViewMatrix(*vmat * *mmat);
		dc.ProjectionMatrix(*pmat);

		dc.UseBuffer(0, vbo_->a0);
		dc.UseBuffer(3, vbo_->a1);
		dc.DrawElements(GL_TRIANGLES, numRenderIndices_, GL_UNSIGNED_SHORT, indices_.data(), RGL_HINT_DENSE|RGL_HINT_READ4);
		dc.ResetX();
	}

public:
	rclmt::jobsys::Job* Compute(rclmt::jobsys::Job* parent = nullptr) {
		if (parent != nullptr) {
			return rclmt::jobsys::make_job_as_child(parent, Impl::ComputeJmp, std::tuple{this}); }
		return rclmt::jobsys::make_job(Impl::ComputeJmp, std::tuple{this}); }

private:
	static void ComputeJmp(rclmt::jobsys::Job*, unsigned threadId [[maybe_unused]], std::tuple<Impl*>* data) {
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
		materialNode_->Run();}}};


class Compiler final : public NodeCompiler {
	void Build() override {
		if (!Input("material", /*required=*/true)) { return; }
		if (!Input("freq", /*required=*/true)) { return; }
		if (!Input("phase", /*required=*/true)) { return; }
		if (!Input("amp", /*required=*/true)) { return; }

		auto divs = DataInt("divs", 0);
		auto hunk = DataInt("hunk", 0);

		out_ = std::make_shared<Impl>(id_, std::move(inputs_), divs, hunk); }};


struct init { init() {
	NodeRegistry::GetInstance().Register("$auraForLaura", [](){ return std::make_unique<Compiler>(); });
}} init{};


}  // close unnamed namespace
