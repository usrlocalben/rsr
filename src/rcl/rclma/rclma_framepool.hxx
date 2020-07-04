#pragma once

namespace rqdq {
namespace rclma {

namespace framepool {

void Init();
void Reset();
auto Allocate(int /*amt*/) -> void*;

}  // namespace framepool


}  // namespace rclma
}  // namespace rqdq
