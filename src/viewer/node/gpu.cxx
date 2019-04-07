#include "gpu.hxx"

#include <string>
#include <string_view>
#include <vector>
#include <tuple>

#include "src/rcl/rclmt/rclmt_jobsys.hxx"
#include "src/rcl/rclx/rclx_gason_util.hxx"
#include "src/rgl/rglv/rglv_gpu.hxx"
#include "src/rgl/rglv/rglv_math.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"
#include "src/viewer/compile.hxx"
#include "src/viewer/node/base.hxx"
#include "src/viewer/node/gl.hxx"
#include "src/viewer/shaders.hxx"

#include "3rdparty/ryg-srgb/ryg-srgb.h"

namespace rqdq {
namespace rqv {

GPUNode::GPUNode(std::string_view id, InputList inputs)
	:NodeBase(id, std::move(inputs)) {}


void GPUNode::Connect(std::string_view attr, NodeBase* other, std::string_view slot) {
	if (attr == "layer") {
		layers_.push_back(static_cast<LayerNode*>(other)); }
	else {
		NodeBase::Connect(attr, other, slot); }}


void GPUNode::AddDeps() {
	NodeBase::AddDeps();
	for (auto node : layers_) {
		AddDep(node); }}


void GPUNode::Reset() {
	NodeBase::Reset();
	width_ = {};
	height_ = {};
	tileDim_ = {}; }


void GPUNode::Main() {
	namespace jobsys = jobsys;
	using rmlv::ivec2, rmlv::vec3, rmlv::vec4;

	const int targetWidth = width_.value_or(256);
	const int targetHeight = height_.value_or(256);
	const ivec2 td = tileDim_.value_or(ivec2{8, 8});
	gpu.reset(ivec2{ targetWidth, targetHeight }, td);
	auto& ic = gpu.IC();

	auto backgroundColor = vec3{ 0, 0, 0 };
	if (!layers_.empty()) {
		auto& firstLayer = layers_[0];
		backgroundColor = firstLayer->GetBackgroundColor(); }
	ic.glClear(vec4{ backgroundColor, 1.0F });

	if (layers_.empty()) {
		jobsys::Job *postJob = Post();
		AddLinksTo(postJob);
		jobsys::run(postJob);
		return; }

	jobsys::Job *drawJob = Draw();
	for (auto layer : layers_) {
		layer->AddLink(AfterAll(drawJob)); }
	for (auto layer : layers_) {
		layer->Run(); } }


void GPUNode::AllThen(jobsys::Job* job, const unsigned tid, std::tuple<std::atomic<int>*, jobsys::Job*>* data) {
	auto [cnt, link] = *data;
	auto& counter = *cnt;
	if (--counter != 0) {
		return; }
	jobsys::run(link); }


void GPUNode::DrawImpl() {
	const int targetWidth = width_.value_or(256);
	const int targetHeight = height_.value_or(256);
	const float targetAspect = aspect_.value_or(float(targetWidth) / float(targetHeight));

	pcnt_ = layers_.size();
	jobsys::Job *postJob = Post();
	AddLinksTo(postJob);
	for (auto layer : layers_) {
		layer->Render(&gpu.IC(), targetWidth, targetHeight, targetAspect, jobsys::make_job(GPUNode::AllThen, std::tuple{&pcnt_, postJob})); } }


}  // namespace rqv

namespace {

using namespace rqv;

class Compiler final : public NodeCompiler {
	void Build() override {
		if (auto jv = rclx::jv_find(data_, "layers", JSON_ARRAY)) {
			for (const auto& item : *jv) {
				if (item->value.getTag() == JSON_STRING) {
					inputs_.emplace_back("layer", item->value.toString()); }}}

		out_ = std::make_shared<GPUNode>(id_, std::move(inputs_)); }};


Compiler compiler{};

struct init { init() {
	NodeRegistry::GetInstance().Register(NodeInfo{
		"$gpu",
		"GPUNode",
		[](NodeBase* node) { return dynamic_cast<GPUNode*>(node) != nullptr; },
		&compiler });
}} init{};

}  // namespace
}  // namespace rqdq
