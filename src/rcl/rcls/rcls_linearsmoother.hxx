#pragma once

namespace rqdq {
namespace rcls {

class LinearSmoother {

	static constexpr double speed = 0.1;

	// DATA
	double value_{0};

public:
	// MANIPULATORS
	void Reset();
	void Append(double a);

	// ACCESSORS
	auto Get() const -> double; };

// ============================================================================
//                                 INLINE DEFINITIONS
// ============================================================================

                            // --------------------
                            // class LinearSmoother
                            // --------------------
// MANIPULATORS
inline
void LinearSmoother::Reset() {
    value_ = 0; }

inline
void LinearSmoother::Append(double a) {
	value_ = (1-speed)*value_ + speed*a; }

// ACCESSORS
auto LinearSmoother::Get() const -> double {
    return value_; }


}  // namespace rcls
}  // namespace rqdq
