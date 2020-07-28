#pragma once
#include "src/rcl/rclmt/rclmt_jobsys.hxx"
#include "src/rgl/rglv/rglv_gl.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"
#include "src/viewer/node/base.hxx"

namespace rqdq {
namespace rqv {

class ILayer : public NodeBase {
public:
	using NodeBase::NodeBase;
	virtual auto GetBackgroundColor() -> rmlv::vec3 = 0;
	virtual auto Render(rglv::GL* dc, rmlv::ivec2 targetSizeInPx, float aspect) -> void = 0; };


}  // namespace rqv
}  // namespace rqdq
