#pragma once
#include <functional>

#include "src/rcl/rclmt/rclmt_reactor.hxx"

namespace rqdq {
namespace rclmt {

int Delay(double millis, std::function<void()> func, Reactor* reactor=nullptr);
int Repeat(double millis, std::function<void()> func, Reactor* reactor=nullptr);

void CancelDelay(int id);
void CancelRepeat(int id);


}  // namespace rclmt
}  // namespace rqdq
