#include <rqv_node_render.hxx>

namespace rqdq {
namespace rqv {

void RenderNode::connect(const std::string & attr, NodeBase * other, const std::string & slot) {
	if (attr == "gpu") {
		gpu_node = dynamic_cast<GPUNode*>(other); }
	else {
		OutputNode::connect(attr, other, slot); }}


std::vector<NodeBase*> RenderNode::deps() {
	std::vector<NodeBase*> out;
	if (gpu_node != nullptr) {
		out.push_back(gpu_node); }
	return out; }


void RenderNode::reset() {
	colorcanvas = nullptr;
	smallcanvas = nullptr;
	OutputNode::reset(); }


bool RenderNode::validate_settings() {
	if (colorcanvas == nullptr && smallcanvas == nullptr && outcanvas == nullptr) {
		std::cout << "RenderNode(" << name << ") has no output set" << std::endl;
		return false; }
	if (gpu_node == nullptr) {
		std::cout << "RenderNode(" << name << ") has no gpu" << std::endl; }
	// bool base_is_valid = OutputNode::validate_settings();
	return true; }


void RenderNode::main() {
	rclmt::jobsys::Job *renderJob = render();
	internal_depth_canvas.resize(width, height);
	internal_color_canvas.resize(width, height);
	gpu_node->setDimensions(width, height);
	gpu_node->setTileDimensions(tile_dim);
	gpu_node->setAspect(float(width) / float(height));
	gpu_node->add_link(after_all(renderJob));
	gpu_node->run(); }


RenderToTexture::RenderToTexture(const std::string & name, const InputList & inputs, int width, int height, const float pa, bool aa)
	:TextureNode(name, inputs), d_width(width), d_height(height), d_aspect(pa), d_aa(aa) {}


void RenderToTexture::connect(const std::string & attr, NodeBase * other, const std::string & slot) {
	if (attr == "gpu") {
		gpu_node = dynamic_cast<GPUNode*>(other); }
	else {
		TextureNode::connect(attr, other, slot); }}


std::vector<NodeBase*> RenderToTexture::deps() {
	std::vector<NodeBase*> out;
	if (gpu_node != nullptr) {
		out.push_back(gpu_node); }
	return out; }


bool RenderToTexture::validate_settings() {
	if (gpu_node == nullptr) {
		std::cout << "RenderToTexture(" << name << ") has no gpu" << std::endl; }
	// bool base_is_valid = OutputNode::validate_settings();
	return true; }


void RenderToTexture::main() {
	rclmt::jobsys::Job *renderJob = render();
	if (d_aa) {
		// internal_depth_canvas.resize(d_dim*2, d_dim*2);
		// internal_color_canvas.resize(d_dim*2, d_dim*2);
		gpu_node->setDimensions(d_width*2, d_height*2); }
	else {
		// internal_depth_canvas.resize(d_dim, d_dim);
		// internal_color_canvas.resize(d_dim, d_dim);
		gpu_node->setDimensions(d_width, d_height); }
	d_out.resize(d_width, d_height);
	gpu_node->setTileDimensions(rmlv::ivec2{ 8, 8 });
	gpu_node->setAspect(d_aspect);
	gpu_node->add_link(after_all(renderJob));
	gpu_node->run(); }


void RenderToTexture::renderImpl() {
	auto& gpu = gpu_node->gpu;
	gpu.enableDoubleBuffering = true; // this->double_buffer;
	gpu.d_cc = &get_colorcanvas();
	gpu.d_dc = &get_depthcanvas();
	auto& ic = gpu.IC();
	d_outCanvas = rglr::FloatingPointCanvas(d_out.buf.data(), d_width, d_height, d_width);
	if (d_aa) {
		ic.storeHalfsize(&d_outCanvas); }
	else {
		ic.storeUnswizzled(&d_outCanvas); }
	ic.endDrawing();
	auto renderJob = gpu.render();
	rclmt::jobsys::add_link(renderJob, postProcess());
	rclmt::jobsys::run(renderJob); }

void RenderToTexture::postProcessImpl() {
	d_out.maybe_make_mipmap();
	for (auto link : links) {
		rclmt::jobsys::run(link); }}


}  // close package namespace
}  // close enterprise namespace
