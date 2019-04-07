#pragma once

namespace rqdq {
namespace rclma {

namespace framepool {
	void Init();
	void Reset();
	void* Allocate(int /*amt*/);


}  // namespace framepool


}  // namespace rclma
}  // namespace rqdq
