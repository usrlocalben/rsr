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
#include "src/rgl/rglv/rglv_math.hxx"
#include "src/rgl/rglv/rglv_vao.hxx"
#include "src/rml/rmlm/rmlm_mat4.hxx"
#include "src/viewer/node/gl.hxx"
#include "src/viewer/node/material.hxx"
#include "src/viewer/node/value.hxx"
#include "src/viewer/shaders.hxx"

namespace rqdq {
namespace rqv {

class FxXYQuad final : public GlNode {
public:
	FxXYQuad(std::string_view id, InputList inputs)
		:GlNode(id, std::move(inputs)) {}

	void Connect(std::string_view attr, NodeBase* other, std::string_view slot) override;

	void AddDeps() override {
		AddDep(materialNode_);
		AddDep(leftTopNode_);
		AddDep(rightBottomNode_);
		AddDep(zNode_); }

	void Reset() override {
		activeBuffer_ = (activeBuffer_+1)%3;
		GlNode::Reset(); }

	void Main() override;

	void Draw(rglv::GL* _dc, const rmlm::mat4* const pmat, const rmlm::mat4* const mvmat, rclmt::jobsys::Job* link, int depth) override {
		auto& dc = *_dc;
		using namespace rglv;
		std::scoped_lock<std::mutex> lock(dc.mutex);
		if (materialNode_ != nullptr) {
			materialNode_->Apply(_dc); }
		dc.glMatrixMode(GL_PROJECTION);
		dc.glLoadMatrix(rglv::make_glOrtho(-1.0f, 1.0f, -1.0f, 1.0f, 1.0, -1.0));
		dc.glMatrixMode(GL_MODELVIEW);
		dc.glLoadMatrix(rmlm::mat4::ident());
		dc.glUseArray(buffers_[activeBuffer_]);
		dc.glDrawArrays(GL_TRIANGLES, 0, 6);
		if (link != nullptr) {
			rclmt::jobsys::run(link); } }

private:
	int activeBuffer_{0};
	std::array<rglv::VertexArray_F3F3F3, 3> buffers_;
	rcls::vector<uint16_t> meshIdx_;

	// connections
	MaterialNode* materialNode_{nullptr};
	ValuesBase* leftTopNode_{nullptr};
	std::string leftTopSlot_{};
	ValuesBase* rightBottomNode_{nullptr};
	std::string rightBottomSlot_{};
	ValuesBase* zNode_{nullptr};
	std::string zSlot_{};

	// config
	rmlv::vec2 leftTop_;
	rmlv::vec2 rightBottom_;
	float z_; };


}  // namespace rqv
}  // namespace rqdq
