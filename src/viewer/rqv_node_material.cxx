#include <rqv_node_material.hxx>
#include <rclmt_jobsys.hxx>
#include <rglr_texture.hxx>
#include <rglv_gl.hxx>
#include <rqv_node_base.hxx>
#include <rqv_shaders.hxx>

#include <memory>
#include <string>

#include <PixelToaster.h>

namespace rqdq {
namespace rqv {

using namespace std;

void MaterialNode::connect(const string& attr, NodeBase* other, const string& slot) {
	if (attr == "texture") {
		texture_node = dynamic_cast<TextureNode*>(other); }
	else if (attr == "u0") {
		u0_node = dynamic_cast<ValuesBase*>(other);
		u0_slot = slot; }
	else if (attr == "u1") {
		u1_node = dynamic_cast<ValuesBase*>(other);
		u1_slot = slot; }
	else {
		NodeBase::connect(attr, other, slot); }}


vector<NodeBase*> MaterialNode::deps() {
	vector<NodeBase*> out;
	if (texture_node != nullptr) {
		out.push_back(texture_node); }
	if (u0_node != nullptr) {
		out.push_back(u0_node); }
	if (u1_node != nullptr) {
		out.push_back(u1_node); }
	return out; }


void MaterialNode::apply(rglv::GL* _dc) {
	auto& dc = *_dc;
	dc.glUseProgram(int(d_program));
	if (texture_node != nullptr) {
		auto& texture = texture_node->getTexture();
		dc.glBindTexture(texture.buf.data(), texture.width, texture.height, texture.stride, d_filter ? 1 : 0); }
	dc.glEnable(rglv::GL_CULL_FACE);
	if (u0_node) {
		dc.glColor(u0_node->get(u0_slot).as_vec3()); }
	if (u1_node) {
		dc.glNormal(u1_node->get(u1_slot).as_vec3()); }
	// dc.vertex_input_uniform(VertexInputUniform{ sin(float(gt.elapsed()*3.0f)) * 0.5f + 0.5f });
	}


}  // close package namespace
}  // close enterprise namespace
