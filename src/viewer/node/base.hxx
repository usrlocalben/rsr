#pragma once
#include <algorithm>
#include <array>
#include <atomic>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

#include "src/rcl/rclmt/rclmt_jobsys.hxx"

namespace rqdq {
namespace rqv {

using InputList = std::vector<std::pair<std::string, std::string>>;

class NodeBase {
	std::vector<NodeBase*> deps_{};
	std::string id_{};
	InputList inputs_{};
	std::atomic<int> groupCnt_{0};
	std::atomic<int> indegreeWaitCnt_{0};
	std::atomic<int> linkCnt_{0};
	std::array<rclmt::jobsys::Job*, 16> links_{};

public:
	NodeBase(std::string_view id, InputList inputs);
	auto operator=(const NodeBase&) -> NodeBase& = delete;
	NodeBase(const NodeBase&) = delete;
	auto operator=(NodeBase&&) -> NodeBase&& = delete;
	NodeBase(NodeBase&&) = delete;
	virtual ~NodeBase();

	void AddLink(rclmt::jobsys::Job *after);
	void Run();
	auto get_id() -> std::string_view;

	static void DecrJob(rclmt::jobsys::Job* job, unsigned tid, std::tuple<std::atomic<int>*, rclmt::jobsys::Job*>* data);
	auto AfterAll(rclmt::jobsys::Job* job) -> rclmt::jobsys::Job*;

	virtual void Reset();
	virtual auto Connect(std::string_view attr, NodeBase* other, std::string_view slot) -> bool;
	virtual void DisconnectAll();
	virtual auto IsValid() -> bool;
	virtual void Main();
	virtual auto Deps() -> const std::vector<NodeBase*>&;
	void inc_indegreeWaitCnt();
	void set_indegreeWaitCnt(int value);

	auto get_inputs() -> const InputList&;

protected:
	void AddLinksTo(rclmt::jobsys::Job* parent);
	void RunLinks();
	virtual void AddDep(NodeBase* node);
	virtual void AddDeps(); };


using NodeList = std::vector<std::shared_ptr<NodeBase>>;

#define TYPE_ERROR(nt) \
	{ std::cerr << "Node id=" << get_id() << ": refusing to connect @gpu to id=" << other->get_id() << " because it is the wrong type (expected " << #nt << ")\n"; }

void ComputeIndegreesFrom(NodeBase *node, int d=0);

// ============================================================================
//								INLINE DEFINITIONS
// ============================================================================

inline
auto NodeBase::get_id() -> std::string_view {
	return id_; }

inline
void NodeBase::inc_indegreeWaitCnt() {
	++indegreeWaitCnt_; }

inline
void NodeBase::set_indegreeWaitCnt(int value) {
	indegreeWaitCnt_ = value; }

inline
auto NodeBase::get_inputs() -> const InputList& {
	return inputs_; }


}  // namespace rqv
}  // namespace rqdq
