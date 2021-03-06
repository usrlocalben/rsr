#include "src/rcl/rclmt/rclmt_jobsys.hxx"
#include "src/rcl/rclx/rclx_gason_util.hxx"
#include "src/rgl/rglv/rglv_gpu.hxx"
#include "src/rgl/rglv/rglv_math.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"
#include "src/viewer/compile.hxx"
#include "src/viewer/node/base.hxx"
#include "src/viewer/node/i_gpu.hxx"
#include "src/viewer/node/i_layer.hxx"
#include "src/viewer/node/i_value.hxx"
#include "src/viewer/shaders.hxx"

#include <atomic>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

namespace {

using namespace rqdq;
using namespace rqv;
namespace jobsys = rclmt::jobsys;

class GPUNode : public IGPU {

	// internal
	rglv::GPU gpu_;

	// static
	const bool aa_;

	std::atomic<int> pcnt_;  // all_then counter

	// inputs
	std::vector<ILayer*> layers_;
	IValue* targetSizeNode_{nullptr};
	std::string targetSizeSlot_{};
	IValue* tileSizeNode_{nullptr};
	std::string tileSizeSlot_{};
	IValue* aspectNode_{nullptr};
	std::string aspectSlot_{};

	// received
	rmlv::ivec2 targetSizeInPx_;
	rmlv::ivec2 tileSizeInBlocks_;
	float aspect_;

public:
	GPUNode(std::string_view id, InputList inputs, bool aa) :
		IGPU(id, inputs),
		gpu_(jobsys::numThreads),
		aa_(aa) {
		rqdq::rqv::Install(gpu_); }

	auto Connect(std::string_view attr, NodeBase* other, std::string_view slot) -> bool override {
		if (attr == "layer") {
			auto tmp = dynamic_cast<ILayer*>(other);
			if (tmp == nullptr) {
				TYPE_ERROR(ILayer);
				return false; }
			layers_.push_back(tmp);
			return true; }
		if (attr == "targetSize") {
			auto tmp = dynamic_cast<IValue*>(other);
			if (tmp == nullptr) {
				TYPE_ERROR(IValue);
				return false; }
			targetSizeNode_ = tmp;
			targetSizeSlot_ = slot;
			return true; }
		if (attr == "tileSize") {
			auto tmp = dynamic_cast<IValue*>(other);
			if (tmp == nullptr) {
				TYPE_ERROR(IValue);
				return false; }
			tileSizeNode_ = tmp;
			tileSizeSlot_ = slot;
			return true; }
		if (attr == "aspect") {
			auto tmp = dynamic_cast<IValue*>(other);
			if (tmp == nullptr) {
				TYPE_ERROR(IValue);
				return false; }
			aspectNode_ = tmp;
			aspectSlot_ = slot;
			return true; }
		return IGPU::Connect(attr, other, slot); }

	void DisconnectAll() override {
		IGPU::DisconnectAll();
		layers_.clear();
		targetSizeNode_ = nullptr;
		tileSizeNode_ = nullptr;
		aspectNode_ = nullptr; }

	void AddDeps() override {
		IGPU::AddDeps();
		for (auto node : layers_) {
			AddDep(node); }}

	void Reset() override {
		NodeBase::Reset(); }

