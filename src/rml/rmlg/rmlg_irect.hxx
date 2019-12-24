#pragma once
#include <algorithm>
#include <ostream>

#include "src/rml/rmlv/rmlv_vec.hxx"

namespace rqdq {
namespace rmlg {


struct irect {

	// DATA
	union {
		rmlv::ivec2 top_left;
		rmlv::ivec2 top;
		rmlv::ivec2 left; };
	union {
		rmlv::ivec2 bottom_right;
		rmlv::ivec2 bottom;
		rmlv::ivec2 right; };

	// CREATORS
	irect() = default;
	constexpr irect(rmlv::ivec2 tl, rmlv::ivec2 br) :
		top_left(tl),
		bottom_right(br) {}
	constexpr irect(const irect& other) :
		top_left(other.top_left),
		bottom_right(other.bottom_right) {}


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
auto Intersect(const irect& a, const irect& b) -> irect {
	irect out;
	out.left.x   = std::max(a.left.x,   b.left.x);
	out.top.y    = std::max(a.top.y,    b.top.y);
	out.right.x  = std::min(a.right.x,  b.right.x);
	out.bottom.y = std::min(a.bottom.y, b.bottom.y);
	return out; }

// FREE OPERATORS
inline
auto operator==(const irect& a, const irect& b) -> bool {
	return a.top_left==b.top_left && a.bottom_right==b.bottom_right; }

inline
auto operator!=(const irect& a, const irect& b) -> bool {
	return a.top_left!=b.top_left || a.bottom_right!=b.bottom_right; }

inline
auto operator<<(std::ostream& os, const irect& a) -> std::ostream& {
	os << "<irect tl(" << a.top_left.x << ", " << a.top_left.y << ") br(" << a.bottom_right.x << ", " << a.bottom_right.y << ")>";
	return os; }


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
