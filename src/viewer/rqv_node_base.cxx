#include "src/viewer/rqv_node_base.hxx"

namespace rqdq {
namespace rqv {

void compute_indegrees_from(NodeBase *node) {
	if (node->visited) {
		return; }
	node->visited = true;
	for (auto& dep : node->deps()) {
		dep->indegree_wait += 1;
		compute_indegrees_from(dep); } }

}  // close package namespace
}  // close enterprise namespace
