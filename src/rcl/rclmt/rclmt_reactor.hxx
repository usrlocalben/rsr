#pragma once
#include <functional>
#include <vector>

#include "src/rcl/rclmt/rclmt_event.hxx"

#include <Windows.h>

namespace rqdq {
namespace rclmt {

struct ReactorEvent {
	HANDLE handle{nullptr};
	std::function<void()> func;
	bool persist{false}; };


class Reactor {
private:
	Reactor() = default;
public:
	static Reactor& GetInstance();

	void Run();
	void Stop();

	void ListenMany(const rclmt::Event&, std::function<void()> cb);
	void ListenOnce(const rclmt::Event&, std::function<void()> cb);
	bool RemoveEventByHandle(HANDLE handle);

private:
	bool d_shouldQuit = false;
	std::vector<ReactorEvent> d_events; };


}  // namespace rclmt
}  // namespace rqdq
