#include "src/viewer/node/fx_xyquad.hxx"

#include <memory>
#include <string>

#include "src/rgl/rglv/rglv_mesh.hxx"
#include "src/viewer/node/base.hxx"
#include "src/viewer/shaders.hxx"

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


}  // namespace rqv
}  // namespace rqdq
