#include "src/rcl/rclmt/rclmt_reactor_delay.hxx"

#include "src/rcl/rclmt/rclmt_reactor.hxx"
#include "src/rcl/rclmt/rclmt_event.hxx"

#include <fmt/printf.h>
#include <Windows.h>

namespace rqdq {
namespace {

// delay
struct DelayInfo {
	bool inUse = false;
	bool canceled = false;
	int id = 0;
	rclmt::Event event; };

std::vector<DelayInfo> delays;

int delaySeq = 1;

std::pair<int, rclmt::Event&> GetDelay() {
	const int id = delaySeq++;
	for (int i=0; i<delays.size(); i++) {
		auto& item = delays[i];
		if (!item.inUse) {
			item.inUse = true;
			item.id = delaySeq;
			return {i, item.event}; }}
	delays.emplace_back(DelayInfo{true, false, id, rclmt::Event::MakeTimer()});
	return {id, delays.back().event}; };


bool ReleaseTimer(HANDLE h) {
	auto found = std::find_if(delays.begin(), delays.end(), [=](auto& item) { return item.event.Get() == h; });
	if (found == delays.end()) {
		throw std::runtime_error("ReleaseTimer called with handle not found in delays collection"); }
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

	auto [id, timer] = GetDelay();
	timer.SignalIn(millis);
	auto rawHandle = timer.Get();
	reactor.ListenOnce(timer, [func{std::move(func)}, rawHandle](){
		bool canceled = ReleaseTimer(rawHandle);
		if (!canceled) {
			func(); }
		});
	return id; };


void CancelDelay(int id, Reactor* reactor_/*=nullptr*/) {
	auto& reactor = reactor_ != nullptr ? *reactor_ : Reactor::GetInstance();

	auto found = std::find_if(delays.begin(), delays.end(), [=](auto& item) { return item.id = id; });
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
