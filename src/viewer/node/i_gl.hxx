#pragma once
#include <string_view>

#include "src/rcl/rcls/rcls_aligned_containers.hxx"
#include "src/rgl/rglv/rglv_gl.hxx"
#include "src/rml/rmlm/rmlm_mat4.hxx"
#include "src/viewer/node/base.hxx"

namespace rqdq {
namespace rqv {

struct Light {
	rmlm::mat4 matrix;
};


class IGl : public NodeBase {
public:
	using NodeBase::NodeBase;
	virtual auto Draw(rglv::GL* dc, const rmlm::mat4* pmat, const rmlm::mat4* mvmat, int depth) -> void = 0;
	// virtual auto GetLights(const rmlm::mat4* mvmat) -> rcls::vector<Light> = 0;
	};


}  // namespace rqv
}  // namespace rqdq
