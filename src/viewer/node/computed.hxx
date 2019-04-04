#pragma once
#include <memory>
#include <string>
#include <string_view>

#include "src/rcl/rclmt/rclmt_jobsys.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"
#include "src/viewer/node/value.hxx"

namespace rqdq {
namespace rqv {

struct ComputedNodeState;

/**
 * vec3 computed using exprTk, with inputs from other nodes
 */
class ComputedVec3Node : public ValuesBase {
public:
	using VarDefList = std::vector<std::pair<std::string, std::string>>;

	ComputedVec3Node(std::string_view id, InputList inputs, std::string code, VarDefList varDefs);
	~ComputedVec3Node();

	// NodeBase
	void Connect(std::string_view attr, NodeBase* other, std::string_view slot) override;

	// ValuesBase
	NamedValue Get(std::string_view name) override;

private:
	std::vector<std::unique_ptr<ComputedNodeState>> state_; };


}  // namespace rqv
}  // namespace rqdq
