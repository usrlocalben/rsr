#pragma once
#include <chrono>

namespace rqdq {
namespace rcls {

class Timer {
public:
	Timer() :beg_(clock_::now()) {}
	void Reset() {
		beg_ = clock_::now(); }
	double GetElapsed() const {
		return std::chrono::duration_cast<second_>(clock_::now() - beg_).count(); }
private:
	typedef std::chrono::high_resolution_clock clock_;
	typedef std::chrono::duration<double, std::ratio<1>> second_;
	std::chrono::time_point<clock_> beg_;
};

}  // close package namespace
}  // close enterprise namespace
