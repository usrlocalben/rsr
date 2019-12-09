#include "src/rcl/rclmt/rclmt_jobsys.hxx"

#include <atomic>
#include <cassert>
#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <random>
#include <thread>
#include <optional>

#include "src/rcl/rclmt/rclmt_barrier.hxx"
#include "src/rcl/rclmt/rclmt_fixedqueue.hxx"
#include "src/rcl/rcls/rcls_timer.hxx"

#define member_size(type, member) sizeof(((type *)0)->member)

namespace rqdq {
namespace {

using _random_source = std::minstd_rand;
//using _random_source = std::mt19937;

auto generate_random_number(int min, int max) -> int {
	thread_local _random_source* generator{nullptr};
	if (generator == nullptr) {
		auto my_hash = std::hash<std::thread::id>()(std::this_thread::get_id());
		generator = new _random_source(int(clock() + my_hash)); }
	return std::uniform_int_distribution(min, max)(*generator); }

}  // namespace
namespace rclmt {

namespace jobsys {

using JobQueue = FixedQueue<Job, kBacklogSizeInJobs>;

thread_local int threadId;
int numThreads;
std::vector<std::thread> thread_pt;

std::vector<Job*> pool_pt;
thread_local uint32_t poolIdx;
JobQueue* queue_pt;

std::vector<std::vector<struct JobStat>> measurements_pt;
std::vector<rcls::Timer> timer_pt;
thread_local double mark_start_time;
bool telemetryEnabled{ false };

std::atomic<int32_t> totalJobs;
std::atomic<bool> should_work{ false };
std::atomic<bool> should_quit{ false };

std::optional<Barrier> startBarrier;
std::optional<Barrier> endBarrier;

void help();
void finish(Job* job);
void execute(Job* job);
void thread_main(int id);

inline
void sleep([[maybe_unused]] int ms) {}
	//std::this_thread::sleep_for(std::chrono::milliseconds(ms)); }

void _sleep(int ms) {
	std::this_thread::sleep_for(std::chrono::milliseconds(ms)); }

inline
void yield() {
	sleep(0); }

void init(const int many) {
    static_assert(sizeof(jobsys::Job) == kDesiredJobSizeInBytes);

	//	std::cout << "extra bytes: " << member_size(Job, data) << std::endl;

	threadId = 0;  // init must be run in main thread
	totalJobs = 0;
	numThreads = many;

    queue_pt = static_cast<JobQueue*>(_aligned_malloc(numThreads * sizeof(JobQueue), 64));

    measurements_pt.resize(numThreads);
    timer_pt.resize(numThreads);

	pool_pt.clear();
	for (int ti=0; ti<numThreads; ++ti) {
		auto pool_ptr = static_cast<Job*>(_aligned_malloc(kBacklogSizeInJobs * sizeof(Job), 64));
		pool_pt.push_back(pool_ptr);
        new (queue_pt+ti) JobQueue{}; }

	startBarrier.emplace(numThreads);
	endBarrier.emplace(numThreads);

	for (int ti=1; ti<numThreads; ++ti) {
		thread_pt.emplace_back(thread_main, ti); }}

void reset() {
	for (int ti{0}; ti < numThreads; ++ti) {
		measurements_pt[ti].clear();
		timer_pt[ti].Reset(); }
	totalJobs = 0; }

auto allocate_job() -> Job* {
	auto& pool = pool_pt[threadId];
	// assert(totalJobs < kBacklogSizeInJobs);
	++totalJobs;
	const uint32_t index = poolIdx++;
	auto job = &pool[index & (kBacklogSizeInJobs - 1)];
	// assert(job->generation != generation);
	// job->generation = generation;
	return job; }

inline
bool is_empty_job(const Job* const job) {
	return job == nullptr; }

auto has_job_completed(const Job* const job) -> bool {
	return job->numPending == 0; }

inline
auto can_execute(const Job* const job) -> bool {
	return job->numPending == 1; }

inline
auto get_my_queue() -> JobQueue& {
	return queue_pt[threadId]; }

inline
auto get_random_queue() -> JobQueue& {
	int random_idx = generate_random_number(0, numThreads-1);
	return queue_pt[random_idx]; }

static
auto get_job() -> Job* {
	auto& my_queue = get_my_queue();

	auto job = my_queue.pop();
	if (is_empty_job(job)) {
		// our queue is empty, try to steal
		auto& steal_queue = get_random_queue();
		if (&steal_queue == &my_queue) {
			yield();  // can't steal from ourselves
			return nullptr; }

		auto stolen_job = steal_queue.steal();
		if (is_empty_job(stolen_job)) {
			yield();  // steal failed
			return nullptr; }

		return stolen_job; }

	return job; }

void wait(const Job* job) {
	while (!has_job_completed(job)) {
		Job* next_job = get_job();
		if (next_job != nullptr) {
			execute(next_job);}}}

void execute(Job* job) {
    auto& timer = timer_pt[threadId];
    auto& measurements = measurements_pt[threadId];

	while (!can_execute(job)) {
		help(); }

	double start_time{telemetryEnabled ? timer.GetElapsed() : 0};
	(job->function)(job, threadId, job->data);
	if (telemetryEnabled) {
		double end_time = timer.GetElapsed();
		measurements.push_back({
			start_time, end_time,
			uint32_t(std::hash<void*>{}(reinterpret_cast<void*>(job->function)))
		});}

	finish(job); }

void run(Job* job) {
	get_my_queue().push(job);}

void help() {
	Job* job = get_job();
	if (job != nullptr) {
		execute(job);}}

void finish(Job* job) {
	const int32_t numPending = --job->numPending;
	if (numPending == 0) {
		// this thread completed the last child
		if (job->parent != nullptr) {
			finish(job->parent); }
		for (auto link_idx = 0; link_idx < job->numLinks; ++link_idx) {
			run(job->link[link_idx]); }}}

void work_start() {
	should_work.store(true);
	startBarrier->Join();}

void work_end() {
	should_work.store(false);
	endBarrier->Join();}

void stop() {
	should_work.store(false);
	should_quit.store(true);
	startBarrier->Join();}

void thread_main(int tid) {
	threadId = tid;
	while (true) {
		startBarrier->Join();
		if (should_quit) {
			break; }
		while (should_work) {
			help(); }
		endBarrier->Join(); }}

void join() {
	for (auto& thread : thread_pt) {
		thread.join();}}

void noop(Job*, const int, void*) {}

void fnjmp(Job*, const int, void* data) {
	auto fn = static_cast<std::function<void()>*>(data);
	(*fn)();
	fn->~function();
}

void mark_start() {
    auto& timer = timer_pt[threadId];
	if (telemetryEnabled) {
		mark_start_time = timer.GetElapsed(); }}

void mark_end(const uint32_t bits) {
    auto& timer = timer_pt[threadId];
    auto& measurements = measurements_pt[threadId];
	if (telemetryEnabled) {
		double mark_end_time = timer.GetElapsed();
		measurements.push_back({ mark_start_time, mark_end_time, bits }); }}


}  // namespace jobsys


}  // namespace rclmt
}  // namespace rqdq
