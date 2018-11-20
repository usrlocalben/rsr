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
#include <rqv_node_material.hxx>
#include <rqv_node_value.hxx>
#include <rqv_shaders.hxx>

#include <memory>
#include <string>
#include <tuple>
#include <PixelToaster.h>

namespace rqdq {
namespace rqv {

struct FxCarpet final : GlNode {
	// state
	std::array<rglv::VertexArray_PNM, 3> d_buffers;
	int d_activeBuffer = 0;

	// inputs
	MaterialNode* material_node = nullptr;
	ValuesBase* freq_node = nullptr;  std::string freq_slot;
	ValuesBase* phase_node = nullptr;  std::string phase_slot;

	FxCarpet(const std::string& name, const InputList& inputs) :GlNode(name, inputs) {}

	void connect(const std::string&, NodeBase*, const std::string&) override;
	std::vector<NodeBase*> deps() override;
	void main() override;

	void draw(rglv::GL*, const rmlm::mat4*, const rmlm::mat4*, rclmt::jobsys::Job*, int) override;

	rclmt::jobsys::Job* compute(rclmt::jobsys::Job* parent = nullptr) {
		if (parent) {
			return rclmt::jobsys::make_job_as_child(parent, FxCarpet::computeJmp, std::tuple{this}); }
		else {
			return rclmt::jobsys::make_job(FxCarpet::computeJmp, std::tuple{this}); }}
	static void computeJmp(rclmt::jobsys::Job* job, unsigned threadId, std::tuple<FxCarpet*>* data) {
		auto&[self] = *data;
		self->computeImpl(); }
	void computeImpl();};

}  // close package namespace
}  // close enterprise namespace
