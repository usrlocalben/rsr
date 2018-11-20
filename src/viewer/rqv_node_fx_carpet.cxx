#include <rqv_node_fx_carpet.hxx>

#include <rqv_node_base.hxx>
#include <rqv_node_material.hxx>
#include <rqv_node_value.hxx>
#include <rglv_mesh.hxx>
#include <rqv_shaders.hxx>

#include <memory>
#include <string>

namespace rqdq {
namespace rqv {

using namespace std;

void FxCarpet::connect(const string& attr, NodeBase* other, const string& slot) {
	if (attr == "material") {
		material_node = dynamic_cast<MaterialNode*>(other); }
	else if (attr == "freq") {
		freq_node = dynamic_cast<ValuesBase*>(other);
		freq_slot = slot; }
	else if (attr == "phase") {
		phase_node = dynamic_cast<ValuesBase*>(other);
		phase_slot = slot; }
	else {
		GlNode::connect(attr, other, slot);}}


std::vector<NodeBase*> FxCarpet::deps() {
	std::vector<NodeBase*> out;
	out.push_back(material_node);
	if (freq_node) out.push_back(freq_node);
	if (phase_node) out.push_back(phase_node);
	return out; }


void FxCarpet::main() {
	rclmt::jobsys::run(compute());}

void FxCarpet::computeImpl() {
	using rmlv::ivec2, rmlv::vec3, rmlv::vec2;
	if (++d_activeBuffer > 2) d_activeBuffer = 0;
	auto& vao = d_buffers[d_activeBuffer];
	vao.clear();

	rmlv::vec3 freq{ 0.0f };
	rmlv::vec3 phase{ 0.0f };
	if (freq_node) freq = freq_node->get(freq_slot).as_vec3();
	if (phase_node) phase = phase_node->get(phase_slot).as_vec3();

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
	add_links_to(postSetup);
	material_node->add_link(postSetup);
	material_node->run();}


void FxCarpet::draw(rglv::GL* _dc, const rmlm::mat4* const pmat, const rmlm::mat4* const mvmat, rclmt::jobsys::Job* link, int depth) {
	using namespace rglv;
	auto& dc = *_dc;
	std::lock_guard<std::mutex> lock(dc.mutex);
	if (material_node != nullptr) {
		material_node->apply(_dc); }
	dc.glMatrixMode(GL_PROJECTION);
	dc.glLoadMatrix(*pmat);
	dc.glMatrixMode(GL_MODELVIEW);
	dc.glLoadMatrix(*mvmat);
	dc.glUseArray(d_buffers[d_activeBuffer]);
	dc.glDrawArrays(GL_TRIANGLES, 0, 6);
	if (link != nullptr) {
		rclmt::jobsys::run(link); } }


}  // close package namespace
}  // close enterprise namespace
