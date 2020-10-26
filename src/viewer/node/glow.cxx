#include <iostream>
#include <string_view>
#include <tuple>
#include <utility>

#include "src/rcl/rclmt/rclmt_jobsys.hxx"
#include "src/rcl/rclx/rclx_gason_util.hxx"
#include "src/rgl/rglr/rglr_algorithm.hxx"
#include "src/rgl/rglr/rglr_canvas.hxx"
#include "src/rgl/rglr/rglr_canvas_util.hxx"
#include "src/rml/rmlv/rmlv_soa.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"
#include "src/viewer/compile.hxx"
#include "src/viewer/node/base.hxx"
#include "src/viewer/node/i_canvas.hxx"
#include "src/viewer/node/i_output.hxx"

namespace rqdq {
namespace {

using namespace rqv;

struct GlowShader {
	static rmlv::qfloat3 ShadeCanvas(
		rmlv::qfloat2 fragCoord [[maybe_unused]],
		rmlv::qfloat3 c1,
		rmlv::qfloat3 c2) {
		auto& image = c1;
		auto& blur = c2;
		const rmlv::qfloat blurAmt{ 0.700F };
		const rmlv::qfloat brightness{ 0.5F };

		rmlv::qfloat3 out;
		out = (image + blur*blurAmt) * brightness;

		return out; }};


class Impl : public IOutput {
public:
	Impl(std::string_view id, InputList inputs, bool sRGB) :
		IOutput(id, std::move(inputs)),
		sRGB_(sRGB) {}

	bool Connect(std::string_view attr, NodeBase* other, std::string_view slot) override {
		if (attr == "image") {
			imageNode_ = dynamic_cast<ICanvas*>(other);
			if (imageNode_ == nullptr) {
				TYPE_ERROR(IGPU);
				return false; }
			imageSlot_ = slot;
			return true; }
		if (attr == "blur") {
			blurNode_ = dynamic_cast<ICanvas*>(other);
			if (blurNode_ == nullptr) {
				TYPE_ERROR(IGPU);
				return false; }
			blurSlot_ = slot;
			return true; }
		return IOutput::Connect(attr, other, slot); }

	void DisconnectAll() override {
		IOutput::DisconnectAll();
		imageNode_ = nullptr;
		blurNode_ = nullptr; }

	void AddDeps() override {
		IOutput::AddDeps();
		AddDep(imageNode_);
		AddDep(blurNode_); }

	void Reset() override {
		IOutput::Reset();
		outCanvas_ = nullptr; }

	bool IsValid() override {
		if (imageNode_ == nullptr) {
			std::cerr << "glow(" << get_id() << ") has no image" << std::endl;
			return false; }
		if (blurNode_ == nullptr) {
			std::cerr << "glow(" << get_id() << ") has no blur" << std::endl;
			return false; }
		return IOutput::IsValid(); }

	void Main() override {
		auto renderJob = Render();
		imageNode_->AddLink(AfterAll(renderJob));
		blurNode_->AddLink(AfterAll(renderJob));
		imageNode_->Run();
		blurNode_->Run(); }

	rclmt::jobsys::Job* Render() override {
		return rclmt::jobsys::make_job(Impl::RenderJmp, std::tuple{this}); }
	static void RenderJmp(rclmt::jobsys::Job*, unsigned threadId [[maybe_unused]], std::tuple<Impl*> * data) {
		auto&[self] = *data;
		self->RenderImpl(); }
	void RenderImpl() {
		{
			auto tmp = imageNode_->GetCanvas(imageSlot_);
			if (tmp.first != ICanvas::CT_FLOAT4_QUADS) {
				std::cout << "image type is \"" << tmp.first << "\"\n";
				throw std::runtime_error("expected image to be CT_FLOAT4_QUADS"); }
			src0_ = static_cast<const rglr::QFloat4Canvas*>(tmp.second);
			}
		{
			auto tmp = blurNode_->GetCanvas(blurSlot_);
			if (tmp.first != ICanvas::CT_FLOAT4_LINEAR) {
				std::cout << "blur type is \"" << tmp.first << "\"\n";
				throw std::runtime_error("expected blur to be CT_FLOAT4_LINEAR"); }
			src1_ = static_cast<const rglr::FloatingPointCanvas*>(tmp.second);
			}

		int yEnd = src0_->height();
		const int taskSize = 16;

		jobs_.clear();
		auto ppParent = rclmt::jobsys::make_job(rclmt::jobsys::noop);
		for (int y=0; y<yEnd; y+=taskSize) {
			int taskYBegin = y;
			int taskYEnd = std::min(y+taskSize, yEnd);
			jobs_.emplace_back(PP(taskYBegin, taskYEnd, ppParent)); }
		for (auto job : jobs_) {
			run(job); }
		run(ppParent);
		wait(ppParent);

		RunLinks(); }

	rclmt::jobsys::Job* PP(int yBegin, int yEnd, rclmt::jobsys::Job* parent = nullptr) {
		if (parent != nullptr) {
			return rclmt::jobsys::make_job_as_child(parent, Impl::PPJmp, std::tuple{this, yBegin, yEnd}); }
		return rclmt::jobsys::make_job(Impl::PPJmp, std::tuple{this, yBegin, yEnd}); }
	static void PPJmp(rclmt::jobsys::Job*, unsigned threadId [[maybe_unused]], std::tuple<Impl*, int, int>* data) {
		auto& [self, yBegin, yEnd] = *data;
		self->PPImpl(yBegin, yEnd); }
	void PPImpl(int yBegin, int yEnd) {
		int left = 0;
		int top = yBegin;
		int right = src0_->width();
		int bottom = yEnd;
		auto& src0 = *src0_;
		auto& src1 = *src1_;
		auto& out = *outCanvas_;
		rmlg::irect rect{ { left, top }, { right, bottom } };
		if (sRGB_) {
			rglr::Filter<GlowShader, rglr::sRGB>(src0, src1, out, rect); }
		else {
			rglr::Filter<GlowShader, rglr::LinearColor>(src0, src1, out, rect); }}

	void SetOutputCanvas(rglr::TrueColorCanvas* canvas) override {
		outCanvas_ = canvas; }

private:
	// static config
	bool sRGB_;

	// runtime config
	rglr::TrueColorCanvas* outCanvas_{nullptr};

	// inputs
	ICanvas* imageNode_{nullptr};
	std::string imageSlot_{};
	ICanvas* blurNode_{nullptr};
	std::string blurSlot_{};

	const rglr::QFloat4Canvas* src0_{nullptr};
	const rglr::FloatingPointCanvas* src1_{nullptr};
	std::vector<rclmt::jobsys::Job*> jobs_; };


class Compiler final : public NodeCompiler {
	void Build() override {
		using rclx::jv_find;
		if (!Input("image", /*required=*/true)) { return; }
		if (!Input("blur", /*required=*/true)) { return; }

		bool srgb{true};
		if (auto jv = jv_find(data_, "sRGB", JSON_FALSE)) {
			srgb = false; }

		out_ = std::make_shared<Impl>(id_, std::move(inputs_), srgb); }};


struct init { init() {
	NodeRegistry::GetInstance().Register("$glow", [](){ return std::make_unique<Compiler>(); });
}} init{};


}  // namespace
}  // namespace rqdq
