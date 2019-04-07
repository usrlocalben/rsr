#pragma once
#include <algorithm>
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
public:
	NodeBase(std::string_view id, InputList inputs);
	virtual ~NodeBase();

	void AddLink(rclmt::jobsys::Job *after);
	void Run();
	std::string_view get_id() {
		return id_; }

	static void DecrJob(rclmt::jobsys::Job* job, unsigned tid, std::tuple<std::atomic<int>*, rclmt::jobsys::Job*>* data);
	rclmt::jobsys::Job* AfterAll(rclmt::jobsys::Job *job);


	virtual void Reset();
	virtual void Connect(std::string_view attr, NodeBase* other, std::string_view slot);
	virtual bool IsValid();
	virtual void Main();
	virtual const std::vector<NodeBase*>& Deps();
	void inc_indegreeWaitCnt() {
		indegreeWaitCnt_++; }
	void set_indegreeWaitCnt(int value) {
		indegreeWaitCnt_ = value; }

	const auto& get_inputs() {
		return inputs_; }
protected:
	void AddLinksTo(rclmt::jobsys::Job* parent);
	void RunLinks();
	virtual void AddDep(NodeBase* node);
	virtual void AddDeps();


private:
	std::vector<NodeBase*> deps_;
	std::string id_;
	InputList inputs_;
	std::atomic<int> groupCnt_{0};
	int indegreeWaitCnt_{0};
	std::vector<rclmt::jobsys::Job*> links_; };


using NodeList = std::vector<std::shared_ptr<NodeBase>>;


void ComputeIndegreesFrom(NodeBase *node);


}  // namespace rqv
}  // namespace rqdq
