#include "fx_auraforlaura.hxx"

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

void FxAuraForLaura::Connect(std::string_view attr, NodeBase* other, std::string_view slot) {
	if (attr == "material") {
		materialNode_ = static_cast<MaterialNode*>(other); }
	else if (attr == "freq") {
		freqNode_ = static_cast<ValuesBase*>(other);
		freqSlot_ = slot; }
	else if (attr == "phase") {
		phaseNode_ = static_cast<ValuesBase*>(other);
		phaseSlot_ = slot; }
	else {
		GlNode::Connect(attr, other, slot); }}


void FxAuraForLaura::AddDeps() {
	AddDep(materialNode_);
	AddDep(freqNode_);
	AddDep(phaseNode_); }


void FxAuraForLaura::Main() {
	rclmt::jobsys::run(Compute());}


void FxAuraForLaura::ComputeImpl() {
	using rmlv::ivec2;

	const int numPoints = src_.points.size();

	rmlv::vec3 freq{ 0.0f };
	if (freqNode_ != nullptr) {
		freq = freqNode_->Get(freqSlot_).as_vec3();

	rmlv::vec3 phase{ 0.0f };
	if (phaseNode_ != nullptr) {
		phase = phaseNode_->Get(phaseSlot_).as_vec3(); }

	//const float t = float(ttt.time());
	/*
	const float angle = t;
	const float freqx = sin(angle) * 0.75 + 0.75;
	const float freqy = freqx * 0.61283476f; // sin(t*1.3f)*4.0f;
	const float freqz = 0;  //sin(t*1.1f)*4.0f;
	*/

	const float amp = 1.0f; //sin(t*1.4)*30.0f;

	for (int i=0; i<numPoints; i++) {
		rmlv::vec3 position = src_.points[i];
		rmlv::vec3 normal = src_.vertex_normals[i];

		float f = (sin(position.x*freq.x + phase.x) + 0.5f);// * sin(vvn.y*freqy+t) * sin(vvn.z*freqz+t)
		position += normal * amp * f; // normal*amp*f
		float ff = (sin(position.y*freq.y + phase.y) + 0.5f);
		position += normal * amp * ff;

		dst_.points[i] = position; }
	dst_.compute_face_and_vertex_normals();

	if (meshIndices_.size() == 0) {
		for (const auto& face : src_.faces) {
			for (auto idx : face.point_idx) {
				meshIndices_.push_back(idx); }}}

	activeBuffer_ = (activeBuffer_+1)%3;
	auto& vao = buffers_[activeBuffer_];
	vao.clear();
	for (int i=0; i<numPoints; i++) {
		vao.append(dst_.points[i], dst_.vertex_normals[i], 0); }

	auto postSetup = rclmt::jobsys::make_job(rclmt::jobsys::noop);
	AddLinksTo(postSetup);
	materialNode_->AddLink(postSetup);
	materialNode_->Run();}}


void FxAuraForLaura::Draw(rglv::GL* _dc, const rmlm::mat4* const pmat, const rmlm::mat4* const mvmat, rclmt::jobsys::Job* link, int depth) {
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
	dc.glDrawElements(GL_TRIANGLES, meshIndices_.size(), GL_UNSIGNED_SHORT, meshIndices_.data());
	if (link != nullptr) {
		rclmt::jobsys::run(link); } }


}  // namespace rqv
}  // namespace rqdq
