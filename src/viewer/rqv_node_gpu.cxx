#include "src/viewer/rqv_node_gpu.hxx"
#include "src/rcl/rclmt/rclmt_jobsys.hxx"
#include "src/rgl/rglv/rglv_math.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"
#include "src/viewer/rqv_node_base.hxx"

#include <string>
#include <vector>
#include <tuple>

#include "3rdparty/ryg-srgb/ryg-srgb.h"

namespace rqdq {
namespace rqv {

using namespace std;
namespace jobsys = rclmt::jobsys;


GPUNode::GPUNode(const string& name,
                 const InputList& inputs) :
	NodeBase(name, inputs),
	gpu() {}


void GPUNode::connect(const string& attr, NodeBase *other, const string& slot) {
	if (attr == "layer") {
		layers.push_back(dynamic_cast<LayerNode*>(other)); }
	else {
		NodeBase::connect(attr, other, slot); }}


vector<NodeBase*> GPUNode::deps() {
	vector<NodeBase*> nodes;
	for (auto node : layers) {
		nodes.push_back(node); }
	return nodes; }


void GPUNode::reset() {
	NodeBase::reset();
	width = {};
	height = {};
	tile_dim = {}; }


void GPUNode::main() {
	namespace jobsys = jobsys;
	using rmlv::ivec2, rmlv::vec3, rmlv::vec4;

	const int targetWidth = width.value_or(256);
	const int targetHeight = height.value_or(256);
	const ivec2 td = tile_dim.value_or(ivec2{8, 8});
	gpu.reset(ivec2{ targetWidth, targetHeight }, td);
	auto& ic = gpu.IC();

	auto backgroundColor = vec3{ 0, 0, 0 };
	if (layers.size()) {
		auto& firstLayer = layers[0];
		backgroundColor = firstLayer->getBackgroundColor(); }
	ic.glClear(vec4{ backgroundColor, 1.0f });

	if (layers.size() == 0) {
		jobsys::Job *postJob = post();
		add_links_to(postJob);
		jobsys::run(postJob);
		return; }

	jobsys::Job *drawJob = draw();
	for (auto layer : layers) {
		layer->add_link(after_all(drawJob)); }
	for (auto layer : layers) {
		layer->run(); } }


void GPUNode::all_then(jobsys::Job* job, const unsigned tid, std::tuple<std::atomic<int>*, jobsys::Job*>* data) {
	auto [cnt, link] = *data;
	auto& counter = *cnt;
	if (--counter != 0) {
		return; }
	jobsys::run(link); }


void GPUNode::drawImpl() {
	const int targetWidth = width.value_or(256);
	const int targetHeight = height.value_or(256);
	const float targetAspect = aspect.value_or(float(targetWidth) / float(targetHeight));

	pcnt = layers.size();
	jobsys::Job *postJob = post();
	add_links_to(postJob);
	for (auto layer : layers) {
		layer->draw(&gpu.IC(), targetWidth, targetHeight, targetAspect, jobsys::make_job(GPUNode::all_then, std::tuple{&pcnt, postJob})); } }


}  // close package namespace
}  // close enterprise namespace
