#pragma once
#include <string_view>

#include "src/rml/rmlm/rmlm_mat4.hxx"
#include "src/viewer/node/base.hxx"

namespace rqdq {
namespace rqv {

class ICamera : public NodeBase {
public:
	using NodeBase::NodeBase;
	virtual auto ProjectionMatrix(float aspect) const -> rmlm::mat4 = 0;
	virtual auto ViewMatrix() const -> rmlm::mat4 = 0; };


}  // namespace rqv
}  // namespace rqdq
