#include <atomic>
#include <memory>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

#include "src/rcl/rclma/rclma_framepool.hxx"
#include "src/rcl/rclmt/rclmt_jobsys.hxx"
#include "src/rcl/rclx/rclx_gason_util.hxx"
#include "src/rgl/rglv/rglv_gl.hxx"
#include "src/rml/rmlm/rmlm_mat4.hxx"
#include "src/viewer/compile.hxx"
#include "src/viewer/node/base.hxx"
#include "src/viewer/node/i_gl.hxx"
#include "src/viewer/node/i_value.hxx"

namespace rqdq {
namespace {

using namespace rqv;
namespace jobsys = rclmt::jobsys;

constexpr float kTau = static_cast<float>(rmlv::M_PI * 2.0);

class Impl final : public IGl {

	std::vector<IGl*> gls_;
	std::string scaleSlot_{"default"};
	IValue* scaleNode_{nullptr};
	std::string rotateSlot_{"default"};
	IValue* rotateNode_{nullptr};
	std::string translateSlot_{"default"};
	IValue* translateNode_{nullptr};
	std::string enableSlot_{"default"};
	IValue* enableNode_{nullptr};

public:
	Impl(std::string_view id, InputList inputs) :
		NodeBase(id, std::move(inputs)),
		IGl() {}

	auto Connect(std::string_view attr, NodeBase* other, std::string_view slot) -> bool override {
		if (attr == "gl") {
			IGl* tmp = dynamic_cast<IGl*>(other);
			if (tmp == nullptr) {
				TYPE_ERROR(IGl);
				return false; }
			gls_.push_back(tmp);
			return true; }
		if (attr == "scale") {
			scaleNode_ = dynamic_cast<IValue*>(other);
			scaleSlot_ = slot;
			if (scaleNode_ == nullptr) {
				TYPE_ERROR(IValue);
				return false; }
			return true; }
		if (attr == "rotate") {
			rotateNode_ = dynamic_cast<IValue*>(other);
			rotateSlot_ = slot;
			if (rotateNode_ == nullptr) {
				TYPE_ERROR(IValue);
				return false; }
			return true; }
		if (attr == "translate") {
			translateNode_ = dynamic_cast<IValue*>(other);
			translateSlot_ = slot;
			if (translateNode_ == nullptr) {
				TYPE_ERROR(IValue);
				return false; }
			return true; }
		if (attr == "enable") {
			enableNode_ = dynamic_cast<IValue*>(other);
			enableSlot_ = slot;
			if (enableNode_ == nullptr) {
				TYPE_ERROR(IValue);
				return false; }
			return true; }
		return IGl::Connect(attr, other, slot); }

	void DisconnectAll() override {
		IGl::DisconnectAll();
		gls_.clear();
		scaleNode_ = nullptr;
		rotateNode_ = nullptr;
		translateNode_ = nullptr;
		enableNode_ = nullptr; }

	void AddDeps() override {
		IGl::AddDeps();
		if (Enabled()) {
			for (auto item : gls_) { AddDep(item); }}}

	void Main() override {
		auto doneJob = jobsys::make_job(jobsys::noop);
		AddLinksTo(doneJob);
		if (Enabled() && (gls_.size()>0)) {
			for (auto glnode : gls_) { glnode->AddLink(AfterAll(doneJob)); }
			for (auto glnode : gls_) { glnode->Run(); }}
		else {
			jobsys::run(doneJob); }}

	// --- IGl ---
	auto Lights(rmlm::mat4 mvmat) -> LightPack override {
		LightPack ax;
		for (auto glnode : gls_) {
			ax = Merge(ax, glnode->Lights(Apply(mvmat))); }
		return ax; }

	void DrawDepth(rglv::GL* dc, const rmlm::mat4* pmat, const rmlm::mat4* mvmat) override {
		if (!Enabled()) return;
		auto M = static_cast<rmlm::mat4*>(rclma::framepool::Allocate(64));
		*M = Apply(*mvmat);
		for (auto glnode : gls_) { glnode->DrawDepth(dc, pmat, M); }}

	void Draw(int pass, const LightPack& lights, rglv::GL* dc, const rmlm::mat4* pmat, const rmlm::mat4* vmat, const rmlm::mat4* mmat) override {
		if (!Enabled()) return;
		auto M = static_cast<rmlm::mat4*>(rclma::framepool::Allocate(64));
		*M = Apply(*mmat);
		for (auto glnode : gls_) { glnode->Draw(pass, lights, dc, pmat, vmat, M); }}

private:
	auto Enabled() -> bool {
		if (enableNode_) {
			return enableNode_->Eval(enableSlot_).as_float() >= 1.0F; }
		return true; }

	auto Apply(rmlm::mat4 a) -> rmlm::mat4 {
		using rmlm::mat4;
		if (translateNode_) {
			auto amt = translateNode_->Eval(translateSlot_).as_vec3();
			a = a * mat4::translate(amt); }

		if (rotateNode_) {
			auto amt = rotateNode_->Eval(rotateSlot_).as_vec3();
			a = a * mat4::rotate(amt.z * kTau, 0, 0, 1);
			a = a * mat4::rotate(amt.y * kTau, 0, 1, 0);
			a = a * mat4::rotate(amt.x * kTau, 1, 0, 0); }

		if (scaleNode_) {
			auto amt = scaleNode_->Eval(scaleSlot_).as_vec3();
			a = a * mat4::scale(amt); }

		return a; } };


class Compiler final : public NodeCompiler {
	void Build() override {
		if (auto jv = rclx::jv_find(data_, "gl", JSON_ARRAY)) {
			for (const auto& item : *jv) {
				if (item->value.getTag() == JSON_STRING) {
					inputs_.emplace_back("gl", item->value.toString()); } } }
		if (auto jv = rclx::jv_find(data_, "gl", JSON_STRING)) {
			inputs_.emplace_back("gl", jv->toString()); }

		if (!Input("rotate", /*required=*/false)) { return; }
		if (!Input("scale", /*required=*/false)) { return; }
		if (!Input("translate", /*required=*/false)) { return; }
		if (!Input("enable", /*required=*/false)) { return; }
		out_ = std::make_shared<Impl>(id_, std::move(inputs_)); }};


struct init { init() {
	NodeRegistry::GetInstance().Register("$group", [](){ return std::make_unique<Compiler>(); });
}} init{};


}  // namespace
}  // namespace rqdq
