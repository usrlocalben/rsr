#pragma once
#include <atomic>

namespace rqdq {
namespace rclmt {

template<typename T, int CAPACITY>
class FixedQueue {
public:
	using value_type = T;
	static constexpr int capacity = CAPACITY;
	static constexpr int ringMask = capacity - 1;

private:
	// DATA
	T* buf_[CAPACITY];
	std::atomic<long> top_{0};
	std::atomic<long> bottom_{0};

public:
	// CREATORS
	FixedQueue() = default;

	// MANIPULATORS
	auto push(T* item) -> void;
	auto pop() -> T*;
	auto steal() -> T*;
};

template<typename T, int CAPACITY>
inline
auto FixedQueue<T, CAPACITY>::steal() -> T* {
	auto t = top_.load(std::memory_order_relaxed);
	std::atomic_thread_fence(std::memory_order_acquire);
	auto b = bottom_.load(std::memory_order_relaxed);
	if (t < b) {
		T* item = buf_[t & ringMask];
		if (!top_.compare_exchange_weak(t, t + 1)) {
			return nullptr; }
		return item; }

	// queue is empty
	return nullptr; }

template<typename T, int CAPACITY>
inline
auto FixedQueue<T, CAPACITY>::push(T* item) -> void {
	auto b = bottom_.load(std::memory_order_relaxed);
	buf_[b & ringMask] = item;
	bottom_.store(b + 1, std::memory_order_release); }

template<typename T, int CAPACITY>
inline
auto FixedQueue<T, CAPACITY>::pop() -> T* {
	auto b = bottom_.load(std::memory_order_relaxed) - 1;
	bottom_.exchange(b);
	auto t = top_.load(std::memory_order_relaxed);
	if (t <= b) {
		// non-empty queue
		T* item = buf_[b & ringMask];
		if (t != b) {
			// still more than one item left in the queue
			return item; }

		// this is the last item in the queue
		long tmp = t;
		if (!top_.compare_exchange_weak(tmp, t + 1)) {
			// failed race against steal operation
			item = nullptr; }

		bottom_.store(t + 1, std::memory_order_relaxed);
		return item; }

	// deque was already empty
	bottom_.store(t, std::memory_order_relaxed);
	return nullptr; }


}  // namespace rclmt
}  // namespace rqdq
