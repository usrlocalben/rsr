#include <memory>
#include <mutex>
#include <string_view>

#include "src/rgl/rglv/rglv_mesh.hxx"
#include "src/viewer/compile.hxx"
#include "src/viewer/node/gl.hxx"
#include "src/viewer/node/base.hxx"
#include "src/viewer/node/material.hxx"
#include "src/viewer/node/value.hxx"
#include "src/viewer/shaders.hxx"

namespace rqdq {
namespace {

using namespace rqv;

class FxCarpet final : public GlNode {
public:
	FxCarpet(std::string_view id, InputList inputs)
		:GlNode(id, std::move(inputs)) {}

	void Connect(std::string_view attr, NodeBase* other, std::string_view slot) override;
	void AddDeps() override;
	void Main() override;

	void Draw(rglv::GL* /*_dc*/, const rmlm::mat4* /*pmat*/, const rmlm::mat4* /*mvmat*/, rclmt::jobsys::Job* /*link*/, int /*depth*/) override;

public:
	rclmt::jobsys::Job* Compute(rclmt::jobsys::Job* parent = nullptr) {
		if (parent != nullptr) {
			return rclmt::jobsys::make_job_as_child(parent, FxCarpet::ComputeJmp, std::tuple{this}); }
		
			return rclmt::jobsys::make_job(FxCarpet::ComputeJmp, std::tuple{this}); }
private:
	static void ComputeJmp(rclmt::jobsys::Job* job, unsigned threadId, std::tuple<FxCarpet*>* data) {
		auto&[self] = *data;
		self->ComputeImpl(); }
	void ComputeImpl();

private:
	// state
	std::array<rglv::VertexArray_F3F3F3, 3> buffers_;
	int activeBuffer_{0};

	// inputs
	MaterialNode* materialNode_{nullptr};
	ValuesBase* freqNode_{nullptr};
	std::string freqSlot_{};
	ValuesBase* phaseNode_{nullptr};
	std::string phaseSlot_{}; };


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

	rmlv::vec3 freq{ 0.0F };
	rmlv::vec3 phase{ 0.0F };
	if (freqNode_ != nullptr) {
		freq = freqNode_->Get(freqSlot_).as_vec3(); }
	if (phaseNode_ != nullptr) {
		phase = phaseNode_->Get(phaseSlot_).as_vec3();}

	vec3 leftTopP{ -1.0F, 0.5F, 0 };
	vec3 leftTopT{ 0.0F, 1.0F, 0 };

	vec3 rightBottomP{ 1.0F, -0.5F, 0 };
	vec3 rightBottomT{ 1.0F, 0.0F, 0 };

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


class Compiler final : public NodeCompiler {
	void Build() override {
		if (!Input("MaterialNode", "material", /*required=*/true)) { return; }
		if (!Input("ValuesBase", "freq", /*required=*/true)) { return; }
		if (!Input("ValuesBase", "phase", /*required=*/true)) { return; }
		out_ = std::make_shared<FxCarpet>(id_, std::move(inputs_)); }};

Compiler compiler{};

struct init { init() {
	NodeRegistry::GetInstance().Register(NodeInfo{
		"$fxCarpet",
		"FxCarpet",
		[](NodeBase* node) { return dynamic_cast<FxCarpet*>(node) != nullptr; },
		&compiler });
}} init{};


}  // namespace
}  // namespace rqdq
