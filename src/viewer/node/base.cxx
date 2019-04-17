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
	for (auto link : links_) {
		rclmt::jobsys::add_link(parent, link); } }

void NodeBase::RunLinks() {
	for (auto link : links_) {
		rclmt::jobsys::run(link); }}


void NodeBase::Run() {
	if (--indegreeWaitCnt_ > 0) {
		return; }
	if (!this->IsValid()) {
		std::cout << "node(" << id_ << "): validation failed" << std::endl;
		for (auto link : links_) {
			rclmt::jobsys::run(link); } }
	this->Main(); }


void NodeBase::DecrJob([[maybe_unused]] rclmt::jobsys::Job* job,
                       [[maybe_unused]] const unsigned tid,
                       std::tuple<std::atomic<int>*,rclmt::jobsys::Job*>* data) {
	auto [cnt, waiting_job] = *data;
	auto& counter = *cnt;
	int nowcnt = --counter;
	if (nowcnt > 0) {
		return; }
	rclmt::jobsys::run(waiting_job); }


rclmt::jobsys::Job* NodeBase::AfterAll(rclmt::jobsys::Job* job) {
	groupCnt_++;
	return rclmt::jobsys::make_job(DecrJob, std::tuple{&groupCnt_, job}); }


void NodeBase::AddLink(rclmt::jobsys::Job *after) {
	links_.push_back(after); }


void NodeBase::Reset() {
	indegreeWaitCnt_ = 0;
	links_.clear();
	groupCnt_ = 0; }


bool NodeBase::Connect(std::string_view attr, NodeBase* other, std::string_view slot) {
	std::cerr << "NodeBase(" << id_ << ") attempted to connect " << attr << " to " << other->id_ << ":" << slot << "\n";
	return false; }


bool NodeBase::IsValid() {
	return true; }


void NodeBase::Main() {
	for (auto& link : links_) {
		rclmt::jobsys::run(link); }}


void NodeBase::AddDep(NodeBase* node) {
	if (node != nullptr) {
		deps_.emplace_back(node); }}


void NodeBase::AddDeps(){}


const std::vector<NodeBase*>& NodeBase::Deps() {
	deps_.clear();
	AddDeps();
	return deps_; }


void ComputeIndegreesFrom(NodeBase *node) {
	thread_local std::unordered_set<NodeBase*> visited;
	visited.clear();
	if (visited.find(node) != end(visited)) {
		return; }
	visited.insert(node);
	for (auto& dep : node->Deps()) {
		dep->inc_indegreeWaitCnt();
		ComputeIndegreesFrom(dep); } }


}  // namespace rqv
}  // namespace rqdq
