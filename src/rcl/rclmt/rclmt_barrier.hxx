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
public:
	Barrier(int numThreads);
	bool Join();

private:
	std::mutex d_mutex;
	std::condition_variable d_condition;
	const int d_numThreads;
	int d_numThreadsStillActive;
	uint32_t d_generation = 0; };


}  // close package namespace
}  // close enterprise namespace
