#include "base.hxx"

#include <string_view>
#include <unordered_set>
#include <utility>
#include <memory>

#include "src/rcl/rclmt/rclmt_jobsys.hxx"

namespace rqdq {
namespace rqv {

NodeBase::NodeBase(std::string_view id, InputList inputs)
	:id_(id), inputs_(std::move(inputs)) {}


NodeBase::~NodeBase() = default;


void NodeBase::AddLinksTo(rclmt::jobsys::Job* parent) {
	for (int i=0; i<linkCnt_; i++) {
		rclmt::jobsys::add_link(parent, links_[i]); } }

void NodeBase::RunLinks() {
	for (int i=0; i<linkCnt_; i++) {
		rclmt::jobsys::run(links_[i]); }}


void NodeBase::Run() {
	if (--indegreeWaitCnt_ > 0) {
		return; }
	if (!this->IsValid()) {
		std::cout << "node(" << id_ << "): validation failed" << std::endl;
		for (int i=0; i<linkCnt_; i++) {
			rclmt::jobsys::run(links_[i]); } }
	this->Main(); }


void NodeBase::DecrJob(rclmt::jobsys::Job* job [[maybe_unused]],
                       const unsigned tid [[maybe_unused]],
                       std::tuple<std::atomic<int>*,rclmt::jobsys::Job*>* data) {
	auto [cnt, waiting_job] = *data;
	auto& counter = *cnt;
	int nowcnt = --counter;
	if (nowcnt > 0) {
		return; }
	rclmt::jobsys::run(waiting_job); }


auto NodeBase::AfterAll(rclmt::jobsys::Job* job) -> rclmt::jobsys::Job* {
	groupCnt_++;
	return rclmt::jobsys::make_job(DecrJob, std::tuple{&groupCnt_, job}); }


void NodeBase::AddLink(rclmt::jobsys::Job *after) {
	links_[linkCnt_++] = after; }


void NodeBase::Reset() {
	indegreeWaitCnt_ = 0;
	linkCnt_ = 0;
	groupCnt_ = 0; }


auto NodeBase::Connect(std::string_view attr, NodeBase* other, std::string_view slot) -> bool {
	std::cerr << "NodeBase(" << id_ << ") attempted to connect " << attr << " to " << other->id_ << ":" << slot << "\n";
	return false; }


auto NodeBase::IsValid() -> bool {
	return true; }


void NodeBase::Main() {
	for (int i=0; i<linkCnt_; i++) {
		rclmt::jobsys::run(links_[i]); }}


void NodeBase::AddDep(NodeBase* node) {
	if (node != nullptr) {
		deps_.emplace_back(node); }}


void NodeBase::AddDeps(){}


auto NodeBase::Deps() -> const std::vector<NodeBase*>& {
	deps_.clear();
	AddDeps();
	return deps_; }


void ComputeIndegreesFrom(NodeBase *node, int d) {
	thread_local std::unordered_set<NodeBase*> visited;
	if (d == 0) {
		visited.clear(); }
	if (visited.find(node) != end(visited)) {
		return; }
	visited.insert(node);
	for (auto& dep : node->Deps()) {
		dep->inc_indegreeWaitCnt();
		ComputeIndegreesFrom(dep, d+1); } }


}  // namespace rqv
}  // namespace rqdq
