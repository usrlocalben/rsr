#pragma once
#include <algorithm>
#include <ostream>

#include "src/rml/rmlv/rmlv_math.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"

namespace rqdq {
namespace rmlg {

struct irect {

	// DATA
	union {
		rmlv::ivec2 topLeft;
		rmlv::ivec2 top;
		rmlv::ivec2 left; };
	union {
		rmlv::ivec2 bottomRight;
		rmlv::ivec2 bottom;
		rmlv::ivec2 right; };

	// CREATORS
	irect() = default;
	constexpr irect(rmlv::ivec2 tl, rmlv::ivec2 br) :
		topLeft(tl),
		bottomRight(br) {}
	constexpr irect(const irect& other) :
		topLeft(other.topLeft),
		bottomRight(other.bottomRight) {}

	// ACCESSORS
	constexpr auto height() const -> int {
		return bottom.y - top.y; }

	constexpr auto width() const -> int {
		return right.x - left.x; } };

// FREE FUNCTIONS

/**
 * return an irect that is the intersection of two irects.
 * if the irects do not intersect, the returned irect will
 * be degenerate.
 */
inline
auto Intersect(irect a, irect b) -> irect {
	return {
		vmax(a.topLeft, b.topLeft),
		vmin(a.bottomRight, b.bottomRight) }; }

// FREE OPERATORS
inline
auto operator==(const irect& a, const irect& b) -> bool {
	return a.topLeft==b.topLeft && a.bottomRight==b.bottomRight; }

inline
auto operator!=(const irect& a, const irect& b) -> bool {
	return a.topLeft!=b.topLeft || a.bottomRight!=b.bottomRight; }

/*
struct ibox3 {
	union {
		ivec3 top_left_back;
		ivec3 top;
		ivec3 left;
		ivec3 back; };
	union {
		ivec3 bottom_right_front;
		ivec3 bottom;
		ivec3 right;
		ivec3 front; };

	ibox3(const ivec3& tlb, const ivec3& brf)
		:top_left_back(tlp), bottom_right_front(brf) {}

	int height() const {
		return bottom.y - top.y; }

	int width() const {
		return right.x - left.x; }

	int depth() const {
		return front.z - back.z; } };
*/

}  // namespace rmlg
}  // namespace rqdq


inline
auto operator<<(std::ostream& os, rqdq::rmlg::irect a) -> std::ostream& {
	os << "<irect tl(" << a.topLeft.x << ", " << a.topLeft.y << ") br(" << a.bottomRight.x << ", " << a.bottomRight.y << ")>";
	return os; }
