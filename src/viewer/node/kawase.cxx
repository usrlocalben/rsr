#include <iostream>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <utility>

#include "src/rcl/rclmt/rclmt_jobsys.hxx"
#include "src/rcl/rclx/rclx_gason_util.hxx"
#include "src/rgl/rglr/rglr_canvas.hxx"
#include "src/rgl/rglr/rglr_kawase.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"
#include "src/viewer/compile.hxx"
#include "src/viewer/node/base.hxx"
#include "src/viewer/node/i_canvas.hxx"

namespace rqdq {
namespace {

using namespace rqv;

class Impl : public ICanvas {
public:
	Impl(std::string_view id, InputList inputs, int intensity, int taskSize) :
		ICanvas(id, std::move(inputs)),
		intensity_(intensity),
		taskSize_(taskSize) {}

	bool Connect(std::string_view attr, NodeBase* other, std::string_view slot) override {
		if (attr == "input") {
			inputNode_ = dynamic_cast<ICanvas*>(other);
			if (inputNode_ == nullptr) {
				TYPE_ERROR(ICanvas);
				return false; }
			inputSlot_ = slot;
			return true; }
		return ICanvas::Connect(attr, other, slot); }

	void AddDeps() override {
		ICanvas::AddDeps();
		AddDep(inputNode_); }

	bool IsValid() override {
		if (inputNode_ == nullptr) {
			std::cerr << "kawase(" << get_id() << ") has no input" << std::endl;
			return false; }
		return ICanvas::IsValid(); }

	void Main() override {
		inputNode_->AddLink(AfterAll(Render()));
		inputNode_->Run(); }

	rclmt::jobsys::Job* Render() {
		return rclmt::jobsys::make_job(Impl::RenderJmp, std::tuple{this}); }
	static void RenderJmp(rclmt::jobsys::Job*, unsigned threadId [[maybe_unused]], std::tuple<Impl*> * data) {
		auto&[self] = *data;
		self->RenderImpl(); }
	void RenderImpl() {
		auto tmp = inputNode_->GetCanvas(inputSlot_);
		if (tmp.first != ICanvas::CT_FLOAT4_LINEAR) {
			throw std::runtime_error("blur requires a FloatingPointCanvas"); }
		const auto* input = static_cast<const rglr::FloatingPointCanvas*>(tmp.second);

		if (intensity_ == 0) {
			output_ = const_cast<rglr::FloatingPointCanvas*>(input);
			RunLinks();
			return; }

		// siggraph2015-mmg-marius-notes.pdf
		// 97px radius = 0, 1, 2, 3, 4, 4, 5, 6, 7

		// https://software.intel.com/en-us/blogs/2014/07/15/an-investigation-of-fast-real-time-gpu-based-image-blur-algorithms
		// 35px radius = 0, 1, 2, 2, 3

		dist_ = 0;
		bool first{true};
		src_ = input;
		dst_ = &ca_;
		ca_.resize(input->width(), input->height());
		if (intensity_ > 1) {
			cb_.resize(input->width(), input->height()); }
		const int yEnd = input->height();

		for (; dist_<intensity_; ++dist_) {
			auto blurParent = rclmt::jobsys::make_job(rclmt::jobsys::noop);
			jobs_.clear();
			for (int y=0; y<yEnd; y+=taskSize_) {
				int taskYBegin = y;
				int taskYEnd = std::min(y+taskSize_, yEnd);
				jobs_.emplace_back(BlurLines(taskYBegin, taskYEnd, blurParent)); }
			for (auto job : jobs_) {
				run(job); }
			run(blurParent);
			wait(blurParent);

			if (first) {
				first = false;
				src_ = &cb_; }

			{auto* tmp = const_cast<rglr::FloatingPointCanvas*>(src_);
			src_ = dst_;
			dst_ = tmp;}}

		output_ = src_;
		RunLinks(); }

	rclmt::jobsys::Job* BlurLines(int yBegin, int yEnd, rclmt::jobsys::Job* parent=nullptr) {
		if (parent) {
			return rclmt::jobsys::make_job_as_child(parent, Impl::BlurLinesJmp, std::tuple{this, yBegin, yEnd}); }
		else {
			return rclmt::jobsys::make_job(Impl::BlurLinesJmp, std::tuple{this, yBegin, yEnd}); }}
	static void BlurLinesJmp(rclmt::jobsys::Job*, unsigned threadId [[maybe_unused]], std::tuple<Impl*, int, int>* data) {
		auto&[self, yBegin, yEnd] = *data;
		self->BlurLinesImpl(yBegin, yEnd); }
	void BlurLinesImpl(int yBegin, int yEnd) {
		KawaseBlurFilter(*src_, *dst_, dist_, yBegin, yEnd); }

    std::pair<int, const void*> GetCanvas(std::string_view slot [[maybe_unused]]) override {
        return { ICanvas::CT_FLOAT4_LINEAR, output_ }; }

private:
	const int intensity_;
	const int taskSize_;

	// inputs
	ICanvas* inputNode_{nullptr};
	std::string inputSlot_{};

	std::vector<rclmt::jobsys::Job*> jobs_;
	rglr::FloatingPointCanvas ca_;
	rglr::FloatingPointCanvas cb_;
	const rglr::FloatingPointCanvas* src_;
	rglr::FloatingPointCanvas* dst_;
	const rglr::FloatingPointCanvas* output_;
	int dist_; };


class Compiler final : public NodeCompiler {
	void Build() override {
		if (!Input("input", /*required=*/true)) { return; }

		int intensity{1};
		if (auto jv = rclx::jv_find(data_, "intensity", JSON_NUMBER)) {
			intensity = (int)jv->toNumber(); }

		int taskSize{16};
		if (auto jv = rclx::jv_find(data_, "taskSize", JSON_NUMBER)) {
			taskSize = (int)jv->toNumber(); }

		out_ = std::make_shared<Impl>(id_, std::move(inputs_), intensity, taskSize); }};


struct init { init() {
	NodeRegistry::GetInstance().Register("$kawase", [](){ return std::make_unique<Compiler>(); });
}} init{};


}  // namespace
}  // namespace rqdq