	void Main() override {
		using rmlv::ivec2, rmlv::vec3, rmlv::vec4;

		targetSizeInPx_ = rmlv::ivec2{ 256, 256 };
		if (targetSizeNode_ != nullptr) {
			auto value = targetSizeNode_->Eval(targetSizeSlot_).as_vec2();
			targetSizeInPx_ = rmlv::ivec2{ (int)value.x, (int)value.y }; }
		if (aa_) {
			targetSizeInPx_ = targetSizeInPx_ * 2; }

		tileSizeInBlocks_ = rmlv::ivec2{ 8, 8 };
		if (tileSizeNode_ != nullptr) {
			auto value = tileSizeNode_->Eval(tileSizeSlot_).as_vec2();
			tileSizeInBlocks_ = ivec2{ (int)value.x, (int)value.y }; }

		aspect_ = float(targetSizeInPx_.x) / targetSizeInPx_.y;
		if (aspectNode_ != nullptr) {
			aspect_ = aspectNode_->Eval(aspectSlot_).as_float(); }

		gpu_.Reset(targetSizeInPx_, tileSizeInBlocks_);
		auto& ic = gpu_.IC();

		auto backgroundColor = vec3{ 0, 0, 0 };
		if (!layers_.empty()) {
			auto& firstLayer = layers_[0];
			backgroundColor = firstLayer->Color(); }
		ic.RenderbufferType(rglv::GL_COLOR_ATTACHMENT0, rglv::RB_COLOR_DEPTH);
		ic.RenderbufferType(rglv::GL_DEPTH_ATTACHMENT, rglv::RB_COLOR_DEPTH);
		ic.ClearColor(backgroundColor);
		ic.ClearDepth(1.0F);
		ic.Clear(rglv::GL_COLOR_BUFFER_BIT|rglv::GL_DEPTH_BUFFER_BIT);

		if (layers_.empty()) {
			RunLinks(); }
		else {
			jobsys::Job *drawJob = Draw();
			for (auto layer : layers_) {
				layer->AddLink(AfterAll(drawJob)); }
			for (auto layer : layers_) {
				layer->Run(); } }}

	auto Draw() -> jobsys::Job* {
		return jobsys::make_job(GPUNode::DrawJmp, std::tuple{this}); }
	static void DrawJmp(jobsys::Job*, unsigned threadId [[maybe_unused]], std::tuple<GPUNode*>* data) {
		auto& [self] = *data;
		self->DrawImpl(); }
	void DrawImpl() {
		pcnt_ = static_cast<int>(layers_.size());
		jobsys::Job *postJob = Post();
		AddLinksTo(postJob);
		for (auto layer : layers_) {
			layer->Render(0, &gpu_.IC(), targetSizeInPx_, aspect_); }
		for (auto layer : layers_) {
			layer->Render(1, &gpu_.IC(), targetSizeInPx_, aspect_); }
		jobsys::run(postJob); }

	static
	void AllThen(jobsys::Job*, unsigned threadId [[maybe_unused]], std::tuple<std::atomic<int>*, jobsys::Job*>* data) {
		auto [cnt, link] = *data;
		auto& counter = *cnt;
		if (--counter != 0) {
			return; }
		jobsys::run(link); }

	auto Post() -> jobsys::Job* {
		return jobsys::make_job(GPUNode::PostJmp, std::tuple{this}); }
	static void PostJmp(jobsys::Job*, unsigned threadId [[maybe_unused]], std::tuple<GPUNode*>* data) {
		auto& [self] = *data;
		self->Post(); }
	void PostImpl() {}

	auto IC() -> rglv::GL& override {
		return gpu_.IC(); }

	auto Render() -> jobsys::Job* override {
		return gpu_.Run(); }

	/**
	 * only valid after Main()!
	 */
	auto GetTargetSize() const -> rmlv::ivec2 override {
		return targetSizeInPx_; }

	auto GetAA() const -> bool override {
		return aa_; }};


class Compiler final : public NodeCompiler {
	void Build() override {
		if (auto jv = rclx::jv_find(data_, "layers", JSON_ARRAY)) {
			for (const auto& item : *jv) {
				if (item->value.getTag() == JSON_STRING) {
					inputs_.emplace_back("layer", item->value.toString()); }}}

		auto aa = DataBool("aa", false);

		if (!Input("tileSize", /*required=*/true)) {
			inputs_.emplace_back("tileSize", "globals:tileSize"); }

		if (!Input("targetSize", /*required=*/true)) { return; }

		Input("aspect", /*required=*/false);

		out_ = std::make_shared<GPUNode>(id_, std::move(inputs_), aa); }};


struct init { init() {
	NodeRegistry::GetInstance().Register("$gpu", [](){ return std::make_unique<Compiler>(); });
}} init{};


}  // close unnamed namespace
