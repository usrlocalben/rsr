#include "camera.hxx"

#include <string_view>

namespace rqdq {
namespace rqv {

void ManCamNode::Connect(std::string_view attr, NodeBase* other, std::string_view slot) {
	if (attr == "position") {
		positionNode_ = static_cast<ValuesBase*>(other);
		positionSlot_ = slot; }
	else {
		CameraNode::Connect(attr, other, slot); }}


}  // namespace rqv
}  // namespace rqdq
