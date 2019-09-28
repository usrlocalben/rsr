#include <atomic>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <tuple>

#include "src/rcl/rclma/rclma_framepool.hxx"
#include "src/rcl/rclmt/rclmt_jobsys.hxx"
#include "src/rcl/rclx/rclx_gason_util.hxx"
#include "src/rgl/rglv/rglv_gl.hxx"
#include "src/rml/rmlm/rmlm_mat4.hxx"
#include "src/viewer/compile.hxx"
#include "src/viewer/node/base.hxx"
#include "src/viewer/node/i_camera.hxx"
#include "src/viewer/node/i_gl.hxx"
#include "src/viewer/node/i_layer.hxx"
#include "src/viewer/node/i_value.hxx"

namespace rqdq {
namespace {

using namespace rqv;

namespace jobsys = rclmt::jobsys;

class Impl : public ILayer {
public:
	using ILayer::ILayer;

	bool Connect(std::string_view attr, NodeBase* other, std::string_view slot) override {
		if (attr == "camera") {
			cameraNode_ = dynamic_cast<ICamera*>(other);
			if (cameraNode_ == nullptr) {
				TYPE_ERROR(ICamera);
				return false; }
			return true; }
		if (attr == "color") {
			colorNode_ = dynamic_cast<IValue*>(other);
			colorSlot_ = slot;
			if (colorNode_ == nullptr) {
				TYPE_ERROR(IValue);
				return false; }
			return true; }
		if (attr == "gl") {
			IGl* tmp = dynamic_cast<IGl*>(other);
			if (tmp == nullptr) {
				TYPE_ERROR(IGl);
				return false; }
			gls_.push_back(tmp);
			return true; }
		return ILayer::Connect(attr, other, slot); }

	void Main() override {
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

	static void AllThen(rclmt::jobsys::Job* job, unsigned tid, std::tuple<std::atomic<int>*, rclmt::jobsys::Job*>* data) {
		auto [cnt, link] = *data;
		auto& counter = *cnt;
		if (--counter != 0) {
			return; }
		jobsys::run(link); }

	rmlv::vec3 GetBackgroundColor() override {
		auto color = rmlv::vec3{0.0F};
		if (colorNode_ != nullptr) {
			color = colorNode_->Eval(colorSlot_).as_vec3(); }
		return color; }

	void Render(rglv::GL* dc, int width, int height, float aspect, rclmt::jobsys::Job *link) override {
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

		for (auto gl : gls_) {
			gl->Draw(dc, pmat, mvmat, nullptr, 0);}
		if (link != nullptr) {
			jobsys::run(link); }}

protected:
	void AddDeps() override {
		ILayer::AddDeps();
		AddDep(cameraNode_);
		AddDep(colorNode_); }

private:
	rclmt::jobsys::Job* Post() {
		return rclmt::jobsys::make_job(Impl::PostJmp, std::tuple{this}); }
	static void PostJmp(rclmt::jobsys::Job* job, const unsigned tid, std::tuple<Impl*>* data) {
		auto& [self] = *data;
		self->Post(); }
	void PostImpl() {}

private:
	std::vector<IGl*> gls_{};
	ICamera* cameraNode_{nullptr};
	IValue* colorNode_{nullptr};
	std::string colorSlot_{}; };


class Compiler final : public NodeCompiler {
	void Build() override {
		if (!Input("camera", /*required=*/true)) { return; }
		if (!Input("color", /*required=*/false)) { return; }

		if (auto jv = rclx::jv_find(data_, "gl", JSON_ARRAY)) {
			for (const auto& item : *jv) {
				if (item->value.getTag() == JSON_STRING) {
					inputs_.emplace_back("gl", item->value.toString()); } } }
		out_ = std::make_shared<Impl>(id_, std::move(inputs_)); }};


struct init { init() {
	NodeRegistry::GetInstance().Register("$layer", [](){ return std::make_unique<Compiler>(); });
}} init{};


}  // namespace
}  // namespace rqdq
