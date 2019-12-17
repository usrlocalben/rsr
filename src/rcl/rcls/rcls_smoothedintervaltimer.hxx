#pragma once
#include "rcls_linearsmoother.hxx"

#include "3rdparty/pixeltoaster/PixelToaster.h"

namespace rqdq {
namespace rcls {

class SmoothedIntervalTimer {

	// DATA
	LinearSmoother ls_{};
	PixelToaster::Timer timer_{};

public:
    // MANIPULATORS
	void Start();
	void Stop();
	void Sample();

    // ACCESSORS
	auto Get() const -> double; };

// ============================================================================
//                             INLINE DEFINITIONS
// ============================================================================

						// ---------------------------
						// class SmoothedIntervalTimer
						// ---------------------------

// MANIPULATORS
inline
void SmoothedIntervalTimer::Start() {
    timer_.reset(); }

inline
void SmoothedIntervalTimer::Stop() {
    auto durationInMs = timer_.time() * 1000.0;
    ls_.Append(durationInMs); }

inline
void SmoothedIntervalTimer::Sample() {
    auto durationInMs = timer_.delta() * 1000.0;
    ls_.Append(durationInMs); }

// ACCESSORS
inline
auto SmoothedIntervalTimer::Get() const -> double {
    return ls_.Get(); }


}  // namespace rcls
}  // namespace rqdq
