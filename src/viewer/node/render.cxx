#include "render.hxx"

#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include "src/rcl/rclx/rclx_gason_util.hxx"
#include "src/viewer/node/gpu.hxx"

namespace rqdq {
namespace rqv {

void RenderNode::Connect(std::string_view attr, NodeBase* other, std::string_view slot) {
	if (attr == "gpu") {
		gpuNode_ = static_cast<GPUNode*>(other); }
	else {
		OutputNode::Connect(attr, other, slot); }}


void RenderNode::AddDeps() {
	OutputNode::AddDeps();
	AddDep(gpuNode_); }


void RenderNode::Reset() {
	OutputNode::Reset();
	colorCanvas_ = nullptr;
	smallCanvas_ = nullptr; }


bool RenderNode::IsValid() {
	if (colorCanvas_ == nullptr && smallCanvas_ == nullptr && outCanvas_ == nullptr) {
		std::cerr << "RenderNode(" << get_id() << ") has no output set" << std::endl;
		return false; }
	if (gpuNode_ == nullptr) {
		std::cerr << "RenderNode(" << get_id() << ") has no gpu" << std::endl;
		return false; }
	return OutputNode::IsValid(); }


void RenderNode::Main() {
	rclmt::jobsys::Job *renderJob = Render();
	internalDepthCanvas_.resize(width_, height_);
	internalColorCanvas_.resize(width_, height_);
	gpuNode_->SetDimensions(width_, height_);
	gpuNode_->SetTileDimensions(tileDim_);
	gpuNode_->SetAspect(float(width_) / float(height_));
	gpuNode_->AddLink(AfterAll(renderJob));
	gpuNode_->Run(); }


RenderToTexture::RenderToTexture(std::string_view id, InputList inputs, int width, int height, const float pa, bool aa)
	:TextureNode(id, std::move(inputs)), width_(width), height_(height), aspect_(pa), enableAA_(aa) {}


void RenderToTexture::Connect(std::string_view attr, NodeBase* other, std::string_view slot) {
	if (attr == "gpu") {
		gpuNode_ = static_cast<GPUNode*>(other); }
	else {
		TextureNode::Connect(attr, other, slot); }}


void RenderToTexture::AddDeps() {
	TextureNode::AddDeps();
	AddDep(gpuNode_); }


bool RenderToTexture::IsValid() {
	if (gpuNode_ == nullptr) {
		std::cerr << "RenderToTexture(" << get_id() << ") has no gpu" << std::endl;
		return false; }
	return TextureNode::IsValid(); }


void RenderToTexture::Main() {
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


void RenderToTexture::RenderImpl() {
	auto& gpu = gpuNode_->get_gpu();
	gpu.enableDoubleBuffering = true; // this->double_buffer;
	gpu.d_cc = &GetColorCanvas();
	gpu.d_dc = &GetDepthCanvas();
	auto& ic = gpu.IC();
	outCanvas_ = rglr::FloatingPointCanvas(outputTexture_.buf.data(), width_, height_, width_);
	if (enableAA_) {
		ic.storeHalfsize(&outCanvas_); }
	else {
		ic.storeUnswizzled(&outCanvas_); }
	ic.endDrawing();
	auto renderJob = gpu.render();
	rclmt::jobsys::add_link(renderJob, PostProcess());
	rclmt::jobsys::run(renderJob); }


void RenderToTexture::PostProcessImpl() {
	outputTexture_.maybe_make_mipmap();
	RunLinks(); }

}  // namespace rqv

namespace {

using namespace rqv;

class RenderCompiler final : public NodeCompiler {
	void Build() override {
		using rclx::jv_find;
		if (!Input("GPUNode", "gpu", /*required=*/true)) { return; }

		ShaderProgramId programId = ShaderProgramId::Default;
		if (auto jv = jv_find(data_, "program", JSON_STRING)) {
			programId = ShaderProgramNameSerializer::Deserialize(jv->toString()); }

		bool srgb{true};
		if (auto jv = jv_find(data_, "sRGB", JSON_FALSE)) {
			srgb = false; }

		out_ = std::make_shared<RenderNode>(id_, std::move(inputs_), programId, srgb); }};


class RenderToTextureCompiler final : public NodeCompiler {
	void Build() override {
		using rclx::jv_find;
		if (!Input("GPUNode", "gpu", /*required=*/true)) { return; }

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

		out_ = std::make_shared<RenderToTexture>(id_, std::move(inputs_), width, height, pa, aa); }};


RenderCompiler renderCompiler{};
RenderToTextureCompiler renderToTextureCompiler{};

struct init { init() {
	// XXX add an IOutput abstract type?
	NodeRegistry::GetInstance().Register(NodeInfo{ "$render",       "Render",       [](NodeBase* node) { return dynamic_cast<RenderNode*>(node) != nullptr; },       &renderCompiler });
	NodeRegistry::GetInstance().Register(NodeInfo{ "$renderToTexture", "RenderToTexture", [](NodeBase* node) { return dynamic_cast<RenderToTexture*>(node) != nullptr; },       &renderToTextureCompiler });
}} init{};


}  // namespace
}  // namespace rqdq
