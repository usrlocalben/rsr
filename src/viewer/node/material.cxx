#include "material.hxx"

#include <memory>
#include <string_view>

#include "src/rcl/rclmt/rclmt_jobsys.hxx"
#include "src/rgl/rglr/rglr_texture.hxx"
#include "src/rgl/rglv/rglv_gl.hxx"
#include "src/viewer/node/base.hxx"
#include "src/viewer/shaders.hxx"

#include "3rdparty/pixeltoaster/PixelToaster.h"

namespace rqdq {
namespace rqv {

void MaterialNode::Connect(std::string_view attr, NodeBase* other, std::string_view slot) {
	if (attr == "texture0") {
		textureNode0_ = static_cast<TextureNode*>(other); }
	else if (attr == "texture1") {
		textureNode1_ = static_cast<TextureNode*>(other); }
	else if (attr == "u0") {
		uNode0_ = static_cast<ValuesBase*>(other);
		uSlot0_ = slot; }
	else if (attr == "u1") {
		uNode1_ = static_cast<ValuesBase*>(other);
		uSlot1_ = slot; }
	else {
		NodeBase::Connect(attr, other, slot); }}


void MaterialNode::AddDeps() {
	NodeBase::AddDeps();
	AddDep(textureNode0_);
	AddDep(textureNode1_);
	AddDep(uNode0_);
	AddDep(uNode1_); }


void MaterialNode::Apply(rglv::GL* _dc) {
	auto& dc = *_dc;
	dc.glUseProgram(int(programId_));
	if (textureNode0_ != nullptr) {
		auto& texture = textureNode0_->GetTexture();
		dc.glBindTexture(0, texture.buf.data(), texture.width, texture.height, texture.stride, filter_ ? 1 : 0); }
	if (textureNode1_ != nullptr) {
		auto& texture = textureNode1_->GetTexture();
		dc.glBindTexture(1, texture.buf.data(), texture.width, texture.height, texture.stride, filter_ ? 1 : 0); }
	dc.glEnable(rglv::GL_CULL_FACE);
	if (uNode0_) {
		dc.glColor(uNode0_->Get(uSlot0_).as_vec3()); }
	if (uNode1_) {
		dc.glNormal(uNode1_->Get(uSlot1_).as_vec3()); }
	// dc.vertex_input_uniform(VertexInputUniform{ sin(float(gt.elapsed()*3.0f)) * 0.5f + 0.5f });
		}


}  // namespace rqv
}  // namespace rqdq
