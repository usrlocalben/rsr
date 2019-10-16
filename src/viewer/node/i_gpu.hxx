#pragma once
#include "src/rcl/rclmt/rclmt_jobsys.hxx"
#include "src/rgl/rglr/rglr_canvas.hxx"
#include "src/rgl/rglv/rglv_gl.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"
#include "src/viewer/node/base.hxx"

namespace rqdq {
namespace rqv {

class IGPU : public NodeBase {
public:
	using NodeBase::NodeBase;
	virtual auto GetTargetSize() const -> rmlv::ivec2 = 0;
	virtual auto GetAA() const -> bool = 0;
	virtual auto IC() -> rglv::GL& = 0;
	virtual auto Render() -> rclmt::jobsys::Job* = 0; };


}  // namespace rqv
}  // namespace rqdq
