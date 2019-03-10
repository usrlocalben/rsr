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
namespace {

PixelToaster::Timer ttt;

}  // namespace

namespace rqv {

struct FxAuraForLaura final : GlNode {
	// state
	std::array<rglv::VertexArray_F3F3F3, 3> d_buffers;
	int d_activeBuffer = 0;
	rcls::vector<uint16_t> d_meshIndices;
	rglv::Mesh d_src;
	rglv::Mesh d_dst;

	// inputs
	MaterialNode* material_node = nullptr;
	ValuesBase* freq_node = nullptr;  std::string freq_slot;
	ValuesBase* phase_node = nullptr;  std::string phase_slot;

	FxAuraForLaura(const std::string& name, const InputList& inputs, const rglv::Mesh& mesh) :GlNode(name, inputs), d_src(mesh), d_dst(mesh) {}

	void connect(const std::string&, NodeBase*, const std::string&) override;
	std::vector<NodeBase*> deps() override;
	void main() override;

	void draw(rglv::GL*, const rmlm::mat4*, const rmlm::mat4*, rclmt::jobsys::Job*, int) override;

	rclmt::jobsys::Job* compute(rclmt::jobsys::Job* parent = nullptr) {
		if (parent) {
			return rclmt::jobsys::make_job_as_child(parent, FxAuraForLaura::computeJmp, std::tuple{this}); }
		else {
			return rclmt::jobsys::make_job(FxAuraForLaura::computeJmp, std::tuple{this}); }}
	static void computeJmp(rclmt::jobsys::Job* job, unsigned threadId, std::tuple<FxAuraForLaura*>* data) {
		auto&[self] = *data;
		self->computeImpl(); }
	void computeImpl();};


}  // namespace rqv
}  // namespace rqdq
