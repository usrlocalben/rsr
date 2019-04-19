#include <iostream>
#include <string_view>
#include <tuple>
#include <utility>

#include "src/rcl/rclmt/rclmt_jobsys.hxx"
#include "src/rcl/rclx/rclx_gason_util.hxx"
#include "src/rgl/rglr/rglr_canvas.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"
#include "src/viewer/compile.hxx"
#include "src/viewer/node/base.hxx"
#include "src/viewer/node/i_gpu.hxx"
#include "src/viewer/node/i_output.hxx"
#include "src/viewer/shaders.hxx"

namespace rqdq {
namespace {

using namespace rqv;

class Impl : public IOutput {
public:
	Impl(std::string_view id, InputList inputs, ShaderProgramId programId, bool sRGB)
		:IOutput(id, std::move(inputs)), programId_(programId), sRGB_(sRGB) {}

	bool Connect(std::string_view attr, NodeBase* other, std::string_view slot) override {
		if (attr == "gpu") {
			gpuNode_ = dynamic_cast<IGPU*>(other);
			if (gpuNode_ == nullptr) {
				TYPE_ERROR(IGPU);
				return false; }
			return true; }
		return IOutput::Connect(attr, other, slot); }

	void AddDeps() override {
		IOutput::AddDeps();
		AddDep(gpuNode_); }

	void Reset() override {
		IOutput::Reset();
		doubleBuffer_ = false;
		tileDim_ = rmlv::ivec2{8, 8};
		width_ = 0;
		height_ = 0;
		outCanvas_ = nullptr;
		colorCanvas_ = nullptr;
		smallCanvas_ = nullptr; }

	bool IsValid() override {
		if (colorCanvas_ == nullptr && smallCanvas_ == nullptr && outCanvas_ == nullptr) {
			std::cerr << "RenderNode(" << get_id() << ") has no output set" << std::endl;
			return false; }
		if (gpuNode_ == nullptr) {
			std::cerr << "RenderNode(" << get_id() << ") has no gpu" << std::endl;
			return false; }
		return IOutput::IsValid(); }

	void Main() override {
		rclmt::jobsys::Job *renderJob = Render();
		internalDepthCanvas_.resize(width_, height_);
		internalColorCanvas_.resize(width_, height_);
		gpuNode_->SetDimensions(width_, height_);
		gpuNode_->SetTileDimensions(tileDim_);
		gpuNode_->SetAspect(float(width_) / float(height_));
		gpuNode_->AddLink(AfterAll(renderJob));
		gpuNode_->Run(); }

	rclmt::jobsys::Job* Render() override {
		return rclmt::jobsys::make_job(Impl::RenderJmp, std::tuple{this}); }
	static void RenderJmp(rclmt::jobsys::Job* job, const unsigned tid, std::tuple<Impl*> * data) {
		auto&[self] = *data;
		self->RenderImpl(); }
	void RenderImpl() {
		gpuNode_->SetDoubleBuffer(doubleBuffer_);
		gpuNode_->SetColorCanvas(GetColorCanvas());
		gpuNode_->SetDepthCanvas(GetDepthCanvas());
		auto& ic = gpuNode_->GetIC();
		if (smallCanvas_ != nullptr) {
			ic.storeHalfsize(smallCanvas_); }
		if (outCanvas_ != nullptr) {
			ic.glUseProgram(int(programId_));
			ic.storeTrueColor(sRGB_, outCanvas_); }
		ic.endDrawing();
		auto renderJob = gpuNode_->Render();
		AddLinksTo(renderJob);
		rclmt::jobsys::run(renderJob); }

	void SetOutputCanvas(rglr::TrueColorCanvas* canvas) override {
		outCanvas_ = canvas;
		width_ = canvas->width();
		height_ = canvas->height(); }

	void SetDoubleBuffer(bool value) override {
		doubleBuffer_ = value; }

	void SetTileDim(rmlv::ivec2 value) override {
		tileDim_ = value; }

	void SetColorCanvas(rglr::QFloat4Canvas* canvas) {
		colorCanvas_ = canvas;
		width_ = canvas->width();
		height_ = canvas->height(); }

	void SetSmallCanvas(rglr::FloatingPointCanvas* canvas) {
		smallCanvas_ = canvas;
		width_ = canvas->width();
		height_ = canvas->height(); }

	rglr::QFloatCanvas* GetDepthCanvas() {
		internalDepthCanvas_.resize(width_, height_);
		return &internalDepthCanvas_; }

	rglr::QFloat4Canvas* GetColorCanvas() {
		if (colorCanvas_ != nullptr) {
			// std::cout << "renderer will use a provided colorcanvas" << std::endl;
			return colorCanvas_; }
		// std::cout << "renderer will use its internal colorcanvas" << std::endl;
		// internalColorCanvas_.resize(width, height);
		return &internalColorCanvas_; }

	// internal
	rglr::QFloat4Canvas internalColorCanvas_;
	rglr::QFloatCanvas internalDepthCanvas_;

	// static config
	ShaderProgramId programId_;
	bool sRGB_;

	// runtime config
	rglr::TrueColorCanvas* outCanvas_{nullptr};
	int width_{0};
	int height_{0};
	bool doubleBuffer_{false};
	rmlv::ivec2 tileDim_{8, 8};

	// inputs
	IGPU* gpuNode_{nullptr};

	// received
	rglr::QFloat4Canvas* colorCanvas_;
	rglr::FloatingPointCanvas* smallCanvas_; };


class Compiler final : public NodeCompiler {
	void Build() override {
		using rclx::jv_find;
		if (!Input("gpu", /*required=*/true)) { return; }

		ShaderProgramId programId = ShaderProgramId::Default;
		if (auto jv = jv_find(data_, "program", JSON_STRING)) {
			programId = ShaderProgramNameSerializer::Deserialize(jv->toString()); }

		bool srgb{true};
		if (auto jv = jv_find(data_, "sRGB", JSON_FALSE)) {
			srgb = false; }

		out_ = std::make_shared<Impl>(id_, std::move(inputs_), programId, srgb); }};



struct init { init() {
	NodeRegistry::GetInstance().Register("$render", [](){ return std::make_unique<Compiler>(); });
}} init{};


}  // namespace
}  // namespace rqdq
