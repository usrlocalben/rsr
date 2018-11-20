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
	Barrier(unsigned count) :threshold(count), count(count), generation(0) {
		assert(count > 0); }

	bool wait() {
		std::unique_lock<std::mutex> lock(my_mutex);
		unsigned gen = generation;
		if (--count == 0) {
			generation++;
			count = threshold;
			cond.notify_all();
			return true; }

		while (gen == generation) {
			cond.wait(lock); }

		return false; }

private:
	std::mutex my_mutex;
	std::condition_variable cond;
	unsigned threshold;
	unsigned count;
	unsigned generation;
	};

}  // close package namespace
}  // close enterprise namespace
