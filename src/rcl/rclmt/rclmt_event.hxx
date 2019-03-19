#pragma once
#include <algorithm>
#include <cassert>

namespace rqdq {
namespace rclmt {

class Event {
public:
	// lifecycle
	Event(void* event=nullptr) :d_handle(event) {}
	Event(const Event& other) = delete;
	Event& operator=(const Event& other) = delete;
	Event(Event&& other) noexcept {
		other.Swap(*this); }
	Event& operator=(Event&& other) noexcept {
		other.Swap(*this);
		return *this; }
	void Swap(Event& other) noexcept {
		std::swap(d_handle, other.d_handle); }
	~Event() {
		Close(); }
	void Close();
	void* Release() {
		const auto tmp = d_handle;
		d_handle = nullptr;
		return tmp; }
	void* Get() const {
		assert(d_handle != nullptr);
		return d_handle; }

	// actions
	void Signal();
	void SignalIn(double millis);
	void SignalEvery(double millis);

	// factories
	static Event MakeEvent(bool initialState=false);
	static Event MakeTimer();

private:
	void* d_handle = nullptr; };


}  // namespace rclmt
}  // namespace rqdq
