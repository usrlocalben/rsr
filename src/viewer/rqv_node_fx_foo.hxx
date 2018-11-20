#pragma once
#include <rclmt_jobsys.hxx>
#include <rcls_aligned_containers.hxx>
#include <rcls_timer.hxx>
#include <rglr_texture.hxx>
#include <rglv_gl.hxx>
#include <rglv_mesh.hxx>
#include <rglv_vao.hxx>
#include <rmlm_mat4.hxx>
#include <rqv_node_gl.hxx>
#include <rqv_node_value.hxx>
#include <rqv_node_material.hxx>
#include <rqv_shaders.hxx>

#include <memory>
#include <string>
#include <tuple>
#include <PixelToaster.h>

namespace rqdq {
namespace rqv {

struct FxFoo : GlNode {
	// config
	rglv::VertexArray_PNM meshVAO;
	rcls::vector<uint16_t> meshIndices;

	// inputs
	MaterialNode* material_node = nullptr;

	FxFoo(const std::string& name, const InputList& inputs, const rglv::Mesh& mesh) :GlNode(name, inputs) {
		std::tie(meshVAO, meshIndices) = rglv::make_indexed_vao_PNM(mesh); }

	void connect(const std::string&, NodeBase*, const std::string&) override;
	std::vector<NodeBase*> deps() override;

	void draw(rglv::GL* _dc, const rmlm::mat4* const pmat, const rmlm::mat4* const mvmat, rclmt::jobsys::Job* link, int depth) override {
		using namespace rglv;
		auto& dc = *_dc;
		std::lock_guard<std::mutex> lock(dc.mutex);
		if (material_node != nullptr) {
			material_node->apply(_dc); }
		dc.glMatrixMode(GL_PROJECTION);
		dc.glLoadMatrix(*pmat);
		dc.glMatrixMode(GL_MODELVIEW);
		dc.glLoadMatrix(*mvmat);
		dc.glUseArray(meshVAO);
		dc.glDrawElements(GL_TRIANGLES, meshIndices.size(), GL_UNSIGNED_SHORT, meshIndices.data());
		if (link != nullptr) {
			rclmt::jobsys::run(link); }}};

}  // close package namespace
}  // close enterprise namespace
