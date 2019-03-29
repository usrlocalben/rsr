#include "fx_foo.hxx"

#include <memory>
#include <string>

#include "src/rgl/rglv/rglv_mesh.hxx"
#include "src/viewer/node/base.hxx"
#include "src/viewer/node/material.hxx"
#include "src/viewer/shaders.hxx"

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


}  // namespace rqv
}  // namespace rqdq
