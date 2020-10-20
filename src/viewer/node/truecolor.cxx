#include <iostream>
#include <string_view>
#include <tuple>
#include <utility>

#include "src/rcl/rclmt/rclmt_jobsys.hxx"
#include "src/rcl/rclx/rclx_gason_util.hxx"
#include "src/rgl/rglr/rglr_canvas.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"
#include "src/viewer/compile.hxx"
#include "src/viewer/node/base.hxx"
#include "src/viewer/node/i_gpu.hxx"
#include "src/viewer/node/i_output.hxx"
#include "src/viewer/shaders.hxx"

namespace rqdq {
namespace {

using namespace rqv;

class Impl : public IOutput {
public:
	Impl(std::string_view id, InputList inputs, int programId, bool sRGB)
		:IOutput(id, std::move(inputs)), programId_(programId), sRGB_(sRGB) {}

	bool Connect(std::string_view attr, NodeBase* other, std::string_view slot) override {
		if (attr == "gpu") {
			gpuNode_ = dynamic_cast<IGPU*>(other);
			if (gpuNode_ == nullptr) {
				TYPE_ERROR(IGPU);
				return false; }
			return true; }
		return IOutput::Connect(attr, other, slot); }

	void AddDeps() override {
		IOutput::AddDeps();
		AddDep(gpuNode_); }

	void Reset() override {
		IOutput::Reset();
		outCanvas_ = nullptr; }

	bool IsValid() override {
		if (outCanvas_ == nullptr) {
			std::cerr << "truecolor(" << get_id() << ") has no output set" << std::endl;
			return false; }
		if (gpuNode_ == nullptr) {
			std::cerr << "truecolor(" << get_id() << ") has no gpu" << std::endl;
			return false; }
		return IOutput::IsValid(); }

	void Main() override {
		gpuNode_->AddLink(AfterAll(Render()));
		gpuNode_->Run(); }

	rclmt::jobsys::Job* Render() override {
		return rclmt::jobsys::make_job(Impl::RenderJmp, std::tuple{this}); }
	static void RenderJmp(rclmt::jobsys::Job*, unsigned threadId [[maybe_unused]], std::tuple<Impl*> * data) {
		auto&[self] = *data;
		self->RenderImpl(); }
	void RenderImpl() {
		auto& ic = gpuNode_->IC();
		if (outCanvas_ != nullptr) {
			ic.UseProgram(int(programId_));
			if (gpuNode_->GetAA()) {
				throw std::runtime_error("truecolor aa not implemented"); }
			else {
				ic.StoreColor(outCanvas_, sRGB_); }}
		ic.Finish();
		auto renderJob = gpuNode_->Render();
		AddLinksTo(renderJob);
		rclmt::jobsys::run(renderJob); }

	void SetOutputCanvas(rglr::TrueColorCanvas* canvas) override {
		outCanvas_ = canvas; }

private:
	// static config
	int programId_;
	bool sRGB_;

	// runtime config
	rglr::TrueColorCanvas* outCanvas_{nullptr};

	// inputs
	IGPU* gpuNode_{nullptr}; };


class Compiler final : public NodeCompiler {
	void Build() override {
		using rclx::jv_find;
		if (!Input("gpu", /*required=*/true)) { return; }

		int programId = 1;
		if (auto jv = jv_find(data_, "program", JSON_STRING)) {
			programId = ShaderProgramNameSerializer::Deserialize(jv->toString()); }

		bool srgb{true};
		if (auto jv = jv_find(data_, "sRGB", JSON_FALSE)) {
			srgb = false; }

		out_ = std::make_shared<Impl>(id_, std::move(inputs_), programId, srgb); }};



struct init { init() {
	NodeRegistry::GetInstance().Register("$truecolor", [](){ return std::make_unique<Compiler>(); });
}} init{};


}  // namespace
}  // namespace rqdq
