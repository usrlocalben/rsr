#include <atomic>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <tuple>

#include "src/rcl/rclma/rclma_framepool.hxx"
#include "src/rcl/rclmt/rclmt_jobsys.hxx"
#include "src/rcl/rclx/rclx_gason_util.hxx"
#include "src/rgl/rglr/rglr_fragmentcursor.hxx"
#include "src/rgl/rglv/rglv_gl.hxx"
#include "src/rgl/rglv/rglv_gpu.hxx"
#include "src/rgl/rglv/rglv_gpu_impl.hxx"
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
	IValue* enableNode_{nullptr};
	std::string enableSlot_{};

	std::vector<rcls::vector<float>> lightmaps_{};
	rglv::GPU gpu_{ jobsys::numThreads, std::string{"gllayer"} };

public:
	Impl(std::string_view id, InputList inputs) :
		ILayer(id, std::move(inputs)) {
		gpu_.Install(0, 0, rglv::GPUBinImpl<rglv::BaseProgram>::MakeBinProgramPtrs());
		gpu_.Install(0, 0x6a2, rglv::GPUTileImpl<rglr::QFloat3FragmentCursor, rglr::QFloatFragmentCursor, rglv::BaseProgram, false, true, rglv::DepthLT, true, false, rglv::BlendOff>::MakeDrawProgramPtrs()); }

	auto Connect(std::string_view attr, NodeBase* other, std::string_view slot) -> bool override {
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
		if (attr == "enable") {
			enableNode_ = dynamic_cast<IValue*>(other);
			enableSlot_ = slot;
			if (enableNode_ == nullptr) {
				TYPE_ERROR(IValue);
				return false; }
			return true; }
		return ILayer::Connect(attr, other, slot); }

	void Main() override {
		using rmlv::ivec2;

		jobsys::Job *doneJob = jobsys::make_job(jobsys::noop);
		AddLinksTo(doneJob);

		if (Enabled() && gls_.size()>0) {
			for (auto glnode : gls_) {
				glnode->AddLink(AfterAll(doneJob)); }
			for (auto glnode : gls_) {
				glnode->Run(); } }
		else {
			jobsys::run(doneJob); }}

	static
	void AllThen(rclmt::jobsys::Job*, unsigned threadId [[maybe_unused]], std::tuple<std::atomic<int>*, rclmt::jobsys::Job*>* data) {
		auto [cnt, link] = *data;
		auto& counter = *cnt;
		if (--counter != 0) {
			return; }
		jobsys::run(link); }

	auto Color() -> rmlv::vec3 override {
		auto color = rmlv::vec3{0.0F};
		if (colorNode_ != nullptr) {
			color = colorNode_->Eval(colorSlot_).as_vec3(); }
		return color; }

	void Render(int pass, rglv::GL* dc, rmlv::ivec2 targetSizeInPx [[maybe_unused]], float aspect) override {
		using namespace rmlm;
		using namespace rglv;

		if (!Enabled()) {
			return; }

		auto& pmat = *static_cast<mat4*>(rclma::framepool::Allocate(64));
		auto& vmat = *static_cast<mat4*>(rclma::framepool::Allocate(64));
		auto& mmat = *static_cast<mat4*>(rclma::framepool::Allocate(64));
		if (cameraNode_ != nullptr) {
			pmat = cameraNode_->ProjectionMatrix(aspect);
			vmat = cameraNode_->ViewMatrix(); }
		else {
			pmat = mat4{1};
			vmat = mat4{1}; }
		mmat = mat4{1};

		auto& lights = *static_cast<LightPack*>(rclma::framepool::Allocate(sizeof(LightPack)));
		lights = LightPack{};
		for (auto gl : gls_) {
			lights = Merge(lights, gl->Lights(mat4{1})); }
		for (int i = 0; i < lights.cnt; ++i) {
			auto lightToView = vmat * lights.vmat[i]; // lights.vmat is actually model matrix here (**)
			auto viewToLight = inverse(lightToView);
			auto pos = lightToView.position();
			auto dir = viewToLight.lookDir();

			lights.pmat[i] = rglv::Perspective2(lights.angle[i], 1.0, 20, 500);
			lights.vmat[i] = rmlm::inverse(lights.vmat[i]);  // now vmat is really viewmatrix (**)
			lights.pos[i] = pos;
			lights.dir[i] = dir;
			lights.cos[i] = std::cosf(lights.angle[i]/2.0F * 3.141592F/180);

			// XXX assert(popcnt(lights.size[i]) == 1);
			if (i >= int(lightmaps_.size())) {
				lightmaps_.emplace_back(); }
			lightmaps_[i].reserve(lights.size[i]*lights.size[i]);
			lights.map[i] = lightmaps_[i].data(); }

		std::array<jobsys::Job*, kMaxLights> lightJobs;
		std::fill(begin(lightJobs), end(lightJobs), nullptr);
		for (int li=0; li<lights.cnt; ++li) {
			auto& light_pmat = *static_cast<mat4*>(rclma::framepool::Allocate(64));
			auto& light_vmat = *static_cast<mat4*>(rclma::framepool::Allocate(64));
			light_pmat = lights.pmat[li];
			light_vmat = lights.vmat[li];

			auto dim = lights.size[li];

			gpu_.Reset({ dim, dim }, { 8, 8 });
			auto& ic = gpu_.IC();
			ic.RenderbufferType(rglv::GL_COLOR_ATTACHMENT0, rglv::RB_RGBF32);
			ic.RenderbufferType(rglv::GL_DEPTH_ATTACHMENT, rglv::RB_F32);
			ic.ColorWriteMask(false);
			ic.DepthWriteMask(true);
			ic.DepthFunc(GL_LESS);
			ic.Enable(GL_CULL_FACE);
			ic.CullFace(GL_FRONT);
			ic.ClearDepth(1.0F);
			ic.Clear(rglv::GL_DEPTH_BUFFER_BIT);
			ic.UseProgram(0);
			for (auto gl : gls_) {
				gl->DrawDepth(&ic, &light_pmat, &light_vmat); }
			ic.StoreDepth(lights.map[li]);
			ic.Finish();
			lightJobs[li] = jobsys::make_job(jobsys::noop);
			auto tmp = gpu_.Run();
			jobsys::add_link(tmp, lightJobs[li]);
			jobsys::run(tmp); }

		for (auto gl : gls_) {
			gl->Draw(pass, lights, dc, &pmat, &vmat, &mmat);}

		for (int li=0; li<lights.cnt; ++li) {
			if (lightJobs[li] != nullptr) {
				jobsys::wait(lightJobs[li]); }}}

protected:
	void AddDeps() override {
		ILayer::AddDeps();
		if (Enabled()) {
			for (auto gl : gls_) {
				AddDep(gl); } }}

private:
	rclmt::jobsys::Job* Post() {
		return rclmt::jobsys::make_job(Impl::PostJmp, std::tuple{this}); }
	static void PostJmp(rclmt::jobsys::Job*, unsigned threadId [[maybe_unused]], std::tuple<Impl*>* data) {
		auto& [self] = *data;
		self->Post(); }
	void PostImpl() {}

	auto Enabled() -> bool {
		bool enabled = true;
		if (enableNode_ != nullptr) {
			if (enableNode_->Eval(enableSlot_).as_float() < 1.0F) {
				enabled = false; }}
		return enabled; } };



class Compiler final : public NodeCompiler {
	void Build() override {
		if (!Input("camera", /*required=*/true)) { return; }
		if (!Input("color", /*required=*/false)) { return; }
		if (!Input("enable", /*required=*/false)) { return; }

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
