#pragma once
#include "src/rgl/rglr/rglr_texture.hxx"
#include "src/viewer/node/base.hxx"

namespace rqdq {
namespace rqv {

class ITexture : public NodeBase {
public:
	using NodeBase::NodeBase;
	virtual auto GetTexture() -> const rglr::Texture& = 0; };


}  // namespace rqv
}  // namespace rqdq
