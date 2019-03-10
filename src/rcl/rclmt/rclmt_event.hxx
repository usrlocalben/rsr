#pragma once
#include <algorithm>
#include <cassert>

#include <Windows.h>

namespace rqdq {
namespace rclmt {

class Event {
public:
	// lifecycle
	Event(HANDLE event=nullptr) :d_handle(event) {}
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
	HANDLE Release() {
		const auto tmp = d_handle;
		d_handle = nullptr;
		return tmp; }
	HANDLE Get() const {
		assert(d_handle != nullptr);
		return d_handle; }

	// actions
	void Signal() { SetEvent(d_handle); }
	void SignalIn(double millis);

	// factories
	static Event MakeEvent(bool initialState=false);
	static Event MakeTimer();

private:
	HANDLE d_handle = nullptr; };


}  // namespace rclmt
}  // namespace rqdq
