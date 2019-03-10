#include "src/rcl/rclmt/rclmt_event.hxx"

#include <stdexcept>

#include <fmt/printf.h>
#include <Windows.h>

namespace rqdq {
namespace rclmt {

void Event::Close() {
    if (d_handle != nullptr) {
        const auto tmp = d_handle;
        d_handle = nullptr;
        if (tmp != GetStdHandle(STD_INPUT_HANDLE)) {
            CloseHandle(tmp); }}}

void Event::SignalIn(double millis) {
	const auto t = static_cast<int64_t>(-millis * 10000);
	const auto result = SetWaitableTimer(d_handle, reinterpret_cast<const LARGE_INTEGER*>(&t), 0, nullptr, nullptr, 0);
	if (result == 0) {
		const auto error = GetLastError();
		const auto msg = fmt::sprintf("SetWaitableTimer error %d", error);
		throw std::runtime_error(msg); }}


Event Event::MakeEvent(bool initialState /*=false*/) {
	const auto event = CreateEventW(nullptr, FALSE, static_cast<BOOL>(initialState), nullptr);
	if (event == nullptr) {
		const auto error = GetLastError();
		const auto msg = fmt::sprintf("CreateEventW error %d", error);
		throw std::runtime_error(msg); }
	auto instance = Event(event);
	return instance; }


Event Event::MakeTimer() {
	const auto event = CreateWaitableTimerW(nullptr, FALSE, nullptr);
	if (event == nullptr) {
		const auto error = GetLastError();
		const auto msg = fmt::sprintf("CreateWaitableTimerW error %d", error);
		throw std::runtime_error(msg); }
	auto instance = Event(event);
	return instance; }


}  // namespace rclmt
}  // namespace rqdq
