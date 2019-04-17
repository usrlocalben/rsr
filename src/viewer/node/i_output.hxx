#pragma once
#include "src/rcl/rclmt/rclmt_jobsys.hxx"
#include "src/rgl/rglr/rglr_canvas.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"
#include "src/viewer/node/base.hxx"

namespace rqdq {
namespace rqv {

class IOutput : public NodeBase {
public:
	using NodeBase::NodeBase;
	virtual auto Render() -> rclmt::jobsys::Job* = 0;
	virtual auto SetOutputCanvas(rglr::TrueColorCanvas* canvas) -> void = 0;
	virtual auto SetDoubleBuffer(bool enable) -> void = 0;
	virtual auto SetTileDim(rmlv::ivec2 dim) -> void = 0; };


}  // namespace rqv
}  // namespace rqdq
