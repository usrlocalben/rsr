#pragma once
#include <memory>
#include <string>
#include <tuple>

#include "src/rcl/rclmt/rclmt_jobsys.hxx"
#include "src/rcl/rcls/rcls_aligned_containers.hxx"
#include "src/rcl/rcls/rcls_timer.hxx"
#include "src/rgl/rglr/rglr_texture.hxx"
#include "src/rgl/rglv/rglv_gl.hxx"
#include "src/rgl/rglv/rglv_mesh.hxx"
#include "src/rgl/rglv/rglv_vao.hxx"
#include "src/rml/rmlm/rmlm_mat4.hxx"
#include "src/viewer/node/gl.hxx"
#include "src/viewer/node/material.hxx"
#include "src/viewer/node/value.hxx"
#include "src/viewer/shaders.hxx"

#include "3rdparty/pixeltoaster/PixelToaster.h"

namespace rqdq {
namespace rqv {

struct FxFoo : GlNode {
	// config
	rglv::VertexArray_F3F3F3 meshVAO;
	rcls::vector<uint16_t> meshIndices;

	// inputs
	MaterialNode* material_node = nullptr;

	FxFoo(const std::string& name, const InputList& inputs, const rglv::Mesh& mesh) :GlNode(name, inputs) {
		std::tie(meshVAO, meshIndices) = rglv::make_indexed_vao_F3F3F3(mesh); }

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


}  // namespace rqv
}  // namespace rqdq
