#include "src/rcl/rclma/rclma_framepool.hxx"
#include "src/rcl/rclmt/rclmt_jobsys.hxx"
#include "src/rcl/rclx/rclx_gason_util.hxx"
#include "src/rgl/rglv/rglv_gl.hxx"
#include "src/rml/rmlm/rmlm_mat4.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"
#include "src/viewer/compile.hxx"
#include "src/viewer/node/base.hxx"
#include "src/viewer/node/i_gl.hxx"
#include "src/viewer/node/i_value.hxx"

#include <map>
#include <string>
#include <string_view>
#include <utility>

namespace {

using namespace rqdq;
using namespace rqv;
namespace jobsys = rclmt::jobsys;

constexpr float kTau = static_cast<float>(rmlv::M_PI * 2.0);
// constexpr int kForkUntilDepth = 100;

class RepeatOp final : public IGl {
	// config
	int cnt_;
	bool fork_;
	/*rmlv::vec3 translateFixed_{};
	rmlv::vec3 rotateFixed_{};
	rmlv::vec3 scaleFixed_{};*/

	// input
	IGl *lowerNode_{nullptr};
	std::string scaleSlot_{"default"};
	IValue* scaleNode_{nullptr};
	std::string rotateSlot_{"default"};
	IValue* rotateNode_{nullptr};
	std::string translateSlot_{"default"};
	IValue* translateNode_{nullptr};

public:
	RepeatOp(std::string_view id, InputList inputs, int cnt, bool fork) :
		NodeBase(id, std::move(inputs)),
		IGl(),
		cnt_(cnt),
		fork_(fork)
		{}

