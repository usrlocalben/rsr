#include "fx_carpet.hxx"

#include <memory>
#include <mutex>
#include <string_view>

#include "src/rgl/rglv/rglv_mesh.hxx"
#include "src/viewer/node/base.hxx"
#include "src/viewer/node/material.hxx"
#include "src/viewer/node/value.hxx"
#include "src/viewer/shaders.hxx"

namespace rqdq {
namespace rqv {

void FxCarpet::Connect(std::string_view attr, NodeBase* other, std::string_view slot) {
	if (attr == "material") {
		materialNode_ = static_cast<MaterialNode*>(other); }
	else if (attr == "freq") {
		freqNode_ = static_cast<ValuesBase*>(other);
		freqSlot_ = slot; }
	else if (attr == "phase") {
		phaseNode_ = static_cast<ValuesBase*>(other);
		phaseSlot_ = slot; }
	else {
		GlNode::Connect(attr, other, slot);}}


void FxCarpet::AddDeps() {
	AddDep(materialNode_);
	AddDep(freqNode_);
	AddDep(phaseNode_); }


void FxCarpet::Main() {
	rclmt::jobsys::run(Compute());}


void FxCarpet::ComputeImpl() {
	using rmlv::ivec2, rmlv::vec3, rmlv::vec2;
	activeBuffer_ = (activeBuffer_+1) % 3;
	auto& vao = buffers_[activeBuffer_];
	vao.clear();

	rmlv::vec3 freq{ 0.0f };
	rmlv::vec3 phase{ 0.0f };
	if (freqNode_ != nullptr) {
		freq = freqNode_->Get(freqSlot_).as_vec3(); }
	if (phaseNode_ != nullptr) {
		phase = phaseNode_->Get(phaseSlot_).as_vec3();}

	vec3 leftTopP{ -1.0f, 0.5f, 0 };
	vec3 leftTopT{ 0.0f, 1.0f, 0 };

	vec3 rightBottomP{ 1.0f, -0.5f, 0 };
	vec3 rightBottomT{ 1.0f, 0.0f, 0 };

	auto emitQuad = [&](vec3 ltp, vec3 ltt, vec3 rbp, vec3 rbt) {
		auto LT = ltp;                    auto RT = vec3{rbp.x, ltp.y, 0};
		auto LB = vec3{ltp.x, rbp.y, 0};  auto RB = rbp;
		{ // lower-left
			auto v0v2 = RB - LT;
			auto v0v1 = LB - LT;
			auto normal = normalize(cross(v0v1, v0v2));
			vao.append(LT, normal, vec3{ltt.x, ltt.y, 0});  // top left
			vao.append(LB, normal, vec3{ltt.x, rbt.y, 0});  // bottom left
			vao.append(RB, normal, vec3{rbt.x, rbt.y, 0});}  // bottom right
		{ // upper-right
			auto v0v2 = RT - LT;
			auto v0v1 = RB - LT;
			auto normal = normalize(cross(v0v1, v0v2));
			vao.append(LT, normal, vec3{ltt.x, ltt.y, 0});  // top left
			vao.append(RB, normal, vec3{rbt.x, rbt.y, 0});  // bottom right
			vao.append(RT, normal, vec3{rbt.x, ltt.y, 0});}};  // top right

	emitQuad(leftTopP, leftTopT, rightBottomP, rightBottomT);

	auto postSetup = rclmt::jobsys::make_job(rclmt::jobsys::noop);
	AddLinksTo(postSetup);
	materialNode_->AddLink(postSetup);
	materialNode_->Run();}


void FxCarpet::Draw(rglv::GL* _dc, const rmlm::mat4* const pmat, const rmlm::mat4* const mvmat, rclmt::jobsys::Job* link, int depth) {
	using namespace rglv;
	auto& dc = *_dc;
	std::lock_guard<std::mutex> lock(dc.mutex);
	if (materialNode_ != nullptr) {
		materialNode_->Apply(_dc); }
	dc.glMatrixMode(GL_PROJECTION);
	dc.glLoadMatrix(*pmat);
	dc.glMatrixMode(GL_MODELVIEW);
	dc.glLoadMatrix(*mvmat);
	dc.glUseArray(buffers_[activeBuffer_]);
	dc.glDrawArrays(GL_TRIANGLES, 0, 6);
	if (link != nullptr) {
		rclmt::jobsys::run(link); } }


}  // namespace rqv
}  // namespace rqdq
