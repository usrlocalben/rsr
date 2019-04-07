#include "translate.hxx"

#include <map>
#include <string>
#include <string_view>

#include "src/rcl/rclx/rclx_gason_util.hxx"
#include "src/viewer/compile.hxx"

namespace rqdq {
namespace rqv {

void RepeatOp::Connect(std::string_view attr, NodeBase* other, std::string_view slot) {
	if (attr == "gl") {
		lowerNode_ = static_cast<GlNode*>(other);
		return; }
	if (attr == "scale") {
		scaleNode_ = static_cast<ValuesBase*>(other);
		scaleSlot_ = slot;
		return; }
	if (attr == "rotate") {
		rotateNode_ = static_cast<ValuesBase*>(other);
		rotateSlot_ = slot;
		return; }
	if (attr == "translate") {
		translateNode_ = static_cast<ValuesBase*>(other);
		translateSlot_ = slot;
		return; }
	GlNode::Connect(attr, other, slot); }


void RepeatOp::AddDeps() {
	GlNode::AddDeps();
	AddDep(lowerNode_); }


void RepeatOp::Main() {
	namespace jobsys = rclmt::jobsys;
	auto* my_noop = jobsys::make_job(jobsys::noop);
	AddLinksTo(my_noop);
	lowerNode_->AddLink(my_noop);
	lowerNode_->Run(); }


void RepeatOp::Draw(rglv::GL* dc, const rmlm::mat4* const pmat, const rmlm::mat4* const mvmat, rclmt::jobsys::Job *link, int depth) {
	using rmlm::mat4;
	using rmlv::M_PI;
	namespace jobsys = rclmt::jobsys;
	namespace framepool = rclma::framepool;

	auto translate = translateFixed_;
	if (translateNode_ != nullptr) {
		translate = translateNode_->Get(translateSlot_).as_vec3(); }
	auto rotate = rotateFixed_;
	if (rotateNode_ != nullptr) {
		rotate = rotateNode_->Get(rotateSlot_).as_vec3(); }
	auto scale = scaleFixed_;
	if (scaleNode_ != nullptr) {
		scale = scaleNode_->Get(scaleSlot_).as_vec3(); }

	if (cnt_ == 0) {
		rclmt::jobsys::run(link);
		return; }

	rmlv::vec3 p{0,0,0};
	rmlv::vec3 r{0,0,0};
	rmlv::vec3 s{1,1,1};
	auto& counter = *reinterpret_cast<std::atomic<int>*>(framepool::Allocate(sizeof(std::atomic<int>)));
	counter = cnt_;
	for (int i = 0; i < cnt_; i++) {
		if (depth < DEPTH_FORK_UNTIL) {
			mat4& M = *reinterpret_cast<mat4*>(framepool::Allocate(64));
			M = *mvmat  * mat4::rotate(r.x * M_PI * 2, 1, 0, 0);
			M = M * mat4::rotate(r.y * M_PI * 2, 0, 1, 0);
			M = M * mat4::rotate(r.z * M_PI * 2, 0, 0, 1);
			M = M * mat4::translate(p);
			M = M * mat4::scale(s);
			jobsys::Job* then = jobsys::make_job(AfterDraw, std::tuple{&counter, link});
			jobsys::run(DrawLower(dc, pmat, &M, then, depth+1)); }
		else {
			mat4 M;
			M = *mvmat * mat4::rotate(r.x * M_PI * 2, 1, 0, 0);
			M = M * mat4::rotate(r.y * M_PI * 2, 0, 1, 0);
			M = M * mat4::rotate(r.z * M_PI * 2, 0, 0, 1);
			M = M * mat4::translate(p);
			M = M * mat4::scale(s);
			lowerNode_->Draw(dc, pmat, &M, nullptr, depth + 1); }
		p += translate;
		r += rotate;
		s *= scale; }
	if (depth >= DEPTH_FORK_UNTIL) {
		jobsys::run(link); } }


void TranslateOp::Connect(std::string_view attr, NodeBase* other, std::string_view slot) {
	if (attr == "gl") {
		lowerNode_ = static_cast<GlNode*>(other);
		return; }
	if (attr == "scale") {
		scaleNode_ = static_cast<ValuesBase*>(other);
		scaleSlot_ = slot;
		return; }
	if (attr == "rotate") {
		rotateNode_ = static_cast<ValuesBase*>(other);
		rotateSlot_ = slot;
		return; }
	if (attr == "translate") {
		translateNode_ = static_cast<ValuesBase*>(other);
		translateSlot_ = slot;
		return; }
	GlNode::Connect(attr, other, slot); }


void TranslateOp::AddDeps() {
	GlNode::AddDeps();
	AddDep(lowerNode_); }


void TranslateOp::Main() {
	auto* my_noop = rclmt::jobsys::make_job(rclmt::jobsys::noop);
	AddLinksTo(my_noop);
	lowerNode_->AddLink(my_noop);
	lowerNode_->Run(); }


void TranslateOp::Draw(rglv::GL* dc, const rmlm::mat4* const pmat, const rmlm::mat4* const mvmat, rclmt::jobsys::Job *link, int depth) {
	using rmlm::mat4;
	using rmlv::M_PI;
	namespace framepool = rclma::framepool;
	namespace jobsys = rclmt::jobsys;

	mat4& M = *reinterpret_cast<mat4*>(framepool::Allocate(64));

	auto scale = rmlv::vec3{1.0F, 1.0F, 1.0F};
	if (scaleNode_ != nullptr) {
		scale = scaleNode_->Get(scaleSlot_).as_vec3(); }

	auto rotate = rmlv::vec3{0,0,0};
	if (rotateNode_ != nullptr) {
		rotate = rotateNode_->Get(rotateSlot_).as_vec3(); }

	auto translate = rmlv::vec3{0,0,0};
	if (translateNode_ != nullptr) {
		translate = translateNode_->Get(translateSlot_).as_vec3(); }

	M = *mvmat * mat4::scale(scale);
	M = M * mat4::rotate(rotate.x * M_PI * 2, 1, 0, 0);
	M = M * mat4::rotate(rotate.y * M_PI * 2, 0, 1, 0);
	M = M * mat4::rotate(rotate.z * M_PI * 2, 0, 0, 1);
	M = M * mat4::translate(translate);
	lowerNode_->Draw(dc, pmat, &M, link, depth); }

}  // namespace rqv

namespace {

using namespace rqv;

class TranslateCompiler final : public NodeCompiler {
	void Build() override {
		if (!Input("GlNode", "gl", /*required=*/true)) { return; }
		if (!Input("ValuesBase", "rotate", /*required=*/false)) { return; }
		if (!Input("ValuesBase", "scale", /*required=*/false)) { return; }
		if (!Input("ValuesBase", "translate", /*required=*/false)) { return; }

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
		std::map<std::string, rmlv::vec3> slotValues = {
			{"translate", rmlv::vec3{0,0,0}},
			{"rotate", rmlv::vec3{0,0,0}},
			{"scale", rmlv::vec3{1,1,1}},
			};
		for (const auto& slot_name : { "translate", "rotate", "scale" }) {
			if (auto jv = jv_find(data_, slot_name)) {
				auto value = jv_decode_ref_or_vec3(*jv);
				if (auto ptr = std::get_if<std::string>(&value)) {
					inputs_.emplace_back(slot_name, *ptr); }
				else if (auto ptr = std::get_if<rmlv::vec3>(&value)) {
					slotValues[slot_name] = *ptr; } } }
		if (auto jv = jv_find(data_, "many", JSON_NUMBER)) {
			many = static_cast<int>(jv->toNumber()); }
		if (auto jv = jv_find(data_, "gl", JSON_STRING)) {
			inputs_.emplace_back("gl", jv->toString()); }
		out_ = std::make_shared<RepeatOp>(id_, std::move(inputs_),
		                                  many,
		                                  slotValues["translate"],
		                                  slotValues["rotate"],
		                                  slotValues["scale"]); }};

TranslateCompiler translateCompiler{};
RepeatCompiler repeatCompiler{};

struct init { init() {
	NodeRegistry::GetInstance().Register(NodeInfo{ "$repeat", "Repeat", [](NodeBase* node) { return dynamic_cast<RepeatOp*>(node) != nullptr; }, &repeatCompiler });
	NodeRegistry::GetInstance().Register(NodeInfo{ "$translate", "Translate", [](NodeBase* node) { return dynamic_cast<TranslateOp*>(node) != nullptr; }, &translateCompiler });
}} init{};


}  // namespace
}  // namespace rqdq
