#include <atomic>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <tuple>

#include "src/rcl/rclma/rclma_framepool.hxx"
#include "src/rcl/rclmt/rclmt_jobsys.hxx"
#include "src/rcl/rclx/rclx_gason_util.hxx"
#include "src/rgl/rglv/rglv_gpu.hxx"
#include "src/rgl/rglv/rglv_gl.hxx"
#include "src/rml/rmlm/rmlm_mat4.hxx"
#include "src/viewer/compile.hxx"
#include "src/viewer/node/base.hxx"
#include "src/viewer/node/i_camera.hxx"
#include "src/viewer/node/i_gl.hxx"
#include "src/viewer/node/i_layer.hxx"
#include "src/viewer/node/i_value.hxx"

namespace rqdq {
namespace {

using namespace rqv;

namespace jobsys = rclmt::jobsys;

class Impl : public ILayer {
	std::vector<IGl*> gls_{};
	ICamera* cameraNode_{nullptr};
	IValue* colorNode_{nullptr};
	std::string colorSlot_{};
	std::vector<rcls::vector<float>> lightmaps_{};
	rglv::GPU gpu_{ jobsys::numThreads };

public:
	Impl(std::string_view id, InputList inputs) :
		ILayer(id, std::move(inputs)) {
		int id = 0;

	id = static_cast<int>(ShaderProgramId::Default);
	gpu.Install(id, 0, rglv::GPUBltImpl<DefaultPostProgram>::MakeBltProgramPtrs());

	id = static_cast<int>(ShaderProgramId::Wireframe);
	gpu.Install(id, 0, rglv::GPUBinImpl<WireframeProgram>::MakeVertexProgramPtrs());
	gpu.Install(id, NOSCISSOR_DEPTH_LESS_DEPTHWRITE_COLORWRITE_NOBLEND,
				rglv::GPUTileImpl<rglv::QFloat4RGBRenderbufferCursor, rglv::QFloat4ARenderbufferCursor, WireframeProgram, false, true, rglv::DepthLT, true, true, rglv::BlendOff>::MakeFragmentProgramPtrs());

	}

	bool Connect(std::string_view attr, NodeBase* other, std::string_view slot) override {
		if (attr == "camera") {
			cameraNode_ = dynamic_cast<ICamera*>(other);
			if (cameraNode_ == nullptr) {
				TYPE_ERROR(ICamera);
				return false; }
			return true; }
		if (attr == "color") {
			colorNode_ = dynamic_cast<IValue*>(other);
			colorSlot_ = slot;
			if (colorNode_ == nullptr) {
				TYPE_ERROR(IValue);
				return false; }
			return true; }
		if (attr == "gl") {
			IGl* tmp = dynamic_cast<IGl*>(other);
			if (tmp == nullptr) {
				TYPE_ERROR(IGl);
				return false; }
			gls_.push_back(tmp);
			return true; }
		return ILayer::Connect(attr, other, slot); }

	void Main() override {
		using rmlv::ivec2;

		jobsys::Job *doneJob = jobsys::make_job(jobsys::noop);
		AddLinksTo(doneJob);

		if (gls_.empty()) {
			jobsys::run(doneJob); }
		else {
			for (auto glnode : gls_) {
				glnode->AddLink(AfterAll(doneJob)); }
			for (auto glnode : gls_) {
				glnode->Run(); } }}

	static void AllThen(rclmt::jobsys::Job*, unsigned threadId [[maybe_unused]], std::tuple<std::atomic<int>*, rclmt::jobsys::Job*>* data) {
		auto [cnt, link] = *data;
		auto& counter = *cnt;
		if (--counter != 0) {
			return; }
		jobsys::run(link); }

	rmlv::vec3 GetBackgroundColor() override {
		auto color = rmlv::vec3{0.0F};
		if (colorNode_ != nullptr) {
			color = colorNode_->Eval(colorSlot_).as_vec3(); }
		return color; }

	void Render(int pass, rglv::GL* dc, rmlv::ivec2 targetSizeInPx [[maybe_unused]], float aspect) override {
		using namespace rmlm;
		using namespace rglv;

		auto& pmat = *static_cast<mat4*>(rclma::framepool::Allocate(64));
		auto& mvmat = *static_cast<mat4*>(rclma::framepool::Allocate(64));
		if (cameraNode_ != nullptr) {
			pmat = cameraNode_->ProjectionMatrix(aspect);
			mvmat = cameraNode_->ViewMatrix(); }
		else {
			pmat = mat4{1};
			mvmat = mat4{1}; }

		auto& lights = *static_cast<LightPack*>(rclma::framepool::Allocate(sizeof(LightPack)));
		lights = LightPack{};
		for (auto gl : gls_) {
			lights = Merge(lights, gl->Lights(mat4{1})); }
		for (int i = 0; i < lights.cnt; ++i) {
			lights.pmat[i] = rglv::Perspective2(lights.angle[i], 1.0, 1, 1000);
			lights.mvmat[i] = rmlm::inverse(lights.mvmat[i]);
			if (i >= int(lightmaps_.size())) {
				lightmaps_.emplace_back(256*256); }
			lights.map[i] = lightmaps_[i].data(); }

		jobsys::Job* lightJob{nullptr};
		if (lights.cnt) {
			gpu_.Reset({ 256, 256 }, { 8, 8 });
			auto& ic = gpu_.IC();
			ic.RenderbufferType(rglv::GL_COLOR_ATTACHMENT0, rglv::RB_RGBAF32);
			ic.RenderbufferType(rglv::GL_DEPTH_ATTACHMENT, rglv::RB_F32);
			ic.ColorWriteMask(false);
			ic.ClearDepth(1.0F);
			ic.Clear(rglv::GL_DEPTH_BUFFER_BIT);
			for (auto gl : gls_) {
				gl->DrawDepth(&ic, &lights.pmat[0], &lights.mvmat[0]); }
			ic.StoreDepth(lights.map[0]);
			ic.Finish();
			lightJob = gpu_.Run();
			jobsys::run(lightJob); }

		for (auto gl : gls_) {
			gl->Draw(pass, lights, dc, &pmat, &mvmat);}

		jobsys::wait(lightJob); }

protected:
	void AddDeps() override {
		ILayer::AddDeps();
		AddDep(cameraNode_);
		AddDep(colorNode_);
		for (auto gl : gls_) {
			AddDep(gl); } }

private:
	rclmt::jobsys::Job* Post() {
		return rclmt::jobsys::make_job(Impl::PostJmp, std::tuple{this}); }
	static void PostJmp(rclmt::jobsys::Job*, unsigned threadId [[maybe_unused]], std::tuple<Impl*>* data) {
		auto& [self] = *data;
		self->Post(); }
	void PostImpl() {}};



class Compiler final : public NodeCompiler {
	void Build() override {
		if (!Input("camera", /*required=*/true)) { return; }
		if (!Input("color", /*required=*/false)) { return; }

		if (auto jv = rclx::jv_find(data_, "gl", JSON_ARRAY)) {
			for (const auto& item : *jv) {
				if (item->value.getTag() == JSON_STRING) {
					inputs_.emplace_back("gl", item->value.toString()); } } }
		out_ = std::make_shared<Impl>(id_, std::move(inputs_)); }};


struct init { init() {
	NodeRegistry::GetInstance().Register("$layer", [](){ return std::make_unique<Compiler>(); });
}} init{};


}  // namespace
}  // namespace rqdq
