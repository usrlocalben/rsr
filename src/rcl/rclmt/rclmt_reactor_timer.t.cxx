#include <iostream>
#include <stdexcept>

#include "src/rcl/rclmt/rclmt_reactor_timer.hxx"
#include "src/rcl/rcls/rcls_timer.hxx"

#include <fmt/printf.h>

using namespace rqdq;
using namespace std;

constexpr int kTestSizeInMillionsOfIterations = 5;
int testCnt = 0;
rcls::Timer eventTimer;


void foo() {
	if (testCnt == 0) {
		eventTimer.Reset();
		cout << "begin event timer test\n" << flush; }
	if (++testCnt >= (kTestSizeInMillionsOfIterations * 1000000)) {
		auto elapsedSec = eventTimer.GetElapsed();
		auto eachMillis = elapsedSec * 1000 / (kTestSizeInMillionsOfIterations * 1000000);
		auto perSec = int64_t(1000.0 / eachMillis);
		auto msg = fmt::sprintf("completed %dM Delay events in %.4f sec, %.6f ms/event, %d event/sec",
								kTestSizeInMillionsOfIterations, elapsedSec, eachMillis, perSec);
		cout << msg << "\n";
		rclmt::Reactor::GetInstance().Stop(); }
	else {
		rclmt::Delay(0, foo); }}


int main(int argc, char **argv) {
	rclmt::Delay(0, foo);
	rclmt::Reactor::GetInstance().Run();
	return 0; }
