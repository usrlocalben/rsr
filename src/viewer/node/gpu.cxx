#include <atomic>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

#include "src/rcl/rclmt/rclmt_jobsys.hxx"
#include "src/rcl/rclx/rclx_gason_util.hxx"
#include "src/rgl/rglv/rglv_gpu.hxx"
#include "src/rgl/rglv/rglv_math.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"
#include "src/viewer/compile.hxx"
#include "src/viewer/node/base.hxx"
#include "src/viewer/node/i_gpu.hxx"
#include "src/viewer/node/i_layer.hxx"
#include "src/viewer/shaders.hxx"

namespace rqdq {
namespace {

using namespace rqv;

namespace jobsys = rclmt::jobsys;

using GPU = rglv::GPU<rglv::BaseProgram, WireframeProgram, IQPostProgram, EnvmapProgram, AmyProgram, EnvmapXProgram>;


class GPUNode : public IGPU {
public:
	using IGPU::IGPU;

	bool Connect(std::string_view attr, NodeBase* other, std::string_view slot) override {
		if (attr == "layer") {
			auto tmp = dynamic_cast<ILayer*>(other);
			if (tmp == nullptr) {
				TYPE_ERROR(ILayer);
				return false; }
			layers_.push_back(tmp);
			return true; }
		return IGPU::Connect(attr, other, slot); }

	void AddDeps() override {
		IGPU::AddDeps();
		for (auto node : layers_) {
			AddDep(node); }}

	void Reset() override {
		NodeBase::Reset();
		width_ = {};
		height_ = {};
		tileDim_ = {}; }

	void Main() override {
		using rmlv::ivec2, rmlv::vec3, rmlv::vec4;

		const int targetWidth = width_.value_or(256);
		const int targetHeight = height_.value_or(256);
		const ivec2 td = tileDim_.value_or(ivec2{8, 8});
		gpu_.Reset(ivec2{ targetWidth, targetHeight }, td);
		auto& ic = gpu_.IC();

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

	void Dimensions(int x, int y) override {
		width_ = x; height_ = y; }

	void TileDimensions(rmlv::ivec2 tileDim) override {
		tileDim_ = tileDim; }

	void Aspect(float aspect) override {
		aspect_ = aspect; }

	rclmt::jobsys::Job* Draw() {
		return rclmt::jobsys::make_job(GPUNode::DrawJmp, std::tuple{this}); }
	static void DrawJmp(rclmt::jobsys::Job* job, const unsigned tid, std::tuple<GPUNode*>* data) {
		auto& [self] = *data;
		self->DrawImpl(); }
	void DrawImpl() {
		const int targetWidth = width_.value_or(256);
		const int targetHeight = height_.value_or(256);
		const float targetAspect = aspect_.value_or(float(targetWidth) / float(targetHeight));

		pcnt_ = layers_.size();
		jobsys::Job *postJob = Post();
		AddLinksTo(postJob);
		for (auto layer : layers_) {
			layer->Render(&gpu_.IC(), targetWidth, targetHeight, targetAspect, jobsys::make_job(GPUNode::AllThen, std::tuple{&pcnt_, postJob})); } }

	static void AllThen(rclmt::jobsys::Job* job, unsigned tid, std::tuple<std::atomic<int>*, rclmt::jobsys::Job*>* data) {
		auto [cnt, link] = *data;
		auto& counter = *cnt;
		if (--counter != 0) {
			return; }
		jobsys::run(link); }

	rclmt::jobsys::Job* Post() {
		return rclmt::jobsys::make_job(GPUNode::PostJmp, std::tuple{this}); }
	static void PostJmp(rclmt::jobsys::Job* job, const unsigned tid, std::tuple<GPUNode*>* data) {
		auto& [self] = *data;
		self->Post(); }
	void PostImpl() {}

	void DoubleBuffer(bool value) override {
		gpu_.DoubleBuffer(value); }

	void ColorCanvas(rglr::Int16Canvas* ptr) override {
		gpu_.ColorCanvas(ptr); }

	void DepthCanvas(rglr::QFloatCanvas* ptr) override {
		gpu_.DepthCanvas(ptr); }

	rglv::GL& IC() override {
		return gpu_.IC(); }

	rclmt::jobsys::Job* Render() override {
		return gpu_.Run(); }

private:
	// internal
	GPU gpu_;

	std::atomic<int> pcnt_;  // all_then counter

	// inputs
	std::vector<ILayer*> layers_;

	// received
	std::optional<int> width_;
	std::optional<int> height_;
	std::optional<float> aspect_;
	std::optional<rmlv::ivec2> tileDim_; };


class Compiler final : public NodeCompiler {
	void Build() override {
		if (auto jv = rclx::jv_find(data_, "layers", JSON_ARRAY)) {
			for (const auto& item : *jv) {
				if (item->value.getTag() == JSON_STRING) {
					inputs_.emplace_back("layer", item->value.toString()); }}}

		out_ = std::make_shared<GPUNode>(id_, std::move(inputs_)); }};


struct init { init() {
	NodeRegistry::GetInstance().Register("$gpu", [](){ return std::make_unique<Compiler>(); });
}} init{};


}  // namespace
}  // namespace rqdq
