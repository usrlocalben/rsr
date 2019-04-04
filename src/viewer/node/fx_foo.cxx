#include "fx_foo.hxx"

#include <memory>
#include <string_view>

#include "src/rgl/rglv/rglv_mesh.hxx"
#include "src/viewer/node/base.hxx"
#include "src/viewer/node/material.hxx"
#include "src/viewer/shaders.hxx"

namespace rqdq {
namespace rqv {

void FxFoo::Connect(std::string_view attr, NodeBase* other, std::string_view slot) {
	if (attr == "material") {
		materialNode_ = static_cast<MaterialNode*>(other); }
	else {
		GlNode::Connect(attr, other, slot); }}


void FxFoo::AddDeps() {
	AddDep(materialNode_); }


void FxFoo::Draw(rglv::GL* _dc, const rmlm::mat4* const pmat, const rmlm::mat4* const mvmat, rclmt::jobsys::Job* link, int depth) {
	using namespace rglv;
	auto& dc = *_dc;
	std::lock_guard<std::mutex> lock(dc.mutex);
	if (materialNode_ != nullptr) {
		materialNode_->Apply(_dc); }
	dc.glMatrixMode(GL_PROJECTION);
	dc.glLoadMatrix(*pmat);
	dc.glMatrixMode(GL_MODELVIEW);
	dc.glLoadMatrix(*mvmat);
	dc.glUseArray(meshVAO_);
	dc.glDrawElements(GL_TRIANGLES, meshIndices_.size(), GL_UNSIGNED_SHORT, meshIndices_.data());
	if (link != nullptr) {
		rclmt::jobsys::run(link); }}


}  // namespace rqv
}  // namespace rqdq
