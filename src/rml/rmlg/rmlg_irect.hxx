#pragma once
#include "src/rml/rmlv/rmlv_vec.hxx"

namespace rqdq {
namespace rmlg {


struct irect {
	irect(const rmlv::ivec2& top_left, const rmlv::ivec2& bottom_right)
		:top_left(top_left), bottom_right(bottom_right) {}
	const auto height() const {
		return bottom.y - top.y; }
	const auto width() const {
		return right.x - left.x; }
	union {
		rmlv::ivec2 top_left;
		rmlv::ivec2 top;
		rmlv::ivec2 left;
		};
	union {
		rmlv::ivec2 bottom_right;
		rmlv::ivec2 bottom;
		rmlv::ivec2 right;
		};
	};


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

}  // close package namespace
}  // close enterprise namespace
