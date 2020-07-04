#include "src/rcl/rclmt/rclmt_barrier.hxx"

#include <mutex>

namespace rqdq {
namespace rclmt {

Barrier::Barrier(int numThreads) :
	concurrencyInThreads_(numThreads),
	activityInThreads_(numThreads) {
	assert(numThreads > 0); }


auto Barrier::Join() -> bool {
	std::unique_lock lock(mutex_);
	const auto thisGeneration = generation_;
	if (--activityInThreads_ == 0) {
		// last thread to join, reset
		++generation_;
		activityInThreads_ = concurrencyInThreads_;
		condition_.notify_all();
		return true; }
	else {
		// spin until others join
		while (generation_ == thisGeneration) {
			condition_.wait(lock); }
		return false; }}


}  // namespace rclmt
}  // namespace rqdq
