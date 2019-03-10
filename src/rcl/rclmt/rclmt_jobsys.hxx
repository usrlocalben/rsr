#pragma once
#include <atomic>
#include <cassert>
#include <vector>

namespace rqdq {
namespace rclmt {

namespace jobsys {

#define member_size(type, member) sizeof(((type*)0)->member)

constexpr int DESIRED_JOB_SIZE_IN_BYTES = 128u;
constexpr int NUMBER_OF_JOBS = 65536;
constexpr int MASK = NUMBER_OF_JOBS - 1u;
constexpr int MAX_LINK = 4u;

typedef void(*JobFunction)(struct Job*, const int thread_id, const void*);


struct Job {
	JobFunction function;
	Job* parent;
	std::atomic<int32_t> unfinished_jobs;
	int32_t generation;
	// currently 68 bytes
	char data[DESIRED_JOB_SIZE_IN_BYTES - (sizeof(JobFunction) +
	                                       sizeof(Job*) +
	                                       sizeof(std::atomic<int>) +
	                                       sizeof(int) +
	                                       sizeof(int) +
	                                       (sizeof(Job*) * MAX_LINK))];
	std::atomic<int32_t> link_count;
	Job* link[MAX_LINK];
	};

static_assert(sizeof(Job) == DESIRED_JOB_SIZE_IN_BYTES, "unexpected sizeof(Job)");

struct JobStat {
	double start_time;
	double end_time;
	uint32_t raw;
	};


class Queue {
public:
	Queue() :top(0), bottom(0) {}

	void push(Job* job) {
		auto b = bottom.load(std::memory_order_relaxed);
		jobs[b & MASK] = job;
		bottom.store(b + 1, std::memory_order_release); }

	Job* pop() {
		auto b = bottom.load(std::memory_order_relaxed) - 1;
		bottom.exchange(b);
		auto t = top.load(std::memory_order_relaxed);
		if (t <= b) {
			// non-empty queue
			Job* job = jobs[b & MASK];
			if (t != b) {
				// still more than one item left in the queue
				return job; }

			// this is the last item in the queue
			long tmp = t;
			if (!top.compare_exchange_weak(tmp, t + 1)) {
				// failed race against steal operation
				job = nullptr; }

			bottom.store(t + 1, std::memory_order_relaxed);
			return job; }
		else {
			// deque was already empty
			bottom.store(t, std::memory_order_relaxed);
			return nullptr; } }

	Job* steal() {
		auto t = top.load(std::memory_order_relaxed);
		std::atomic_thread_fence(std::memory_order_acquire);
		auto b = bottom.load(std::memory_order_relaxed);
		if (t < b) {
			Job* job = jobs[t & MASK];
			if (!top.compare_exchange_weak(t, t + 1)) {
				return nullptr; }
			return job; }
		else {
			// queue is empty
			return nullptr; } }

private:
	Job* jobs[NUMBER_OF_JOBS];
	std::atomic<long> top;
	std::atomic<long> bottom;
	};


extern thread_local int thread_id;
extern int thread_count;
extern std::vector<std::vector<struct JobStat>> telemetry_stores;

void run(Job* job);
void wait(const Job* job);
void init(const int threads);
void reset();
void stop();
void join();

void work_start();
void work_end();

Job* allocate_job();

void mark_start();
void mark_end(const uint32_t bits);

template <typename T>
Job* make_job(T function) {
	Job* job = allocate_job();
	job->function = reinterpret_cast<JobFunction>(function);
	job->parent = nullptr;
	job->unfinished_jobs.store(1);
	return job; }


template <typename T, typename D>
Job* make_job(T function, D data) {
	Job* job = allocate_job();
	job->function = reinterpret_cast<JobFunction>(function);
	job->parent = nullptr;
	job->unfinished_jobs.store(1);
	assert(sizeof(data) <= member_size(Job, data));
	memcpy(job->data, &data, sizeof(data));
	return job; }


template <typename T>
Job* make_job_as_child(Job* parent, T function) {
	parent->unfinished_jobs++;

	Job* job = allocate_job();
	job->function = reinterpret_cast<JobFunction>(function);
	job->parent = parent;
	job->unfinished_jobs.store(1);
	return job; }


template <typename T, typename D>
Job* make_job_as_child(Job* parent, T function, D data) {
	parent->unfinished_jobs++;

	Job* job = allocate_job();
	job->function = reinterpret_cast<JobFunction>(function);
	job->parent = parent;
	job->unfinished_jobs.store(1);
	assert(sizeof(data) <= member_size(Job, data));
	memcpy(job->data, &data, sizeof(data));
	return job; }


inline void add_link(Job* dest, Job* linked) {
	const auto idx = dest->link_count++;
	dest->link[idx] = linked; }

inline void move_links(Job* src, Job* dst) {
	for (int i=0; i<src->link_count; i++) {
		add_link(dst, src->link[i]); }
	src->link_count = 0; }

void noop(jobsys::Job* job, const int tid, void*);

void _sleep(int ms);

#undef member_size
}


}  // namespace rclmt
}  // namespace rqdq
