#include <map>
#include <string>
#include <string_view>
#include <utility>

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

namespace rqdq {
namespace {

using namespace rqv;

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
		IGl(id, std::move(inputs)),
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

	void AddDeps() override {
		IGl::AddDeps();
		AddDep(lowerNode_); }

	void Main() override {
		namespace jobsys = rclmt::jobsys;
		auto* my_noop = jobsys::make_job(jobsys::noop);
		AddLinksTo(my_noop);
		lowerNode_->AddLink(my_noop);
		lowerNode_->Run(); }

	void Draw(int pass, rglv::GL* dc, const rmlm::mat4* pmat, const rmlm::mat4* mvmat, int depth) override {
		using rmlm::mat4;
		using rmlv::M_PI;
		namespace jobsys = rclmt::jobsys;

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

		auto root = rclmt::jobsys::make_job(rclmt::jobsys::noop);
		jobsys::Job* ptrs[128];  // XXX
		int ptrcnt{0};

		rmlv::vec3 p{0,0,0};
		rmlv::vec3 r{0,0,0};
		rmlv::vec3 s{1,1,1};

		for (int i=0; i<cnt_; ++i, p+=translate, r+=rotate, s*=scale) {
			auto& M = *static_cast<mat4*>(rclma::framepool::Allocate(64));
			M = *mvmat;
			M = M * mat4::rotate(r.x * M_PI * 2, 1, 0, 0);
			M = M * mat4::rotate(r.y * M_PI * 2, 0, 1, 0);
			M = M * mat4::rotate(r.z * M_PI * 2, 0, 0, 1);
			M = M * mat4::translate(p);
			M = M * mat4::scale(s);

			if (fork_) {
				ptrs[ptrcnt++] = DrawLower(root, pass, dc, pmat, &M, depth +1); }
			else {
				lowerNode_->Draw(pass, dc, pmat, &M, depth+1); }}

		if (fork_) {
			for (int i = 0; i < ptrcnt; ++i) {
				jobsys::run(ptrs[i]); }
			jobsys::run(root);
			jobsys::wait(root); }}

	static 
	void AfterDraw(rclmt::jobsys::Job*, unsigned threadId [[maybe_unused]], std::tuple<std::atomic<int>*, rclmt::jobsys::Job*>* data) {
		auto [cnt, link] = *data;
		auto& counter = *cnt;
		if (--counter == 0) {
			rclmt::jobsys::run(link); }}

	auto DrawLower(rclmt::jobsys::Job* p, int pass, rglv::GL* dc, const rmlm::mat4* const pmat, const rmlm::mat4* mvmat, int depth) -> rclmt::jobsys::Job* {
		return rclmt::jobsys::make_job_as_child(p, DrawLowerJmp, std::tuple{this, pass, dc, pmat, mvmat, depth}); }
	static
	void DrawLowerJmp(rclmt::jobsys::Job*, unsigned threadId [[maybe_unused]], std::tuple<RepeatOp*, int, rglv::GL*, const rmlm::mat4* const, const rmlm::mat4* const, int>* data) {
		auto [self, pass, dc, pmat, mvmat, depth] = *data;
		self->lowerNode_->Draw(pass, dc, pmat, mvmat, depth); }};


class TranslateOp final : public IGl {
	IGl *lowerNode_{nullptr};
	std::string scaleSlot_{"default"};
	IValue* scaleNode_{nullptr};
	std::string rotateSlot_{"default"};
	IValue* rotateNode_{nullptr};
	std::string translateSlot_{"default"};
	IValue* translateNode_{nullptr};

public:
	using IGl::IGl;

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

	void AddDeps() override {
		IGl::AddDeps();
		AddDep(lowerNode_); }

	void Main() override {
		auto* my_noop = rclmt::jobsys::make_job(rclmt::jobsys::noop);
		AddLinksTo(my_noop);
		lowerNode_->AddLink(my_noop);
		lowerNode_->Run(); }

	void Draw(int pass, rglv::GL* dc, const rmlm::mat4* pmat, const rmlm::mat4* mvmat, int depth) override {
		using rmlm::mat4;
		using rmlv::M_PI;
		namespace jobsys = rclmt::jobsys;

		auto& M = *static_cast<mat4*>(rclma::framepool::Allocate(64));

		auto scale = rmlv::vec3{1.0F, 1.0F, 1.0F};
		if (scaleNode_ != nullptr) {
			scale = scaleNode_->Eval(scaleSlot_).as_vec3(); }

		auto rotate = rmlv::vec3{0,0,0};
		if (rotateNode_ != nullptr) {
			rotate = rotateNode_->Eval(rotateSlot_).as_vec3(); }

		auto translate = rmlv::vec3{0,0,0};
		if (translateNode_ != nullptr) {
			translate = translateNode_->Eval(translateSlot_).as_vec3(); }

		M = *mvmat;
		M = M * mat4::scale(scale);
		M = M * mat4::rotate(rotate.x * M_PI * 2, 1, 0, 0);
		M = M * mat4::rotate(rotate.y * M_PI * 2, 0, 1, 0);
		M = M * mat4::rotate(rotate.z * M_PI * 2, 0, 0, 1);
		M = M * mat4::translate(translate);
		lowerNode_->Draw(pass, dc, pmat, &M, depth); } };


class TranslateCompiler final : public NodeCompiler {
	void Build() override {
		if (!Input("gl", /*required=*/true)) { return; }
		if (!Input("rotate", /*required=*/false)) { return; }
		if (!Input("scale", /*required=*/false)) { return; }
		if (!Input("translate", /*required=*/false)) { return; }

/*
		map<string, vec3> slot_values = {
			{"translate", vec3{0,0,0}},
			{"rotate", vec3{0,0,0}},
			{"scale", vec3{1,1,1}},
			};

		for (const auto& slot_name : { "translate", "rotate", "scale" }) {
			if (auto jv = jv_find(data_, slot_name)) {
				auto value = jv_decode_ref_or_vec3(*jv);
				if (auto ptr = get_if<string>(&value)) {
					inputs.emplace_back(slot_name, *ptr); }
				else if (auto ptr = get_if<vec3>(&value)) {
					slot_values[slot_name] = *ptr; } } }

		if (auto jv = jv_find(data_, "gl", JSON_STRING)) {
			inputs.emplace_back("gl", jv->toString()); }
*/
		out_ = std::make_shared<TranslateOp>(id_, std::move(inputs_)); }};


class RepeatCompiler final : public NodeCompiler {
	void Build() override {
		using namespace rclx;
		int many{1};
		bool fork{false};
		if (!Input("rotate", /*required=*/false)) { return; }
		if (!Input("scale", /*required=*/false)) { return; }
		if (!Input("translate", /*required=*/false)) { return; }
		if (!Input("gl", /*required=*/true)) { return; }
		if (auto jv = jv_find(data_, "many", JSON_NUMBER)) {
			many = static_cast<int>(jv->toNumber()); }
		if (auto jv = jv_find(data_, "fork", JSON_TRUE)) {
			fork = true; }
		out_ = std::make_shared<RepeatOp>(id_, std::move(inputs_), many, fork); }};


struct init { init() {
	NodeRegistry::GetInstance().Register("$repeat", [](){ return std::make_unique<RepeatCompiler>(); });
	NodeRegistry::GetInstance().Register("$translate", [](){ return std::make_unique<TranslateCompiler>(); });
}} init{};


}  // namespace
}  // namespace rqdq
