#include "src/rcl/rclmt/rclmt_reactor.hxx"

#include <algorithm>
#include <functional>
#include <utility>
#include <vector>
#include <stdexcept>

#include "src/rcl/rclmt/rclmt_event.hxx"

#include <fmt/printf.h>
#include <Windows.h>

namespace rqdq {
namespace rclmt {

Reactor& Reactor::GetInstance() {
	static Reactor reactor;
	return reactor; }


void Reactor::Run() {
	std::vector<HANDLE> pendingEvents;
	while (!d_shouldQuit) {
		pendingEvents.clear();
		for (auto& re : d_events) {
			pendingEvents.emplace_back(re.handle); }
		DWORD result = WaitForMultipleObjects(static_cast<DWORD>(pendingEvents.size()),
		                                      pendingEvents.data(), FALSE, 1000);
		if (result == WAIT_TIMEOUT) {
			// nothing
			}
		else if (WAIT_OBJECT_0 <= result && result < (WAIT_OBJECT_0+pendingEvents.size())) {
			const int eventIdx = result - WAIT_OBJECT_0;
			auto& re = d_events[eventIdx];
			if (re.func) {
				re.func(); }
			if (!re.persist) {
				d_events.erase(d_events.begin() + eventIdx); }}
		else {
			auto err = GetLastError();
			throw std::runtime_error(fmt::sprintf("WaitForMultipleObjects error %d", err)); }}}


void Reactor::Stop() {
	d_shouldQuit = true; }


void Reactor::ListenMany(const rclmt::Event& event, std::function<void()> cb) {
	d_events.emplace_back(ReactorEvent{ event.Get(), std::move(cb) });
	d_events.back().persist = true; }


void Reactor::ListenOnce(const rclmt::Event& event, std::function<void()> cb) {
	d_events.emplace_back(ReactorEvent{ event.Get(), std::move(cb) });
	d_events.back().persist = false; }


bool Reactor::RemoveEventByHandle(HANDLE handle) {
	auto found = std::find_if(d_events.begin(), d_events.end(),
	                          [=](auto& item) { return item.handle == handle; });
	if (found == d_events.end()) {
		return false; }  // not found is a noop
	d_events.erase(found);
	return true; }


}  // namespace rclmt
}  // namespace rqdq
