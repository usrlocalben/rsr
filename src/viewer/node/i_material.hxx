#pragma once
#include "src/rgl/rglv/rglv_gl.hxx"
#include "src/viewer/node/base.hxx"

namespace rqdq {
namespace rqv {

class IMaterial : public NodeBase {
public:
	using NodeBase::NodeBase;
	virtual auto Apply(rglv::GL* /*_dc*/) -> void = 0; };


}  // namespace rqv
}  // namespace rqdq
