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
namespace rqv {

class FxFoo final : public GlNode {
public:
	FxFoo(std::string_view id, InputList inputs, const rglv::Mesh& mesh)
		:GlNode(id, std::move(inputs)) {
		std::tie(meshVAO_, meshIndices_) = rglv::make_indexed_vao_F3F3F3(mesh); }

	void Connect(std::string_view, NodeBase* other, std::string_view slot) override;

	void AddDeps() override;

	void Draw(rglv::GL* _dc, const rmlm::mat4* const pmat, const rmlm::mat4* const mvmat, rclmt::jobsys::Job* link, int depth) override;

private:
	// config
	rglv::VertexArray_F3F3F3 meshVAO_;
	rcls::vector<uint16_t> meshIndices_;

	// inputs
	MaterialNode* materialNode_{nullptr}; };

}  // namespace rqv
}  // namespace rqdq
