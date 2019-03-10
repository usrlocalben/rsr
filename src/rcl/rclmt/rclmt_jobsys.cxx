#include "src/rcl/rclmt/rclmt_jobsys.hxx"

#include <atomic>
#include <cassert>
#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <random>
#include <thread>

#include "src/rcl/rclmt/rclmt_barrier.hxx"
#include "src/rcl/rcls/rcls_timer.hxx"

namespace rqdq {
namespace rclmt {

namespace jobsys {

#define member_size(type, member) sizeof(((type *)0)->member)

void help();
void finish(Job* job);
void execute(Job* job);
void thread_main(int id);


inline void sleep([[maybe_unused]] int ms) {}
	//std::this_thread::sleep_for(std::chrono::milliseconds(ms)); }


void _sleep(int ms) {
	std::this_thread::sleep_for(std::chrono::milliseconds(ms)); }


inline void yield() {
	sleep(0); }


using _random_source = std::minstd_rand;
//using _random_source = std::mt19937;


int generate_random_number(const int min, const int max) {
	static thread_local _random_source * generator = nullptr;
	if (generator == nullptr) {
		auto my_hash = std::hash<std::thread::id>()(std::this_thread::get_id());
		generator = new _random_source(int(clock() + my_hash)); }
	return std::uniform_int_distribution(min, max)(*generator); }


std::vector<Job*> jobpools;
std::vector<std::unique_ptr<Queue>> jobqueues;

std::vector<std::vector<struct JobStat>> telemetry_stores;
std::vector<rcls::Timer> telemetry_timers;

thread_local int thread_id;
int generation;
int total_jobs;
thread_local uint32_t pool_idx;
std::atomic<bool> should_work{ false };
std::atomic<bool> should_quit{ false };
int thread_count;

std::vector<std::thread> pool;

std::unique_ptr<Barrier> startBarrier;
std::unique_ptr<Barrier> endBarrier;

thread_local double mark_start_time;


void init(const int numThreads) {
	assert(sizeof(jobsys::Job) == DESIRED_JOB_SIZE_IN_BYTES);

	//	std::cout << "extra bytes: " << member_size(Job, data) << std::endl;

	thread_id = 0; // init should be run in main thread...
	generation = 1;
	total_jobs = 0;
	thread_count = numThreads;

	jobpools.clear();
	jobqueues.clear();
	for (int ti=0; ti<numThreads; ti++) {
		auto telemetry = std::vector<struct JobStat>(NUMBER_OF_JOBS);
		auto timer = rcls::Timer();
		telemetry_stores.push_back(telemetry);
		telemetry_timers.push_back(timer);

		void * ptr = _aligned_malloc(NUMBER_OF_JOBS * sizeof(Job), 64);
		auto job_pool_ptr = static_cast<Job*>(ptr);
		for (int pi = 0; pi < NUMBER_OF_JOBS; pi++) {
			job_pool_ptr[pi].generation = 0; }
		jobpools.push_back(job_pool_ptr);
		jobqueues.push_back(std::make_unique<Queue>()); }

	startBarrier = std::make_unique<Barrier>(numThreads);
	endBarrier = std::make_unique<Barrier>(numThreads);

	for (int ti=1; ti<numThreads; ti++) {
		pool.emplace_back(thread_main, ti); }}


void reset() {
	// XXX in debug, search pool for unfinished jobs
	for (int ti = 0; ti < thread_count; ti++) {
		telemetry_stores[ti].clear();
		telemetry_timers[ti].Reset(); }
	generation++;
	total_jobs = 0; }


Job* allocate_job() {
	auto my_store = jobpools[thread_id];
	assert(total_jobs < NUMBER_OF_JOBS);
	total_jobs += 1;
	const uint32_t index = pool_idx++;
	auto job = &my_store[index & (NUMBER_OF_JOBS - 1)];
	assert(job->generation != generation);
	job->generation = generation;
	job->link_count = 0;
	return job; }


inline bool is_empty_job(const Job* const job) {
	return job == nullptr; }


inline bool has_job_completed(const Job* const job) {
	return job->unfinished_jobs == 0; }


inline bool can_execute(const Job* const job) {
	return job->unfinished_jobs == 1; }




inline Queue* get_my_queue() {
	return jobqueues[thread_id].get(); }


inline Queue* get_random_queue() {
	int random_idx = generate_random_number(0, thread_count - 1);
	return jobqueues[random_idx].get(); }


Job* get_job() {
	auto my_queue = get_my_queue();

	auto job = my_queue->pop();
	if (is_empty_job(job)) {
		// our queue is empty, try to steal
		auto steal_queue = get_random_queue();
		if (steal_queue == my_queue) {
			yield();  // can't steal from ourselves
			return nullptr; }

		auto stolen_job = steal_queue->steal();
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
	while (!can_execute(job)) {
		help(); }

	double start_time = telemetry_timers[thread_id].GetElapsed();
	(job->function)(job, thread_id, job->data);
	double end_time = telemetry_timers[thread_id].GetElapsed();
	telemetry_stores[thread_id].push_back({
		start_time, end_time,
		uint32_t(std::hash<void*>{}(job->function))
	});

	finish(job); }


void run(Job* job) {
	get_my_queue()->push(job);}


void help() {
	Job* job = get_job();
	if (job != nullptr) {
		execute(job);}}


void finish(Job* job) {
	const int32_t unfinished_jobs = --job->unfinished_jobs;
	if (unfinished_jobs == 0) {
		// this thread completed the last child
		if (job->parent != nullptr) {
			finish(job->parent); }
		for (auto link_idx = 0; link_idx < job->link_count; link_idx++) {
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




void thread_main(int id) {
	thread_id = id;
	while (true) {
		startBarrier->Join();
		if (should_quit) {
			break; }
		while (should_work) {
			help(); }
		endBarrier->Join(); }}


void join() {
	for (auto &thread : pool) {
		thread.join();}}


void noop([[maybe_unused]] jobsys::Job *job, [[maybe_unused]] const int tid, void * /*unused*/) {}


void mark_start() {
	mark_start_time = telemetry_timers[thread_id].GetElapsed(); }

void mark_end(const uint32_t bits) {
	double mark_end_time = telemetry_timers[thread_id].GetElapsed();
	telemetry_stores[thread_id].push_back({
		mark_start_time, mark_end_time, bits }); }

}  // namespace jobsys


}  // namespace rclmt
}  // namespace rqdq
