#pragma once
#include "src/rcl/rclmt/rclmt_jobsys.hxx"
#include "src/rgl/rglv/rglv_gl.hxx"
#include "src/viewer/node/base.hxx"

namespace rqdq {
namespace rqv {

class ILayer : public NodeBase {
public:
	using NodeBase::NodeBase;
	virtual auto GetBackgroundColor() -> rmlv::vec3 = 0;
	virtual auto Render(rglv::GL* dc, int width, int height, float aspect, rclmt::jobsys::Job *link) -> void = 0; };


}  // namespace rqv
}  // namespace rqdq
