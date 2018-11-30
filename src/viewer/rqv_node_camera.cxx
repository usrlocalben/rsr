#include "src/viewer/rqv_node_camera.hxx"

namespace rqdq {
namespace rqv {

using namespace std;

void ManCamNode::connect(const string& attr, NodeBase* other, const std::string& slot) {
	if (attr == "position") {
		position_node = dynamic_cast<ValuesBase*>(other);
		position_slot = slot; }
	else {
		cout << "ManCamNode(" << name << ") attempted to add " << other->name << ":" << slot << " as " << attr << endl; }}

}  // close package namespace
}  // close enterprise namespace