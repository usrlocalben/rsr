#include "src/rcl/rclmt/rclmt_barrier.hxx"

#include <mutex>

namespace rqdq {
namespace rclmt {


Barrier::Barrier(int numThreads)
	:d_numThreads(numThreads), d_numThreadsStillActive(numThreads) {
	assert(numThreads > 0); }


bool Barrier::Join() {
	std::unique_lock<std::mutex> lock(d_mutex);
	auto gen = d_generation;
	if (--d_numThreadsStillActive == 0) {
		d_generation++;
		d_numThreadsStillActive = d_numThreads;
		d_condition.notify_all();
		return true; }

	while (gen == d_generation) {
		d_condition.wait(lock); }

	return false; }


}  // close package namespace
}  // close enterprise namespace
