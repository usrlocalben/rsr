#include "fx_xyquad.hxx"

#include <memory>
#include <string_view>

#include "src/rgl/rglv/rglv_mesh.hxx"
#include "src/viewer/node/base.hxx"
#include "src/viewer/shaders.hxx"

namespace rqdq {
namespace rqv {

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
	vec3 tul{ 0.0f, 1.0f, 0 };               vec3 tur{ 1.0f, 1.0f, 0 };

	vec3 pll{ leftTop.x, rightBottom.y, z }; vec3 plr{ rightBottom.x, rightBottom.y, z };
	vec3 tll{ 0.0f, 0.0f, 0 };               vec3 tlr{ 1.0f, 0.0f, 0 };

	vec3 normal{ 0, 0, 1.0f };

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


}  // namespace rqv
}  // namespace rqdq
