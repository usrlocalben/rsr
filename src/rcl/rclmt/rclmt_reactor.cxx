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
namespace {

struct ReactorEvent {
	HANDLE handle{nullptr};
	std::function<void()> func;
	bool persist{false}; };

bool shouldQuit = false;

std::unordered_map<HANDLE, ReactorEvent> eventTab;


}  // namespace

namespace rclmt {

Reactor& Reactor::GetInstance() {
	static Reactor reactor;
	return reactor; }


void Reactor::Run() {
	std::vector<HANDLE> pendingEvents;
	while (!shouldQuit) {
		if (eventTab.empty()) {
			throw std::runtime_error("reactor event table is empty."); }
		pendingEvents.clear();
		for (auto& re : eventTab) {
			pendingEvents.emplace_back(re.first); }
		DWORD result = WaitForMultipleObjects(static_cast<DWORD>(pendingEvents.size()),
		                                      pendingEvents.data(), FALSE, 1000);
		if (result == WAIT_TIMEOUT) {
			// nothing
			}
		else if (WAIT_OBJECT_0 <= result && result < (WAIT_OBJECT_0+pendingEvents.size())) {
			const int eventIdx = result - WAIT_OBJECT_0;
			const auto handle = pendingEvents[eventIdx];

			if (eventTab[handle].persist) {
				eventTab[handle].func(); }
			else {
				auto f = std::move(eventTab[handle].func);
				eventTab.erase(handle);
				f(); }}
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
	shouldQuit = true; }


void Reactor::ListenImpl(const rclmt::Event& event, std::function<void()> cb, bool persist) {
	if (eventTab.find(event.Get()) != end(eventTab)) {
		auto msg = fmt::sprintf("waitable handle %p already being monitored", event.Get());
		throw std::runtime_error(msg); }
	eventTab.emplace(event.Get(), ReactorEvent{ event.Get(), std::move(cb), persist }); }


void Reactor::ListenMany(const rclmt::Event& event, std::function<void()> cb) {
	ListenImpl(event, std::move(cb), true); }


void Reactor::ListenOnce(const rclmt::Event& event, std::function<void()> cb) {
	ListenImpl(event, std::move(cb), false); }


bool Reactor::RemoveEventByHandle(HANDLE handle) {
	auto found = eventTab.find(handle);
	if (found == end(eventTab)) {
		return false; }  // not found is a noop
	eventTab.erase(found);
	return true; }


}  // namespace rclmt
}  // namespace rqdq
