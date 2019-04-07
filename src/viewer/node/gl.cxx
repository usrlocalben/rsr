#include "gl.hxx"

#include <memory>
#include <string_view>

#include "src/rcl/rclma/rclma_framepool.hxx"
#include "src/rcl/rclmt/rclmt_jobsys.hxx"
#include "src/rcl/rclx/rclx_gason_util.hxx"
#include "src/rgl/rglv/rglv_math.hxx"
#include "src/rml/rmlm/rmlm_mat4.hxx"
#include "src/viewer/compile.hxx"
#include "src/viewer/node/camera.hxx"
#include "src/viewer/node/value.hxx"

namespace rqdq {
namespace rqv {

namespace jobsys = rclmt::jobsys;

void GroupNode::Connect(std::string_view attr, NodeBase* other, std::string_view slot) {
	if (attr == "gl") {
		gls_.push_back(static_cast<GlNode*>(other)); }
	else {
		GlNode::Connect(attr, other, slot); }}


void GroupNode::Main() {
	using rmlv::ivec2;

	jobsys::Job *doneJob = jobsys::make_job(jobsys::noop);
	AddLinksTo(doneJob);

	if (gls_.empty()) {
		jobsys::run(doneJob); }
	else {
		for (auto glnode : gls_) {
			glnode->AddLink(AfterAll(doneJob)); }
		for (auto glnode : gls_) {
			glnode->Run(); } }}


void GroupNode::Draw(rglv::GL* dc, const rmlm::mat4* const pmat, const rmlm::mat4* const mvmat, rclmt::jobsys::Job *link, int depth) {
	pcnt_ = gls_.size();
	for (auto glnode : gls_) {
		// XXX increment depth even if matrix/transform stack is not modified?
		glnode->Draw(dc, pmat, mvmat, jobsys::make_job(GroupNode::AllThen, std::tuple{&pcnt_, link}), depth); }}


void GroupNode::AllThen(jobsys::Job* job, const unsigned tid, std::tuple<std::atomic<int>*, jobsys::Job*>* data) {
	auto [cnt, link] = *data;
	auto& counter = *cnt;
	if (--counter != 0) {
		return; }
	jobsys::run(link); }


void LayerNode::Connect(std::string_view attr, NodeBase* other, std::string_view slot) {
	if (attr == "camera") {
		cameraNode_ = static_cast<CameraNode*>(other); }
	else if (attr == "color") {
		colorNode_ = static_cast<ValuesBase*>(other);
		colorSlot_ = slot; }
	else {
		GroupNode::Connect(attr, other, slot); }}


void LayerNode::Main() {
	using rmlv::ivec2;

	jobsys::Job *doneJob = jobsys::make_job(jobsys::noop);
	AddLinksTo(doneJob);

	if (gls_.empty()) {
		jobsys::run(doneJob); }
	else {
		for (auto glnode : gls_) {
			glnode->AddLink(AfterAll(doneJob)); }
		for (auto glnode : gls_) {
			glnode->Run(); } }}


void LayerNode::Render(rglv::GL * dc, int width, int height, float aspect, rclmt::jobsys::Job * link) {
	using namespace rmlm;
	using namespace rglv;
	namespace framepool = rclma::framepool;

	mat4* pmat = reinterpret_cast<mat4*>(framepool::Allocate(64));
	mat4* mvmat = reinterpret_cast<mat4*>(framepool::Allocate(64));
	if (cameraNode_ != nullptr) {
		*pmat = cameraNode_->GetProjectionMatrix(aspect);
		*mvmat = cameraNode_->GetModelViewMatrix(); }
	else {
		*pmat = mat4::ident();
		*mvmat = mat4::ident(); }

	pcnt_ = gls_.size();
	for (auto gl : gls_) {
		gl->Draw(dc, pmat, mvmat, jobsys::make_job(LayerNode::AllThen, std::tuple{&pcnt_, link}), 0);}}


void LayerChooser::Connect(std::string_view attr, NodeBase* other, std::string_view slot) {
	if (attr == "layer") {
		layers_.push_back(static_cast<LayerNode*>(other)); }
	else if (attr == "selector") {
		selectorNode_ = static_cast<ValuesBase*>(other);
		selectorSlot_ = slot; }
	else {
		LayerNode::Connect(attr, other, slot); }}


void LayerChooser::AddDeps() {
	GroupNode::AddDeps();
	AddDep(selectorNode_);
	AddDep(GetSelectedLayer()); }


void LayerChooser::Main() {
	using rmlv::ivec2;
	jobsys::Job *doneJob = jobsys::make_job(jobsys::noop);
	AddLinksTo(doneJob);

	auto layerNode = GetSelectedLayer();
	if (layerNode == nullptr) {
		jobsys::run(doneJob); }
	else {
		layerNode->AddLink(AfterAll(doneJob));
		layerNode->Run(); }}


rmlv::vec3 LayerChooser::GetBackgroundColor() {
	auto layerNode = GetSelectedLayer();
	if (layerNode == nullptr) {
		return rmlv::vec3{ 0.0F }; }
	return layerNode->GetBackgroundColor(); }


void LayerChooser::Render(rglv::GL * dc, int width, int height, float aspect, rclmt::jobsys::Job* link) {
	auto layerNode = GetSelectedLayer();
	if (layerNode == nullptr) {
		rclmt::jobsys::run(link); }
	else {
		layerNode->Render(dc, width, height, aspect, link); }}


LayerNode* LayerChooser::GetSelectedLayer() const {
	if (layers_.empty()) {
		return nullptr; }
	auto idx = selectorNode_->Get(selectorSlot_).as_int();
	const int lastLayer = int(layers_.size() - 1);
	if (idx < 0 || lastLayer < idx) {
		return nullptr; }
	return layers_[idx]; }

}  // namespace rqv

namespace {

using namespace rqv;

class LayerCompiler final : public NodeCompiler {
	void Build() override {
		if (!Input("CameraNode", "camera", /*required=*/true)) { return; }
		if (!Input("ValuesBase", "color", /*required=*/false)) { return; }

		if (auto jv = rclx::jv_find(data_, "gl", JSON_ARRAY)) {
			for (const auto& item : *jv) {
				if (item->value.getTag() == JSON_STRING) {
					inputs_.emplace_back("gl", item->value.toString()); } } }
		out_ = std::make_shared<LayerNode>(id_, std::move(inputs_)); }};


class GroupCompiler final : public NodeCompiler {
	void Build() override {
		if (auto jv = rclx::jv_find(data_, "gl", JSON_ARRAY)) {
			for (const auto& item : *jv) {
				if (item->value.getTag() == JSON_STRING) {
					inputs_.emplace_back("gl", item->value.toString()); } } }
		out_ = std::make_shared<GroupNode>(id_, std::move(inputs_)); }};


class LayerChooserCompiler final : public NodeCompiler {
	void Build() override {
		if (!Input("ValuesBase", "selector", /*required=*/true)) { return; }
		if (auto jv = rclx::jv_find(data_, "layers", JSON_ARRAY)) {
			for (const auto& item : *jv) {
				if (item->value.getTag() == JSON_STRING) {
					inputs_.emplace_back("layer", item->value.toString()); } } }
		out_ = std::make_shared<LayerChooser>(id_, std::move(inputs_)); }};


GroupCompiler groupCompiler{};
LayerChooserCompiler layerChooserCompiler{};
LayerCompiler layerCompiler{};

struct init { init() {
	NodeRegistry::GetInstance().Register(NodeInfo{ "IGl", "GlNode", [](NodeBase* node) { return dynamic_cast<GlNode*>(node) != nullptr; }, nullptr });
	NodeRegistry::GetInstance().Register(NodeInfo{ "$group", "Group", [](NodeBase* node) { return dynamic_cast<GroupNode*>(node) != nullptr; }, &groupCompiler });
	NodeRegistry::GetInstance().Register(NodeInfo{ "$layer", "Layer", [](NodeBase* node) { return dynamic_cast<LayerNode*>(node) != nullptr; }, &layerCompiler });
	NodeRegistry::GetInstance().Register(NodeInfo{ "$layerChooser", "LayerChooser", [](NodeBase* node) { return dynamic_cast<LayerChooser*>(node) != nullptr; }, &layerChooserCompiler });
}} init{};


}  // namespace
}  // namespace rqdq
