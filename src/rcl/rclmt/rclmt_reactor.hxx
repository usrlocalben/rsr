#pragma once
#include <functional>
#include <vector>

#include "src/rcl/rclmt/rclmt_event.hxx"

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
	bool RemoveEventByHandle(void* handle); };


}  // namespace rclmt
}  // namespace rqdq
