#include "src/rcl/rclmt/rclmt_jobsys.hxx"
#include "src/rcl/rclx/rclx_gason_util.hxx"
#include "src/rgl/rglr/rglr_canvas.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"
#include "src/viewer/compile.hxx"
#include "src/viewer/node/base.hxx"
#include "src/viewer/node/i_canvas.hxx"
#include "src/viewer/node/i_gpu.hxx"

#include <iostream>
#include <stdexcept>
#include <string_view>
#include <utility>

namespace {

using namespace rqdq;
using namespace rqv;
namespace jobsys = rclmt::jobsys;

class Impl : public ICanvas {

	rglr::QFloat4Canvas colorCanvas_;
	rglr::QFloatCanvas depthCanvas_;
	rglr::FloatingPointCanvas halfCanvas_;

	// config
	const bool storeColor_;
	const bool storeDepth_;
	const bool storeHalf_;

	// inputs
	IGPU* gpuNode_{nullptr};

public:
	Impl(std::string_view id, InputList inputs, bool color, bool depth, bool half) :
		ICanvas(id, std::move(inputs)),
		storeColor_(color),
		storeDepth_(depth),
		storeHalf_(half) {}

	auto Connect(std::string_view attr, NodeBase* other, std::string_view slot) -> bool override {
		if (attr == "gpu") {
			gpuNode_ = dynamic_cast<IGPU*>(other);
			if (gpuNode_ == nullptr) {
				TYPE_ERROR(IGPU);
				return false; }
			return true; }
		return ICanvas::Connect(attr, other, slot); }

	void DisconnectAll() override {
		ICanvas::DisconnectAll();
		gpuNode_ = nullptr; }

	void AddDeps() override {
		AddDep(gpuNode_); }

	auto IsValid() -> bool override {
		if (gpuNode_ == nullptr) {
			std::cerr << "buffers(" << get_id() << ") has no gpu" << std::endl;
			return false; }
		return ICanvas::IsValid(); }

	void Main() override {
		gpuNode_->AddLink(AfterAll(Render()));
		gpuNode_->Run(); }

	auto Render() -> jobsys::Job* {
		return jobsys::make_job(Impl::RenderJmp, std::tuple{this}); }
	static void RenderJmp(jobsys::Job*, unsigned threadId [[maybe_unused]], std::tuple<Impl*> * data) {
		auto&[self] = *data;
		self->RenderImpl(); }
	void RenderImpl() {
		auto& ic = gpuNode_->IC();
		auto targetSizeInPx = gpuNode_->GetTargetSize();

		if (storeDepth_) {
			depthCanvas_.resize(targetSizeInPx.x, targetSizeInPx.y);
			/* ic.StoreDepth(&depthCanvas_); */ }
		if (storeColor_) {
			colorCanvas_.resize(targetSizeInPx.x, targetSizeInPx.y);
			ic.StoreColor(&colorCanvas_); }
		if (storeHalf_) {
			halfCanvas_.resize(targetSizeInPx.x/2, targetSizeInPx.y/2);
			ic.StoreColor(&halfCanvas_, /*downsample=*/true); }

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
		RunLinks(); }

	// -- ICanvas --
    auto GetCanvas(std::string_view slot) -> std::pair<int, const void*> override {
		if (slot == "depth") {
			return {ICanvas::CT_FLOAT_QUADS, &depthCanvas_}; }
		if (slot == "color") {
			return {ICanvas::CT_FLOAT4_QUADS, &colorCanvas_}; }
		if (slot == "half") {
			return {ICanvas::CT_FLOAT4_LINEAR, &halfCanvas_}; }
		throw std::runtime_error("renderbuffer: GetCanvas with invalid name"); } };


class Compiler final : public NodeCompiler {
	void Build() override {
		using rclx::jv_find;
		if (!Input("gpu", /*required=*/true)) { return; }

		auto color = DataBool("color", false);
		auto depth = DataBool("depth", false);
		auto half = DataBool("half", false);

		out_ = std::make_shared<Impl>(id_, std::move(inputs_), color, depth, half); }};


struct init { init() {
	NodeRegistry::GetInstance().Register("$buffers", [](){ return std::make_unique<Compiler>(); });
}} init{};


}  // close unnamed namespace
