#include "src/rcl/rclmt/rclmt_reactor_delay.hxx"

#include "src/rcl/rclmt/rclmt_reactor.hxx"
#include "src/rcl/rclmt/rclmt_event.hxx"

#include <fmt/printf.h>
#include <Windows.h>

namespace rqdq {
namespace {

// delay
struct DelayInfo {
	bool inUse{false};
	bool canceled{false};
	int id{0};
	rclmt::Event event{rclmt::Event::MakeTimer()}; };

std::vector<DelayInfo> delays;

int delaySeq = 1;

std::pair<int, rclmt::Event*> AllocDelay() {
	DelayInfo* delay{nullptr};

	auto available = std::find_if(delays.begin(), delays.end(),
	                              [](auto& item) { return !item.inUse; });
	if (available != delays.end()) {
		delay = &(*available);}
	else {
		delays.emplace_back();
		delay = &delays.back();}

	delay->inUse = true;
	delay->canceled = false;
	delay->id = delaySeq++;
	return {delay->id, &delays.back().event}; }


bool ReleaseDelay(HANDLE h) {
	auto found = std::find_if(delays.begin(), delays.end(),
	                          [=](auto& item) { return item.event.Get() == h; });
	if (found == delays.end()) {
		throw std::runtime_error("ReleaseDelay called with handle not found in delays collection"); }
	bool canceled = found->canceled;
	found->inUse = false;
	found->id = 0;
	found->canceled = false;
	return canceled; }

}  // namespace

namespace rclmt {

/**
 * Delay()
 * similar to Twisted callLater() or JavaScript's setTimeout()
 */
int Delay(const double millis, std::function<void()> func, Reactor* reactor_/*=nullptr*/) {
	auto& reactor = reactor_ != nullptr ? *reactor_ : Reactor::GetInstance();

	auto [id, timer] = AllocDelay();
	timer->SignalIn(millis);
	auto rawHandle = timer->Get();
	reactor.ListenOnce(*timer, [func{std::move(func)}, rawHandle](){
		bool canceled = ReleaseDelay(rawHandle);
		if (!canceled) {
			func(); } });
	return id; };


void CancelDelay(int id, Reactor* reactor_/*=nullptr*/) {
	auto& reactor = reactor_ != nullptr ? *reactor_ : Reactor::GetInstance();

	auto found = std::find_if(delays.begin(), delays.end(),
	                          [=](auto& item) { return item.id = id; });
	if (found == delays.end()) {
		return; }  // not found is a no-op

	found->canceled = true;
	auto handle = found->event.Get();
	auto result = CancelWaitableTimer(handle);
	if (result != 0) {
		auto error = GetLastError();
		auto msg = fmt::sprintf("CancelWaitableTimer error %d", error);
		throw std::runtime_error(msg); }
	reactor.RemoveEventByHandle(handle);
	found->inUse = false; }


}  // namespace rclmt
}  // namespace rqdq
