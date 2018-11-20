#pragma once
#include <rclmt_jobsys.hxx>
#include <rcls_aligned_containers.hxx>
#include <rcls_timer.hxx>
#include <rglr_texture.hxx>
#include <rglv_gl.hxx>
#include <rglv_math.hxx>
#include <rglv_vao.hxx>
#include <rmlm_mat4.hxx>
#include <rqv_node_gl.hxx>
#include <rqv_node_material.hxx>
#include <rqv_node_value.hxx>
#include <rqv_shaders.hxx>

#include <memory>
#include <string>
#include <tuple>

namespace rqdq {
namespace rqv {

struct FxXYQuad : GlNode {
	int d_activeBuffer = 0;
	std::array<rglv::VertexArray_PNM, 3> d_buffers;
	rcls::vector<uint16_t> mesh_idx;

	// connections
	MaterialNode* material_node = nullptr;
	ValuesBase* leftTop_node = nullptr; std::string leftTop_slot;
	ValuesBase* rightBottom_node = nullptr; std::string rightBottom_slot;
	ValuesBase* z_node = nullptr; std::string z_slot;

	// config
	rmlv::vec2 d_leftTop;
	rmlv::vec2 d_rightBottom;
	float d_z;

	FxXYQuad(
		const std::string& name,
		const InputList& inputs
	) :GlNode(name, inputs) {}

	void connect(const std::string&, NodeBase*, const std::string&) override;

	std::vector<NodeBase*> deps() override{
		std::vector<NodeBase*> out;
		out.push_back(material_node);
		if (leftTop_node) out.push_back(leftTop_node);
		if (rightBottom_node) out.push_back(rightBottom_node);
		if (z_node) out.push_back(z_node);
		return out; }

	void main() override {
		namespace jobsys = rclmt::jobsys;
		using rmlv::ivec2, rmlv::vec3;
		if (++d_activeBuffer > 2) d_activeBuffer = 0;
		auto& vao = d_buffers[d_activeBuffer];
		vao.clear();

		rmlv::vec2 leftTop = leftTop_node->get(leftTop_slot).as_vec2();
		rmlv::vec2 rightBottom = rightBottom_node->get(rightBottom_slot).as_vec2();
		float z = z_node->get(z_slot).as_float();

		vec3 pul{ leftTop.x, leftTop.y, z };     vec3 pur{ rightBottom.x, leftTop.y, z };
		vec3 tul{ 0.0f, 1.0f, 0 };               vec3 tur{ 1.0f, 1.0f, 0 };

		vec3 pll{ leftTop.x, rightBottom.y, z }; vec3 plr{ rightBottom.x, rightBottom.y, z };
		vec3 tll{ 0.0f, 0.0f, 0 };               vec3 tlr{ 1.0f, 0.0f, 0 };

		std::array<vec3, 3> quad_ul = { pur, pul, pll };
		std::array<vec3, 3> quad_lr = { pur, pll, plr };

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

		jobsys::Job *postSetup = jobsys::make_job(jobsys::noop);
		add_links_to(postSetup);
		material_node->add_link(postSetup);
		material_node->run();}

	void draw(rglv::GL* _dc, const rmlm::mat4* const pmat, const rmlm::mat4* const mvmat, rclmt::jobsys::Job* link, int depth) override {
		auto& dc = *_dc;
		using namespace rglv;
		std::scoped_lock<std::mutex> lock(dc.mutex);
		if (material_node != nullptr) {
			material_node->apply(_dc); }
		dc.glMatrixMode(GL_PROJECTION);
		dc.glLoadMatrix(rglv::make_glOrtho(-1.0f, 1.0f, -1.0f, 1.0f, 1.0, -1.0));
		dc.glMatrixMode(GL_MODELVIEW);
		dc.glLoadMatrix(rmlm::mat4::ident());
		dc.glUseArray(d_buffers[d_activeBuffer]);
		dc.glDrawArrays(GL_TRIANGLES, 0, 6);
		if (link != nullptr) {
			rclmt::jobsys::run(link); } }};

}  // close package namespace
}  // close enterprise namespace
