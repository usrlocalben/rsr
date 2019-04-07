#pragma once
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "src/rcl/rclmt/rclmt_jobsys.hxx"
#include "src/rgl/rglr/rglr_canvas.hxx"
#include "src/rgl/rglv/rglv_mesh_store.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"
#include "src/viewer/node/base.hxx"
#include "src/viewer/node/gpu.hxx"
#include "src/viewer/node/material.hxx"
#include "src/viewer/node/output.hxx"
#include "src/viewer/shaders.hxx"

namespace rqdq {
namespace rqv {

class RenderNode : public OutputNode {
public:
	RenderNode(std::string_view id, InputList inputs, ShaderProgramId programId, bool sRGB)
		:OutputNode(id, std::move(inputs)), programId_(programId), sRGB_(sRGB) {}

	void Connect(std::string_view attr, NodeBase* other, std::string_view slot) override;

	void AddDeps() override;

	void Reset() override;

	bool IsValid() override;

	void Main() override;

	rclmt::jobsys::Job* Render() {
		return rclmt::jobsys::make_job(RenderNode::RenderJmp, std::tuple{this}); }
	static void RenderJmp(rclmt::jobsys::Job* job, const unsigned tid, std::tuple<RenderNode*> * data) {
		auto&[self] = *data;
		self->RenderImpl(); }
	void RenderImpl() {
		auto& gpu = gpuNode_->get_gpu();
		gpu.enableDoubleBuffering = doubleBuffer_;
		gpu.d_cc = GetColorCanvas();
		gpu.d_dc = GetDepthCanvas();
		auto& ic = gpu.IC();
		if (smallCanvas_ != nullptr) {
			ic.storeHalfsize(smallCanvas_); }
		if (outCanvas_ != nullptr) {
			ic.glUseProgram(int(programId_));
			ic.storeTrueColor(sRGB_, outCanvas_); }
		ic.endDrawing();
		auto renderJob = gpu.render();
		AddLinksTo(renderJob);
		rclmt::jobsys::run(renderJob); }

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

	// config
	ShaderProgramId programId_;
	bool sRGB_;

	// inputs
	GPUNode* gpuNode_{nullptr};

	// received
	rglr::QFloat4Canvas* colorCanvas_;
	rglr::FloatingPointCanvas* smallCanvas_; };


// XXX multiple inheritance can be used here
class RenderToTexture : public TextureNode {
public:
	RenderToTexture(std::string_view id, InputList inputs, int width, int height, float pa, bool aa);

	void Connect(std::string_view attr, NodeBase* other, std::string_view slot) override;

	void AddDeps() override;

	bool IsValid() override;

	void Main() override;

	rclmt::jobsys::Job* Render() {
		return rclmt::jobsys::make_job(RenderToTexture::RenderJmp, std::tuple{this}); }
	static void RenderJmp(rclmt::jobsys::Job* job, const unsigned tid, std::tuple<RenderToTexture*> * data) {
		auto&[self] = *data;
		self->RenderImpl(); }
	void RenderImpl();

	rclmt::jobsys::Job* PostProcess() {
		return rclmt::jobsys::make_job(RenderToTexture::PostProcessJmp, std::tuple{this}); }
	static void PostProcessJmp(rclmt::jobsys::Job* job, const unsigned tid, std::tuple<RenderToTexture*>* data) {
		auto&[self] = *data;
		self->PostProcessImpl(); }
	void PostProcessImpl();

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
	GPUNode* gpuNode_{nullptr}; };



}  // namespace rqv
}  // namespace rqdq
