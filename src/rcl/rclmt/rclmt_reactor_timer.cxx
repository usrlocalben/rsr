#include "src/rcl/rclmt/rclmt_reactor_timer.hxx"

#include <deque>

#include "src/rcl/rclmt/rclmt_reactor.hxx"
#include "src/rcl/rclmt/rclmt_event.hxx"

#include <fmt/printf.h>
#include <Windows.h>

namespace rqdq {
namespace {

constexpr bool TT_ONESHOT{false};
constexpr bool TT_PERIODIC{true};

struct TimerInfo {
	bool inUse{false};
	bool canceled{false};
	int id{0};
	bool type{TT_ONESHOT};
	rclmt::Reactor *reactor{nullptr};
	rclmt::Event event{rclmt::Event::MakeTimer()}; };

std::vector<TimerInfo> timers;

int timerSeq = 1;

std::pair<int, rclmt::Event*> AllocTimer(bool type, rclmt::Reactor* reactor) {
	TimerInfo* timer{nullptr};

	auto available = std::find_if(begin(timers), end(timers),
	                              [](auto& item) { return !item.inUse; });
	if (available != end(timers)) {
		timer = &*available;}
	else {
		timer = &timers.emplace_back(); }

	timer->inUse = true;
	timer->canceled = false;
	timer->type = type;
	timer->reactor = reactor;
	timer->id = timerSeq++;
	return {timer->id, &(timer->event)}; }


bool MaybeReleaseTimer(HANDLE h) {
	auto found = std::find_if(begin(timers), end(timers),
	                          [=](auto& item) { return item.event.Get() == h; });
	if (found == end(timers)) {
		throw std::runtime_error("MaybeReleaseTimer called with handle not found in timer collection"); }

	auto& timer = *found;
	bool canceled = timer.canceled;
	if (canceled || timer.type == TT_ONESHOT) {
		timer.inUse = false;
		timer.id = 0;
		timer.canceled = false; }
	return canceled; }


}  // namespace

namespace rclmt {

/**
 * Delay()
 * similar to Twisted callLater() or JavaScript's setTimeout()
 */
int Delay(const double millis, std::function<void()> func, Reactor* reactor_/*=nullptr*/) {
	auto& reactor = reactor_ != nullptr ? *reactor_ : Reactor::GetInstance();
	auto [id, timer] = AllocTimer(TT_ONESHOT, &reactor);
	timer->SignalIn(millis);
	auto rawHandle = timer->Get();
	reactor.ListenOnce(*timer, [func{std::move(func)}, rawHandle](){
		bool canceled = MaybeReleaseTimer(rawHandle);
		if (!canceled) {
			func(); } });
	return id; };


int Repeat(const double millis, std::function<void()> func, Reactor* reactor_/*=nullptr*/) {
	auto& reactor = reactor_ != nullptr ? *reactor_ : Reactor::GetInstance();
	auto [id, timer] = AllocTimer(TT_PERIODIC, &reactor);
	timer->SignalEvery(millis);
	auto rawHandle = timer->Get();
	reactor.ListenMany(*timer, [func{std::move(func)}, rawHandle, &reactor](){
		bool canceled = MaybeReleaseTimer(rawHandle);
		if (!canceled) {
			func(); }
		else {
			reactor.RemoveEventByHandle(rawHandle); } });
	return id; };


void CancelTimer(int id) {
	auto found = std::find_if(begin(timers), end(timers),
	                          [=](auto& item) { return item.id == id; });
	if (found == end(timers)) {
		return; }  // not found is a no-op

	auto& timer = *found;
	timer.canceled = true;
	timer.inUse = false;
	auto handle = timer.event.Get();
	auto success = CancelWaitableTimer(handle);
	if (success == 0) {
		auto error = GetLastError();
		auto msg = fmt::sprintf("CancelWaitableTimer error %d", error);
		throw std::runtime_error(msg); }
	timer.reactor->RemoveEventByHandle(handle); }


void CancelDelay(int id) {
	return CancelTimer(id); }


void CancelRepeat(int id) {
	return CancelTimer(id); }


}  // namespace rclmt
}  // namespace rqdq
