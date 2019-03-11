#pragma once
#include <iostream>
#include <memory>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "src/rcl/rclmt/rclmt_jobsys.hxx"

namespace rqdq {
namespace rqv {

using InputList = std::vector<std::pair<std::string, std::string>>;

class NodeBase {
public:
	std::string name;
	InputList inputs;

	std::atomic<int> grpcnt;
	bool visited;
	int indegree_wait;
	std::vector<rclmt::jobsys::Job*> links;

	NodeBase(std::string  name, InputList  inputs)
		:name(std::move(name)), inputs(std::move(inputs)), indegree_wait(0) {}
	virtual ~NodeBase() = default;

	void add_links_to(rclmt::jobsys::Job* parent) {
		for (auto link : links) {
			rclmt::jobsys::add_link(parent, link); } }

	void run() {
		if (--indegree_wait > 0) {
			return; }
		if (!this->validate_settings()) {
			std::cout << "node(" << name << "): validation failed" << std::endl;
			for (auto link : links) {
				rclmt::jobsys::run(link); } }
		this->main(); }

	static void decrjob([[maybe_unused]] rclmt::jobsys::Job* job, [[maybe_unused]] const unsigned tid, std::tuple<std::atomic<int>*, rclmt::jobsys::Job*> * data) {
		auto [cnt, waiting_job] = *data;
		auto& counter = *cnt;
		int nowcnt = --counter;
		if (nowcnt > 0) {
			return; }
		rclmt::jobsys::run(waiting_job); }

	virtual void add_link(rclmt::jobsys::Job *after) {
		links.push_back(after); }

	rclmt::jobsys::Job* after_all(rclmt::jobsys::Job *job) {
		grpcnt++;
		return rclmt::jobsys::make_job(decrjob, std::tuple{&grpcnt, job}); }

	virtual void reset() {
		visited = false;
		indegree_wait = 0;
		links.clear();
		grpcnt = 0; }

	virtual void connect(const std::string& attr, NodeBase* node, const std::string& slot) {
		std::cout << "NodeBase(" << name << ") attempted to add " << node->name << ":" << slot << " as " << attr << "\n"; }

	virtual bool validate_settings() { return true; }

	virtual void main() {
		for (auto link : links) {
			rclmt::jobsys::run(link); }}

	virtual std::vector<NodeBase*> deps() {
		return std::vector<NodeBase*>(); }};


using NodeList = std::vector<std::shared_ptr<NodeBase>>;


void compute_indegrees_from(NodeBase *node);


}  // namespace rqv
}  // namespace rqdq
