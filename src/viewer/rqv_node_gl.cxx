#include "src/viewer/rqv_node_gl.hxx"
#include "src/rcl/rclma/rclma_framepool.hxx"
#include "src/rcl/rclmt/rclmt_jobsys.hxx"
#include "src/rgl/rglv/rglv_math.hxx"
#include "src/rml/rmlm/rmlm_mat4.hxx"
#include "src/viewer/rqv_node_camera.hxx"
#include "src/viewer/rqv_node_value.hxx"

#include <memory>
#include <string>

namespace rqdq {
namespace rqv {

using namespace std;
namespace jobsys = rclmt::jobsys;

void GroupNode::connect(const string& attr, NodeBase* other, const string& slot) {
	if (attr == "gl") {
		gls.push_back(dynamic_cast<GlNode*>(other)); }
	else {
		GlNode::connect(attr, other, slot); }}


void GroupNode::main() {
	using rmlv::ivec2;

	jobsys::Job *doneJob = jobsys::make_job(jobsys::noop);
	add_links_to(doneJob);

	if (gls.size() == 0) {
		jobsys::run(doneJob); }
	else {
		for (auto glnode : gls) {
			glnode->add_link(after_all(doneJob)); }
		for (auto glnode : gls) {
			glnode->run(); } }}


void GroupNode::draw(rglv::GL* dc, const rmlm::mat4* const pmat, const rmlm::mat4* const mvmat, rclmt::jobsys::Job *link, int depth) {
	pcnt = gls.size();
	for (auto glnode : gls) {
		// XXX increment depth even if matrix/transform stack is not modified?
		glnode->draw(dc, pmat, mvmat, jobsys::make_job(GroupNode::all_then, std::tuple{&pcnt, link}), depth); }}


void GroupNode::all_then(jobsys::Job* job, const unsigned tid, std::tuple<std::atomic<int>*, jobsys::Job*>* data) {
	auto [cnt, link] = *data;
	auto& counter = *cnt;
	if (--counter != 0) {
		return; }
	jobsys::run(link); }


void LayerNode::connect(const string& attr, NodeBase* other, const string& slot) {
	if (attr == "camera") {
		camera_node = dynamic_cast<CameraNode*>(other); }
	else if (attr == "color") {
		color_node = dynamic_cast<ValuesBase*>(other);
		color_slot = slot; }
	else {
		GroupNode::connect(attr, other, slot); }}


void LayerNode::main() {
	using rmlv::ivec2;

	jobsys::Job *doneJob = jobsys::make_job(jobsys::noop);
	add_links_to(doneJob);

	if (gls.size() == 0) {
		jobsys::run(doneJob); }
	else {
		for (auto glnode : gls) {
			glnode->add_link(after_all(doneJob)); }
		for (auto glnode : gls) {
			glnode->run(); } }}


void LayerNode::draw(rglv::GL * dc, int width, int height, float aspect, rclmt::jobsys::Job * link) {
	using namespace rmlm;
	using namespace rglv;
	namespace framepool = rclma::framepool;

	mat4* pmat = reinterpret_cast<mat4*>(framepool::allocate(64));
	mat4* mvmat = reinterpret_cast<mat4*>(framepool::allocate(64));
	if (camera_node) {
		*pmat = camera_node->getProjectionMatrix(aspect);
		*mvmat = camera_node->getModelViewMatrix(); }
	else {
		*pmat = mat4::ident();
		*mvmat = mat4::ident(); }

	pcnt = gls.size();
	for (auto gl : gls) {
		gl->draw(dc, pmat, mvmat, jobsys::make_job(LayerNode::all_then, std::tuple{&pcnt, link}), 0);}}


void LayerChooser::connect(const string& attr, NodeBase* other, const string& slot) {
	if (attr == "layer") {
		if (auto node = dynamic_cast<LayerNode*>(other)) {
			layers.push_back(node); }}
	else if (attr == "selector") {
		selector_node = dynamic_cast<ValuesBase*>(other);
		selector_slot = slot; }
	else {
		LayerNode::connect(attr, other, slot); }}


std::vector<NodeBase*> LayerChooser::deps() {
	auto deps = GroupNode::deps();
	auto layerNode = getSelectedLayer();
	deps.push_back(selector_node);
	if (layerNode != nullptr) {
		deps.push_back(layerNode);
	}
	return deps;
}


void LayerChooser::main() {
	using rmlv::ivec2;
	jobsys::Job *doneJob = jobsys::make_job(jobsys::noop);
	add_links_to(doneJob);

	auto layerNode = getSelectedLayer();
	if (layerNode == nullptr) {
		jobsys::run(doneJob); }
	else {
		layerNode->add_link(after_all(doneJob));
		layerNode->run(); }}


rmlv::vec3 LayerChooser::getBackgroundColor() {
	auto layerNode = getSelectedLayer();
	if (layerNode == nullptr) {
		return rmlv::vec3{ 0.0f }; }
	return layerNode->getBackgroundColor(); }


void LayerChooser::draw(rglv::GL * dc, int width, int height, float aspect, rclmt::jobsys::Job * link) {
	auto layerNode = getSelectedLayer();
	if (layerNode == nullptr) {
		rclmt::jobsys::run(link); }
	else {
		layerNode->draw(dc, width, height, aspect, link); }}


LayerNode* LayerChooser::getSelectedLayer() const {
	if (layers.empty()) {
		return nullptr; }
	auto idx = selector_node->get(selector_slot).as_int();
	const int lastLayer = int(layers.size() - 1);
	if (idx < 0 || lastLayer < idx) {
		return nullptr; }
	return layers[idx]; }


}  // close package namespace
}  // close enterprise namespace
