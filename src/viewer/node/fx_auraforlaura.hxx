#pragma once
#include <memory>
#include <string>
#include <string_view>
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

class FxAuraForLaura final : public GlNode {
public:
	FxAuraForLaura(std::string_view id, InputList inputs, const rglv::Mesh& mesh)
		:GlNode(id, std::move(inputs)), src_(mesh), dst_(mesh) {}

	void Connect(std::string_view attr, NodeBase* other, std::string_view slot) override;
	void AddDeps() override;
	void Main() override;

	void Draw(rglv::GL*, const rmlm::mat4*, const rmlm::mat4*, rclmt::jobsys::Job*, int) override;

public:
	rclmt::jobsys::Job* Compute(rclmt::jobsys::Job* parent = nullptr) {
		if (parent) {
			return rclmt::jobsys::make_job_as_child(parent, FxAuraForLaura::ComputeJmp, std::tuple{this}); }
		else {
			return rclmt::jobsys::make_job(FxAuraForLaura::ComputeJmp, std::tuple{this}); }}
private:
	static void ComputeJmp(rclmt::jobsys::Job* job, unsigned threadId, std::tuple<FxAuraForLaura*>* data) {
		auto&[self] = *data;
		self->ComputeImpl(); }
	void ComputeImpl();

private:
	// state
	std::array<rglv::VertexArray_F3F3F3, 3> buffers_{};
	int activeBuffer_{0};
	rcls::vector<uint16_t> meshIndices_;
	rglv::Mesh src_;
	rglv::Mesh dst_;

	// inputs
	MaterialNode* materialNode_{nullptr};
	ValuesBase* freqNode_{nullptr};
	std::string freqSlot_{};
	ValuesBase* phaseNode_{nullptr};
	std::string phaseSlot_{}; };


}  // namespace rqv
}  // namespace rqdq
