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
	virtual auto SetDimensions(int x, int y) -> void = 0;
	virtual auto SetTileDimensions(rmlv::ivec2 tileDim) -> void = 0;
	virtual auto SetAspect(float aspect) -> void = 0;
	virtual auto SetDoubleBuffer(bool value) -> void = 0;
	virtual auto SetColorCanvas(rglr::QFloat4Canvas* ptr) -> void = 0;
	virtual auto SetDepthCanvas(rglr::QFloatCanvas* ptr) -> void = 0;
	virtual auto GetIC() -> rglv::GL& = 0;
	virtual auto Render() -> rclmt::jobsys::Job* = 0; };


}  // namespace rqv
}  // namespace rqdq
