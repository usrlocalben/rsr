#pragma once

namespace rqdq {
namespace rclma {

namespace framepool {
	void Init();
	void Reset();
	void* Allocate(const int);


}  // namespace framepool


}  // namespace rclma
}  // namespace rqdq
