#pragma once
#include <rclmt_jobsys.hxx>
#include <rglr_canvas.hxx>
#include <rglv_mesh_store.hxx>
#include <rmlv_vec.hxx>
#include <rqv_node_base.hxx>
#include <rqv_node_gpu.hxx>
#include <rqv_node_material.hxx>
#include <rqv_node_output.hxx>
#include <rqv_shaders.hxx>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace rqdq {
namespace rqv {

struct RenderNode : OutputNode {
	// internal
	rglr::QFloat4Canvas internal_color_canvas;
	rglr::QFloatCanvas internal_depth_canvas;

	// config
	const ShaderProgramId program;
	const bool sRGB;

	// inputs
	GPUNode* gpu_node = nullptr;

	// received
	rglr::QFloat4Canvas* colorcanvas;
	rglr::FloatingPointCanvas* smallcanvas;

	RenderNode(const std::string& name, const InputList& inputs, ShaderProgramId program, bool sRGB) :OutputNode(name, inputs), program(program), sRGB(sRGB) {}

	void connect(const std::string& attr, NodeBase* other, const std::string& slot) override;
	std::vector<NodeBase*> deps() override;
	void reset() override;
	bool validate_settings() override;
	void main() override;

	rclmt::jobsys::Job* render() {
		return rclmt::jobsys::make_job(RenderNode::renderJmp, std::tuple{this}); }
	static void renderJmp(rclmt::jobsys::Job* job, const unsigned tid, std::tuple<RenderNode*> * data) {
		auto&[self] = *data;
		self->renderImpl(); }
	void renderImpl() {
		auto& gpu = gpu_node->gpu;
		gpu.enableDoubleBuffering = this->double_buffer;
		gpu.d_cc = &get_colorcanvas();
		gpu.d_dc = &get_depthcanvas();
		auto& ic = gpu.IC();
		if (smallcanvas != nullptr) {
			ic.storeHalfsize(smallcanvas); }
		if (outcanvas != nullptr) {
			ic.glUseProgram(int(program));
			ic.storeTrueColor(sRGB, outcanvas); }
		ic.endDrawing();
		auto renderJob = gpu.render();
		add_links_to(renderJob);
		rclmt::jobsys::run(renderJob); }

	void set_color_canvas(rglr::QFloat4Canvas *canvas) {
		colorcanvas = canvas;
		width = canvas->width();
		height = canvas->height(); }

	void set_small_canvas(rglr::FloatingPointCanvas *canvas) {
		smallcanvas = canvas;
		width = canvas->width();
		height = canvas->height(); }

	rglr::QFloatCanvas& get_depthcanvas() {
		internal_depth_canvas.resize(width, height);
		return internal_depth_canvas; }

	rglr::QFloat4Canvas& get_colorcanvas() {
		if (colorcanvas != nullptr) {
			// std::cout << "renderer will use a provided colorcanvas" << std::endl;
			return *colorcanvas; }
		else {
			// std::cout << "renderer will use its internal colorcanvas" << std::endl;
			// internal_color_canvas.resize(width, height);
			return internal_color_canvas; } }};


// XXX multiple inheritance can be used here
struct RenderToTexture : TextureNode {
	// internal
	rglr::QFloat4Canvas internal_color_canvas;
	rglr::QFloatCanvas internal_depth_canvas;
	rglr::Texture d_out;

	rglr::FloatingPointCanvas d_outCanvas;

	// config
	const int d_width;
	const int d_height;
	const float d_aspect;
	const bool d_aa;

	// inputs
	GPUNode* gpu_node = nullptr;

	RenderToTexture(const std::string& name, const InputList& inputs, int width, int height, float pa, bool aa);

	void connect(const std::string& attr, NodeBase* other, const std::string& slot) override;
	std::vector<NodeBase*> deps() override;
	bool validate_settings() override;
	void main() override;

	rclmt::jobsys::Job* render() {
		return rclmt::jobsys::make_job(RenderToTexture::renderJmp, std::tuple{this}); }
	static void renderJmp(rclmt::jobsys::Job* job, const unsigned tid, std::tuple<RenderToTexture*> * data) {
		auto&[self] = *data;
		self->renderImpl(); }
	void renderImpl();

	rclmt::jobsys::Job* postProcess() {
		return rclmt::jobsys::make_job(RenderToTexture::postProcessJmp, std::tuple{this}); }
	static void postProcessJmp(rclmt::jobsys::Job* job, const unsigned tid, std::tuple<RenderToTexture*>* data) {
		auto&[self] = *data;
		self->postProcessImpl(); }
	void postProcessImpl();

	rglr::QFloatCanvas& get_depthcanvas() {
		const int renderWidth = d_aa ? d_width * 2 : d_width;
		const int renderHeight = d_aa ? d_height * 2 : d_height;
		internal_depth_canvas.resize(renderWidth, renderHeight);
		return internal_depth_canvas; }

	rglr::QFloat4Canvas& get_colorcanvas() {
		const int renderWidth = d_aa ? d_width * 2 : d_width;
		const int renderHeight = d_aa ? d_height * 2 : d_height;
		internal_color_canvas.resize(renderWidth, renderHeight);
		return internal_color_canvas; }

	const rglr::Texture& getTexture() override {
		return d_out; }};

}  // close package namespace
}  // close enterprise namespace
