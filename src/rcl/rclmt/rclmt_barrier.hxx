/**
 * thread synchronization barrier
 * similar to boost/thread/barrier
 */
#pragma once
#include <mutex>
#include <cassert>

namespace rqdq {
namespace rclmt {

class Barrier {
	std::mutex mutex_;
	std::condition_variable condition_;
	const int concurrencyInThreads_;
	int activityInThreads_;
	uint32_t generation_{0};

public:
	explicit Barrier(int /*numThreads*/);
	auto Join() -> bool; };


}  // namespace rclmt
}  // namespace rqdq