	auto Connect(std::string_view attr, NodeBase* other, std::string_view slot) -> bool override {
		if (attr == "gl") {
			lowerNode_ = dynamic_cast<IGl*>(other);
			if (lowerNode_ == nullptr) {
				TYPE_ERROR(IGl);
				return false; }
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
		return IGl::Connect(attr, other, slot); }

	void DisconnectAll() override {
		IGl::DisconnectAll();
		lowerNode_ = nullptr;
		scaleNode_ = nullptr;
		rotateNode_ = nullptr;
		translateNode_ = nullptr; }

	void AddDeps() override {
		IGl::AddDeps();
		AddDep(lowerNode_); }

	void Main() override {
		auto* my_noop = jobsys::make_job(jobsys::noop);
		AddLinksTo(my_noop);
		lowerNode_->AddLink(my_noop);
		lowerNode_->Run(); }

	void DrawDepth(rglv::GL* dc, const rmlm::mat4* pmat, const rmlm::mat4* mvmat) override {
		using rmlm::mat4;

		auto translate = rmlv::vec3{0,0,0};
		if (translateNode_ != nullptr) {
			translate = translateNode_->Eval(translateSlot_).as_vec3(); }
		auto rotate = rmlv::vec3{0,0,0};
		if (rotateNode_ != nullptr) {
			rotate = rotateNode_->Eval(rotateSlot_).as_vec3(); }
		auto scale = rmlv::vec3{1,1,1};
		if (scaleNode_ != nullptr) {
			scale = scaleNode_->Eval(scaleSlot_).as_vec3(); }

		if (cnt_ == 0) {
			return; }


		rmlv::vec3 p{0,0,0};
		rmlv::vec3 r{0,0,0};
		rmlv::vec3 s{1,1,1};
		for (int i=0; i<cnt_; ++i, p+=translate, r+=rotate, s*=scale) {
			auto& M = *static_cast<mat4*>(rclma::framepool::Allocate(64));
			M = *mvmat;
			M = M * mat4::rotate(r.x * kTau, 1, 0, 0);
			M = M * mat4::rotate(r.y * kTau, 0, 1, 0);
			M = M * mat4::rotate(r.z * kTau, 0, 0, 1);
			M = M * mat4::translate(p);
			M = M * mat4::scale(s);

			lowerNode_->DrawDepth(dc, pmat, &M); }}

	void Draw(int pass, const LightPack& lights, rglv::GL* dc, const rmlm::mat4* pmat, const rmlm::mat4* vmat, const rmlm::mat4* mmat) override {
		using rmlm::mat4;

		auto translate = rmlv::vec3{0,0,0};
		if (translateNode_ != nullptr) {
			translate = translateNode_->Eval(translateSlot_).as_vec3(); }
		auto rotate = rmlv::vec3{0,0,0};
		if (rotateNode_ != nullptr) {
			rotate = rotateNode_->Eval(rotateSlot_).as_vec3(); }
		auto scale = rmlv::vec3{1,1,1};
		if (scaleNode_ != nullptr) {
			scale = scaleNode_->Eval(scaleSlot_).as_vec3(); }

		if (cnt_ == 0) {
			return; }

		auto root = jobsys::make_job(jobsys::noop);
		jobsys::Job* ptrs[128];  // XXX
		int ptrcnt{0};

		rmlv::vec3 p{0,0,0};
		rmlv::vec3 r{0,0,0};
		rmlv::vec3 s{1,1,1};

		for (int i=0; i<cnt_; ++i, p+=translate, r+=rotate, s*=scale) {
			auto& M = *static_cast<mat4*>(rclma::framepool::Allocate(64));
			M = *mmat;
			M = M * mat4::rotate(r.x * kTau, 1, 0, 0);
			M = M * mat4::rotate(r.y * kTau, 0, 1, 0);
			M = M * mat4::rotate(r.z * kTau, 0, 0, 1);
			M = M * mat4::translate(p);
			M = M * mat4::scale(s);

			if (fork_) {
				ptrs[ptrcnt++] = DrawLower(root, pass, lights, dc, pmat, vmat, &M); }
			else {
				lowerNode_->Draw(pass, lights, dc, pmat, vmat, &M); }}

		if (fork_) {
			for (int i = 0; i < ptrcnt; ++i) {
				jobsys::run(ptrs[i]); }
			jobsys::run(root);
			jobsys::wait(root); }}

	static 
	void AfterDraw(jobsys::Job*, unsigned threadId [[maybe_unused]], std::tuple<std::atomic<int>*, jobsys::Job*>* data) {
		auto [cnt, link] = *data;
		auto& counter = *cnt;
		if (--counter == 0) {
			jobsys::run(link); }}

	auto DrawLower(jobsys::Job* p, int pass, const LightPack& lights, rglv::GL* dc, const rmlm::mat4* const pmat, const rmlm::mat4* vmat, const rmlm::mat4* mmat) -> jobsys::Job* {
		return jobsys::make_job_as_child(p, DrawLowerJmp, std::tuple{this, pass, &lights, dc, pmat, vmat, mmat}); }
	static
	void DrawLowerJmp(jobsys::Job*, unsigned threadId [[maybe_unused]], std::tuple<RepeatOp*, int, const LightPack&, rglv::GL*, const rmlm::mat4* const, const rmlm::mat4* const, const rmlm::mat4* const>* data) {
		auto [self, pass, lights, dc, pmat, vmat, mmat] = *data;
		self->lowerNode_->Draw(pass, lights, dc, pmat, vmat, mmat); }};


class RepeatCompiler final : public NodeCompiler {
	void Build() override {
		using namespace rclx;
		if (!Input("rotate", /*required=*/false)) { return; }
		if (!Input("scale", /*required=*/false)) { return; }
		if (!Input("translate", /*required=*/false)) { return; }
		if (!Input("gl", /*required=*/true)) { return; }

		auto many = DataInt("many", 1);
		auto fork = DataBool("fork", false);

		out_ = std::make_shared<RepeatOp>(id_, std::move(inputs_), many, fork); }};


struct init { init() {
	NodeRegistry::GetInstance().Register("$repeat", [](){ return std::make_unique<RepeatCompiler>(); });
}} init{};


}  // close unnamed namespace
