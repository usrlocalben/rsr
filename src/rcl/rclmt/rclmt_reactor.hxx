#pragma once
#include <functional>
#include <vector>

#include "src/rcl/rclmt/rclmt_event.hxx"

#include <Windows.h>

namespace rqdq {
namespace rclmt {

class Reactor {
private:
	Reactor() = default;
public:
	static Reactor& GetInstance();

	void Run();
	void Stop();

	void ListenMany(const rclmt::Event&, std::function<void()> cb);
	void ListenOnce(const rclmt::Event&, std::function<void()> cb);
private:
	void ListenImpl(const rclmt::Event&, std::function<void()> cb, bool persist);
public:
	bool RemoveEventByHandle(HANDLE handle); };


}  // namespace rclmt
}  // namespace rqdq
