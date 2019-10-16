#pragma once
#include <string_view>
#include <utility>

#include "src/viewer/node/base.hxx"

namespace rqdq {
namespace rqv {

class ICanvas : public NodeBase {
public:
	using NodeBase::NodeBase;

	static constexpr int CT_FLOAT4_LINEAR = 1;
	static constexpr int CT_FLOAT_QUADS  = 2;
	static constexpr int CT_FLOAT4_QUADS  = 3;
	virtual auto GetCanvas(std::string_view slot) -> std::pair<int, const void*> = 0; };


}  // namespace rqv
}  // namespace rqdq
