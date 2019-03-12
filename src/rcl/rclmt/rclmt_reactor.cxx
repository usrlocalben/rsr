#include "src/rcl/rclmt/rclmt_reactor.hxx"

#include <algorithm>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <utility>
#include <vector>

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
		if (d_events.empty()) {
			throw std::runtime_error("reactor event list is empty."); }
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
			const auto handle = pendingEvents[eventIdx];

			d_events[eventIdx].func();
			// func() might cause d_events to be modified, invalidating eventIdx, and re

			auto search = std::find_if(d_events.begin(), d_events.end(),
			                           [=](auto& item) { return item.handle == handle; });
			if (search == d_events.end()) {
				throw std::runtime_error("event being processed was not found after executing the callback"); }
			if (!search->persist) {
				d_events.erase(search); }}
		else {
			auto err = GetLastError();
			std::string msg;
			bool first = true;
			for (auto item : pendingEvents) {
				if (first) {
					msg += fmt::sprintf("%p", item);
					first = false; }
				else {
					msg += fmt::sprintf(", %p", item); }}
			msg = fmt::sprintf("WaitForMultipleObjects error %d, items: %s", err, msg);
			throw std::runtime_error(msg); }}}


void Reactor::Stop() {
	d_shouldQuit = true; }


void Reactor::ListenMany(const rclmt::Event& event, std::function<void()> cb) {
	d_events.emplace_back(ReactorEvent{ event.Get(), std::move(cb), true }); }


void Reactor::ListenOnce(const rclmt::Event& event, std::function<void()> cb) {
	d_events.emplace_back(ReactorEvent{ event.Get(), std::move(cb), false }); }


bool Reactor::RemoveEventByHandle(HANDLE handle) {
	auto found = std::find_if(d_events.begin(), d_events.end(),
	                          [=](auto& item) { return item.handle == handle; });
	if (found == d_events.end()) {
		return false; }  // not found is a noop
	d_events.erase(found);
	return true; }


}  // namespace rclmt
}  // namespace rqdq
