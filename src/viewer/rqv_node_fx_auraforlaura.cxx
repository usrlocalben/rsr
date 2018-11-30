#include "src/viewer/rqv_node_fx_auraforlaura.hxx"
#include "src/viewer/rqv_node_base.hxx"
#include "src/viewer/rqv_node_material.hxx"
#include "src/viewer/rqv_node_value.hxx"
#include "src/rgl/rglv/rglv_mesh.hxx"
#include "src/viewer/rqv_shaders.hxx"

#include <memory>
#include <string>

namespace rqdq {
namespace rqv {

using namespace std;

void FxAuraForLaura::connect(const string& attr, NodeBase* other, const string& slot) {
	if (attr == "material") {
		material_node = dynamic_cast<MaterialNode*>(other); }
	else if (attr == "freq") {
		freq_node = dynamic_cast<ValuesBase*>(other);
		freq_slot = slot; }
	else if (attr == "phase") {
		phase_node = dynamic_cast<ValuesBase*>(other);
		phase_slot = slot; }
	else {
		cout << "FxAuraForLaura(" << name << ") attempted to add " << other->name << ":" << slot << " as " << attr << endl; } }


std::vector<NodeBase*> FxAuraForLaura::deps() {
	std::vector<NodeBase*> out;
	out.push_back(material_node);
	if (freq_node) out.push_back(freq_node);
	if (phase_node) out.push_back(phase_node);
	return out; }


void FxAuraForLaura::main() {
	rclmt::jobsys::run(compute());}

void FxAuraForLaura::computeImpl() {
	using rmlv::ivec2;

	const int numPoints = d_src.points.size();

	rmlv::vec3 freq{ 0.0f };
	rmlv::vec3 phase{ 0.0f };
	if (freq_node) freq = freq_node->get(freq_slot).as_vec3();
	if (phase_node) phase = phase_node->get(phase_slot).as_vec3();

	//const float t = float(ttt.time());
	/*
	const float angle = t;
	const float freqx = sin(angle) * 0.75 + 0.75;
	const float freqy = freqx * 0.61283476f; // sin(t*1.3f)*4.0f;
	const float freqz = 0;  //sin(t*1.1f)*4.0f;
	*/

	const float amp = 1.0f; //sin(t*1.4)*30.0f;

	for (int i=0; i<numPoints; i++) {
		rmlv::vec3 position = d_src.points[i];
		rmlv::vec3 normal = d_src.vertex_normals[i];

		float f = (sin(position.x*freq.x + phase.x) + 0.5f);// * sin(vvn.y*freqy+t) * sin(vvn.z*freqz+t)
		position += normal * amp * f; // normal*amp*f
		float ff = (sin(position.y*freq.y + phase.y) + 0.5f);
		position += normal * amp * ff;

		d_dst.points[i] = position; }
	d_dst.compute_face_and_vertex_normals();

	if (d_meshIndices.size() == 0) {
		for (const auto& face:d_src.faces) {
			for (auto idx : face.point_idx) {
				d_meshIndices.push_back(idx); }}}

	if (++d_activeBuffer > 2) d_activeBuffer = 0;
	auto& vao = d_buffers[d_activeBuffer];
	vao.clear();
	for (int i=0; i<numPoints; i++) {
		vao.append(d_dst.points[i], d_dst.vertex_normals[i], 0); }

	auto postSetup = rclmt::jobsys::make_job(rclmt::jobsys::noop);
	add_links_to(postSetup);
	material_node->add_link(postSetup);
	material_node->run();}


void FxAuraForLaura::draw(rglv::GL* _dc, const rmlm::mat4* const pmat, const rmlm::mat4* const mvmat, rclmt::jobsys::Job* link, int depth) {
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
	dc.glDrawElements(GL_TRIANGLES, d_meshIndices.size(), GL_UNSIGNED_SHORT, d_meshIndices.data());
	if (link != nullptr) {
		rclmt::jobsys::run(link); } }


}  // close package namespace
}  // close enterprise namespace
