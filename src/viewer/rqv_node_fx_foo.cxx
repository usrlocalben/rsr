#include <rqv_node_fx_foo.hxx>
#include <rglv_mesh.hxx>
#include <rqv_node_base.hxx>
#include <rqv_node_material.hxx>
#include <rqv_shaders.hxx>

#include <memory>
#include <string>

namespace rqdq {
namespace rqv {

using namespace std;

void FxFoo::connect(const string& attr, NodeBase* other, const string& slot) {
	if (attr == "material") {
		material_node = dynamic_cast<MaterialNode*>(other); }
	else {
		cout << "FxFoo(" << name << ") attempted to add " << other->name << ":" << slot << " as " << attr << endl; } }


vector<NodeBase*> FxFoo::deps() {
	vector<NodeBase*> out;
	out.push_back(material_node);
	return out; }


}  // close package namespace
}  // close enterprise namespace
