#pragma once
#include <string_view>

#include "src/rgl/rglv/rglv_gl.hxx"
#include "src/rml/rmlm/rmlm_mat4.hxx"
#include "src/viewer/node/base.hxx"

namespace rqdq {
namespace rqv {

class IGl : public NodeBase {
public:
	using NodeBase::NodeBase;
	virtual auto Draw(rglv::GL* dc, const rmlm::mat4* pmat, const rmlm::mat4* mvmat, rclmt::jobsys::Job *link, int depth) -> void = 0; };


}  // namespace rqv
}  // namespace rqdq
