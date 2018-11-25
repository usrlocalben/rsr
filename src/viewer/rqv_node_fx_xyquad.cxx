#include <rqv_node_fx_xyquad.hxx>
#include <rglv_mesh.hxx>
#include <rqv_node_base.hxx>
#include <rqv_shaders.hxx>

#include <memory>
#include <string>

namespace rqdq {
namespace rqv {

using namespace std;

void FxXYQuad::connect(const string& attr, NodeBase* other, const string& slot) {
	if (attr == "material") {
		material_node = dynamic_cast<MaterialNode*>(other); }
	else if (attr == "leftTop") {
		leftTop_node = dynamic_cast<ValuesBase*>(other);
		leftTop_slot = slot; }
	else if (attr == "rightBottom") {
		rightBottom_node = dynamic_cast<ValuesBase*>(other);
		rightBottom_slot = slot; }
	else if (attr == "z") {
		z_node = dynamic_cast<ValuesBase*>(other);
		z_slot = slot; }
	else {
		cout << "FxXYQuad(" << name << ") attempted to add " << other->name << ":" << slot << " as " << attr << endl; } }


}  // close package namespace
}  // close enterprise namespace
