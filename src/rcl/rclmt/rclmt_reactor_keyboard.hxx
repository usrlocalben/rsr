#pragma once
#include <functional>

#include "src/rcl/rclmt/rclmt_reactor.hxx"

namespace rqdq {
namespace rclmt {

struct KeyEvent {
	bool down;
	char ch;
	uint32_t control;
	uint16_t scanCode; };

bool IsKeyDown(int scanCode);

void ListenKeyboard(std::function<void(KeyEvent)>, Reactor* reactor_=nullptr);


}  // namespace rclmt
}  // namespace rqdq
