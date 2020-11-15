#include "src/rcl/rclmt/rclmt_jobsys.hxx"
#include "src/rcl/rclx/rclx_gason_util.hxx"
#include "src/rgl/rglr/rglr_canvas.hxx"
#include "src/rgl/rglr/rglr_texture.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"
#include "src/viewer/compile.hxx"
#include "src/viewer/node/base.hxx"
#include "src/viewer/node/i_gpu.hxx"
#include "src/viewer/node/i_texture.hxx"

#include <iostream>
#include <string_view>
#include <utility>

namespace {

using namespace rqdq;
using namespace rqv;
namespace jobsys = rclmt::jobsys;

class Impl : public ITexture {

	rglr::Texture outputTexture_;
	rglr::FloatingPointCanvas outCanvas_;

	// inputs
	IGPU* gpuNode_{nullptr};

public:
	Impl(std::string_view id, InputList inputs) :
		ITexture(id, std::move(inputs)) {}

	auto Connect(std::string_view attr, NodeBase* other, std::string_view slot) -> bool override {
		if (attr == "gpu") {
			gpuNode_ = dynamic_cast<IGPU*>(other);
			if (gpuNode_ == nullptr) {
				TYPE_ERROR(IGPU);
				return false; }
			return true; }
		return ITexture::Connect(attr, other, slot); }

	void DisconnectAll() override {
		ITexture::DisconnectAll();
		gpuNode_ = nullptr; }

	void AddDeps() override {
		ITexture::AddDeps();
		AddDep(gpuNode_); }

	auto IsValid() -> bool override {
		if (gpuNode_ == nullptr) {
			std::cerr << "RenderToTexture(" << get_id() << ") has no gpu" << std::endl;
			return false; }
		return ITexture::IsValid(); }

	void Main() override {
		gpuNode_->AddLink(AfterAll(Render()));
		gpuNode_->Run(); }

	auto Render() -> jobsys::Job* {
		return jobsys::make_job(Impl::RenderJmp, std::tuple{this}); }
	static void RenderJmp(jobsys::Job*, unsigned threadId [[maybe_unused]], std::tuple<Impl*> * data) {
		auto&[self] = *data;
		self->RenderImpl(); }
	void RenderImpl() {
		auto targetSizeInPx = gpuNode_->GetTargetSize();
		auto aa = gpuNode_->GetAA();
		auto& ic = gpuNode_->IC();

		if (aa) {
			int w = targetSizeInPx.x / 2;
			int h = targetSizeInPx.y / 2;
			outputTexture_.resize(w, h);
			outCanvas_ = rglr::FloatingPointCanvas(outputTexture_.buf.data(), w, h, w);
			ic.StoreColor(&outCanvas_, /*downsample=*/true); }
		else {
			int w = targetSizeInPx.x;
			int h = targetSizeInPx.y;
			outputTexture_.resize(w, h);
			outCanvas_ = rglr::FloatingPointCanvas(outputTexture_.buf.data(), w, h, w);
			ic.StoreColor(&outCanvas_, /*downsample=*/false); }

		ic.Finish();
		auto renderJob = gpuNode_->Render();
		jobsys::add_link(renderJob, PostProcess());
		jobsys::run(renderJob); }

	auto PostProcess() -> jobsys::Job* {
		return jobsys::make_job(Impl::PostProcessJmp, std::tuple{this}); }
	static void PostProcessJmp(jobsys::Job*, unsigned threadId [[maybe_unused]], std::tuple<Impl*>* data) {
		auto&[self] = *data;
		self->PostProcessImpl(); }
	void PostProcessImpl() {
		outputTexture_.maybe_make_mipmap();
		RunLinks(); }

	// -- ITexture --
	auto GetTexture() -> const rglr::Texture& override {
		return outputTexture_; }};


class Compiler final : public NodeCompiler {
	void Build() override {
		if (!Input("gpu", /*required=*/true)) { return; }
		out_ = std::make_shared<Impl>(id_, std::move(inputs_)); }};


struct init { init() {
	NodeRegistry::GetInstance().Register("$renderToTexture", [](){ return std::make_unique<Compiler>(); });
}} init{};


}  // close unnamed namespace
