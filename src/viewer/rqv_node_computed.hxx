#include <rclmt_jobsys.hxx>
#include <rmlv_vec.hxx>
#include <rqv_node_value.hxx>

#include <memory>

namespace rqdq {
namespace rqv {

struct ComputedNodeState;

/**
 * vec3 computed using exprTk, with inputs from other nodes
 */
struct ComputedVec3Node : public ValuesBase {
	std::vector<std::unique_ptr<ComputedNodeState>> cn_thread_state;

	ComputedVec3Node(
		const std::string&,
		const InputList&,
		const std::string&,
		const std::vector<std::pair<std::string, std::string>>&);
	~ComputedVec3Node();

	void connect(const std::string&, NodeBase*, const std::string&) override;
	NamedValue get(const std::string& name) override; };

}  // close package namespace
}  // close enterprise namespace
