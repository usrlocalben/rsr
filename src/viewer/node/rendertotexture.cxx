#include <iostream>
#include <string_view>
#include <utility>

#include "src/rcl/rclmt/rclmt_jobsys.hxx"
#include "src/rcl/rclx/rclx_gason_util.hxx"
#include "src/rgl/rglr/rglr_canvas.hxx"
#include "src/rgl/rglr/rglr_texture.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"
#include "src/viewer/compile.hxx"
#include "src/viewer/node/base.hxx"
#include "src/viewer/node/i_gpu.hxx"
#include "src/viewer/node/i_texture.hxx"

namespace rqdq {
namespace {

using namespace rqv;

class Impl : public ITexture {
public:
	Impl(std::string_view id, InputList inputs, int width, int height, float pa, bool aa)
		:ITexture(id, std::move(inputs)), width_(width), height_(height), aspect_(pa), enableAA_(aa) {}

	bool Connect(std::string_view attr, NodeBase* other, std::string_view slot) override {
		if (attr == "gpu") {
			gpuNode_ = dynamic_cast<IGPU*>(other);
			if (gpuNode_ == nullptr) {
				TYPE_ERROR(IGPU);
				return false; }
			return true; }
		return ITexture::Connect(attr, other, slot); }

	void AddDeps() override {
		ITexture::AddDeps();
		AddDep(gpuNode_); }

	bool IsValid() override {
		if (gpuNode_ == nullptr) {
			std::cerr << "RenderToTexture(" << get_id() << ") has no gpu" << std::endl;
			return false; }
		return ITexture::IsValid(); }

	void Main() override {
		rclmt::jobsys::Job *renderJob = Render();
		if (enableAA_) {
			// internalDepthCanvas_.resize(dim_*2, dim_*2);
			// internalColorCanvas_.resize(dim_*2, dim_*2);
			gpuNode_->SetDimensions(width_*2, height_*2); }
		else {
			// internalDepthCanvas_.resize(dim_, dim_);
			// internalColorCanvas_.resize(dim_, dim_);
			gpuNode_->SetDimensions(width_, height_); }
		outputTexture_.resize(width_, height_);
		gpuNode_->SetTileDimensions(rmlv::ivec2{ 8, 8 });
		gpuNode_->SetAspect(aspect_);
		gpuNode_->AddLink(AfterAll(renderJob));
		gpuNode_->Run(); }

	rclmt::jobsys::Job* Render() {
		return rclmt::jobsys::make_job(Impl::RenderJmp, std::tuple{this}); }
	static void RenderJmp(rclmt::jobsys::Job* job, const unsigned tid, std::tuple<Impl*> * data) {
		auto&[self] = *data;
		self->RenderImpl(); }
	void RenderImpl() {
		gpuNode_->SetDoubleBuffer(true);  // this->double_buffer;
		gpuNode_->SetColorCanvas(&GetColorCanvas());
		gpuNode_->SetDepthCanvas(&GetDepthCanvas());
		auto& ic = gpuNode_->GetIC();
		outCanvas_ = rglr::FloatingPointCanvas(outputTexture_.buf.data(), width_, height_, width_);
		if (enableAA_) {
			ic.storeHalfsize(&outCanvas_); }
		else {
			ic.storeUnswizzled(&outCanvas_); }
		ic.endDrawing();
		auto renderJob = gpuNode_->Render();
		rclmt::jobsys::add_link(renderJob, PostProcess());
		rclmt::jobsys::run(renderJob); }

	rclmt::jobsys::Job* PostProcess() {
		return rclmt::jobsys::make_job(Impl::PostProcessJmp, std::tuple{this}); }
	static void PostProcessJmp(rclmt::jobsys::Job* job, const unsigned tid, std::tuple<Impl*>* data) {
		auto&[self] = *data;
		self->PostProcessImpl(); }
	void PostProcessImpl() {
		outputTexture_.maybe_make_mipmap();
		RunLinks(); }

	rglr::QFloatCanvas& GetDepthCanvas() {
		const int renderWidth = enableAA_ ? width_*2 : width_;
		const int renderHeight = enableAA_ ? height_*2 : height_;
		internalDepthCanvas_.resize(renderWidth, renderHeight);
		return internalDepthCanvas_; }

	rglr::QFloat4Canvas& GetColorCanvas() {
		const int renderWidth = enableAA_ ? width_*2 : width_;
		const int renderHeight = enableAA_ ? height_*2 : height_;
		internalColorCanvas_.resize(renderWidth, renderHeight);
		return internalColorCanvas_; }

	const rglr::Texture& GetTexture() override {
		return outputTexture_; }

private:
	rglr::QFloat4Canvas internalColorCanvas_;
	rglr::QFloatCanvas internalDepthCanvas_;

	rglr::Texture outputTexture_;

	rglr::FloatingPointCanvas outCanvas_;

	// config
	int width_{0};
	int height_{0};
	float aspect_{1.0};
	bool enableAA_{false};

	// inputs
	IGPU* gpuNode_{nullptr}; };


class Compiler final : public NodeCompiler {
	void Build() override {
		using rclx::jv_find;
		if (!Input("gpu", /*required=*/true)) { return; }

		int width{256};
		if (auto jv = jv_find(data_, "width", JSON_NUMBER)) {
			width = static_cast<int>(jv->toNumber()); }

		int height{256};
		if (auto jv = jv_find(data_, "height", JSON_NUMBER)) {
			height = static_cast<int>(jv->toNumber()); }

		bool aa{false};
		if (auto jv = jv_find(data_, "aa", JSON_TRUE)) {
			aa = true; }

		float pa{1.0F};
		if (auto jv = jv_find(data_, "aspect", JSON_NUMBER)) {
			pa = static_cast<float>(jv->toNumber()); }

		out_ = std::make_shared<Impl>(id_, std::move(inputs_), width, height, pa, aa); }};


struct init { init() {
	NodeRegistry::GetInstance().Register("$renderToTexture", [](){ return std::make_unique<Compiler>(); });
}} init{};


}  // namespace
}  // namespace rqdq
