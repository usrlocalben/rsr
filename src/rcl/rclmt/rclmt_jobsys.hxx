#pragma once
#include <atomic>
#include <functional>
#include <vector>

#define member_size(type, member) sizeof(((type*)0)->member)

namespace rqdq {
namespace rclmt {

namespace jobsys {

constexpr auto kDesiredJobSizeInBytes = 128L;

constexpr auto kBacklogSizeInJobs = 8192L;

constexpr auto kMaxLinks = 4L;

using JobFunction = void (*)(struct Job*, int, const void*);

#pragma pack(1)
struct Job {
	JobFunction function;
	Job* parent;
	std::atomic<int32_t> numPending;

	// 75 bytes on windows x64
	char data[kDesiredJobSizeInBytes - (sizeof(JobFunction) +
	                                    sizeof(Job*) +
	                                    sizeof(std::atomic<int32_t>) +
	                                    sizeof(std::atomic<int8_t>) +
	                                    (sizeof(Job*) * kMaxLinks))];

	std::atomic<int8_t> numLinks;
	Job* link[kMaxLinks]; };
#pragma pack()

static_assert(sizeof(Job) == kDesiredJobSizeInBytes, "unexpected sizeof(Job)");

extern thread_local int threadId;

extern int numThreads;

extern bool telemetryEnabled;

void init(int threads);

auto allocate_job() -> Job*;

void run(Job* job);

void wait(const Job* job);

auto has_job_completed(const Job* const) -> bool;

void stop();

void join();

void work_start();

void work_end();

void reset();

template <typename T>
auto make_job(T function) -> Job*;

template <typename T, typename D>
auto make_job(T function, D data) -> Job*;

template <typename T>
auto make_job_as_child(Job* parent, T function) -> Job*;

template <typename T, typename D>
auto make_job_as_child(Job* parent, T function, D data) -> Job*;

auto make_job_as_child_fn(Job* parent, std::function<void()> fn) -> Job*;

void add_link(Job* dest, Job* linked);

void move_links(Job* src, Job* dst);

void noop(jobsys::Job*, int, void*);

void fnjmp(jobsys::Job*, int, void*);

void _sleep(int ms);

void mark_start();

void mark_end(uint32_t bits);

struct JobStat {
	double start_time;
	double end_time;
	uint32_t raw; };

extern std::vector<std::vector<struct JobStat>> measurements_pt;

}  // namespace jobsys

// ============================================================================
//                         INLINE DEFINITIONS
// ============================================================================

						// ----------------
						// namespace jobsys
						// ----------------

// FREE FUNCTIONS
template <typename T>
auto jobsys::make_job(T function) -> Job* {
	Job* job = allocate_job();
	job->function = reinterpret_cast<JobFunction>(function);
	job->parent = nullptr;
	job->numPending.store(1);
	job->numLinks = 0;
	return job; }

template <typename T, typename D>
auto jobsys::make_job(T function, D data) -> Job* {
	Job* job = make_job<T>(function);
	static_assert(sizeof(data) <= member_size(Job, data));
	memcpy(job->data, &data, sizeof(data));
	return job; }

template <typename T>
auto jobsys::make_job_as_child(Job* parent, T function) -> Job* {
	parent->numPending++;
	Job* job = make_job<T>(function);
	job->parent = parent;
	return job; }

template <typename T, typename D>
auto jobsys::make_job_as_child(Job* parent, T function, D data) -> Job* {
	Job* job = make_job_as_child<T>(parent, function);
	static_assert(sizeof(data) <= member_size(Job, data));
	memcpy(job->data, &data, sizeof(data));
	return job; }

inline
auto jobsys::make_job_as_child_fn(Job* parent, std::function<void()> fn) -> Job* {
	Job* job = make_job_as_child(parent, jobsys::fnjmp);
	void* d = static_cast<void*>(&job->data);
	new (d) std::function<void()>{std::move(fn)};
	return job; }

inline
void jobsys::add_link(Job* dest, Job* linked) {
	const auto idx = dest->numLinks++;
	dest->link[idx] = linked; }

inline
void jobsys::move_links(Job* src, Job* dst) {
	for (int i=0; i<src->numLinks; i++) {
		add_link(dst, src->link[i]); }
	src->numLinks = 0; }


}  // namespace rclmt
}  // namespace rqdq

#undef member_size
