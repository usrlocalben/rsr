#include <memory>
#include <string_view>

#include "src/rgl/rglv/rglv_mesh.hxx"
#include "src/viewer/node/base.hxx"
#include "src/viewer/node/gl.hxx"
#include "src/viewer/node/material.hxx"
#include "src/viewer/compile.hxx"
#include "src/viewer/shaders.hxx"

namespace rqdq {
namespace {

using namespace rqv;

class FxXYQuad final : public GlNode {
public:
	FxXYQuad(std::string_view id, InputList inputs)
		:GlNode(id, std::move(inputs)) {}

	void Connect(std::string_view attr, NodeBase* other, std::string_view slot) override;

	void AddDeps() override {
		AddDep(materialNode_);
		AddDep(leftTopNode_);
		AddDep(rightBottomNode_);
		AddDep(zNode_); }

	void Reset() override {
		activeBuffer_ = (activeBuffer_+1)%3;
		GlNode::Reset(); }

	void Main() override;

	void Draw(rglv::GL* _dc, const rmlm::mat4* const pmat, const rmlm::mat4* const mvmat, rclmt::jobsys::Job* link, int depth) override {
		auto& dc = *_dc;
		using namespace rglv;
		std::scoped_lock<std::mutex> lock(dc.mutex);
		if (materialNode_ != nullptr) {
			materialNode_->Apply(_dc); }
		dc.glMatrixMode(GL_PROJECTION);
		dc.glLoadMatrix(rglv::make_glOrtho(-1.0F, 1.0F, -1.0F, 1.0F, 1.0, -1.0));
		dc.glMatrixMode(GL_MODELVIEW);
		dc.glLoadMatrix(rmlm::mat4::ident());
		dc.glUseArray(buffers_[activeBuffer_]);
		dc.glDrawArrays(GL_TRIANGLES, 0, 6);
		if (link != nullptr) {
			rclmt::jobsys::run(link); } }

private:
	int activeBuffer_{0};
	std::array<rglv::VertexArray_F3F3F3, 3> buffers_;
	rcls::vector<uint16_t> meshIdx_;

	// connections
	MaterialNode* materialNode_{nullptr};
	ValuesBase* leftTopNode_{nullptr};
	std::string leftTopSlot_{};
	ValuesBase* rightBottomNode_{nullptr};
	std::string rightBottomSlot_{};
	ValuesBase* zNode_{nullptr};
	std::string zSlot_{};

	// config
	rmlv::vec2 leftTop_;
	rmlv::vec2 rightBottom_;
	float z_; };


void FxXYQuad::Connect(std::string_view attr, NodeBase* other, std::string_view slot) {
	if (attr == "material") {
		materialNode_ = static_cast<MaterialNode*>(other); }
	else if (attr == "leftTop") {
		leftTopNode_ = static_cast<ValuesBase*>(other);
		leftTopSlot_ = slot; }
	else if (attr == "rightBottom") {
		rightBottomNode_ = static_cast<ValuesBase*>(other);
		rightBottomSlot_ = slot; }
	else if (attr == "z") {
		zNode_ = static_cast<ValuesBase*>(other);
		zSlot_ = slot; }
	else {
		GlNode::Connect(attr, other, slot); }}


void FxXYQuad::Main() {
	using namespace rclmt::jobsys;
	using rmlv::ivec2, rmlv::vec3;
	auto& vao = buffers_[activeBuffer_];
	vao.clear();

	rmlv::vec2 leftTop = leftTopNode_ != nullptr ? leftTopNode_->Get(leftTopSlot_).as_vec2() : leftTop_;
	rmlv::vec2 rightBottom = rightBottomNode_ != nullptr ? rightBottomNode_->Get(rightBottomSlot_).as_vec2() : rightBottom_;
	float z = zNode_ != nullptr ? zNode_->Get(zSlot_).as_float() : z_;

	vec3 pul{ leftTop.x, leftTop.y, z };     vec3 pur{ rightBottom.x, leftTop.y, z };
	vec3 tul{ 0.0F, 1.0F, 0 };               vec3 tur{ 1.0F, 1.0F, 0 };

	vec3 pll{ leftTop.x, rightBottom.y, z }; vec3 plr{ rightBottom.x, rightBottom.y, z };
	vec3 tll{ 0.0F, 0.0F, 0 };               vec3 tlr{ 1.0F, 0.0F, 0 };

	vec3 normal{ 0, 0, 1.0F };

	// quad upper left
	vao.append(pur, normal, tur);
	vao.append(pul, normal, tul);
	vao.append(pll, normal, tll);

	// quad lower right
	vao.append(pur, normal, tur);
	vao.append(pll, normal, tll);
	vao.append(plr, normal, tlr);
	vao.pad();

	Job* postSetup = make_job(noop);
	AddLinksTo(postSetup);
	materialNode_->AddLink(postSetup);
	materialNode_->Run();}


class Compiler final : public NodeCompiler {
	void Build() override {
		if (!Input("MaterialNode", "material", /*required=*/true)) { return; }
		if (!Input("ValuesBase", "leftTop", /*required=*/true)) { return; }
		if (!Input("ValuesBase", "rightBottom", /*required=*/true)) { return; }
		if (!Input("ValuesBase", "z", /*required=*/true)) { return; }

		/*vec2 leftTop{ -1.0F, 1.0F };
		vec2 rightBottom{ 1.0F, -1.0F };
		float z{0.0F};*/

		out_ = std::make_shared<FxXYQuad>(id_, std::move(inputs_)); }};

Compiler compiler{};

struct init { init() {
	NodeRegistry::GetInstance().Register(NodeInfo{
		"$fxXYQuad",
		"FxXYQuad",
		[](NodeBase* node) { return dynamic_cast<FxXYQuad*>(node) != nullptr; },
		&compiler });
}} init{};


}  // namespace
}  // namespace rqdq
