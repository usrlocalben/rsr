#pragma once
#include "src/rml/rmlv/rmlv_vec.hxx"

namespace rqdq {
namespace rmlg {

/**
 * 2*Area of a 2D triangle
 * result is positive if p1,p2,p3 have CCW winding
 */
inline float triangle2Area(const rmlv::vec4 p1, const rmlv::vec4 p2, const rmlv::vec4 p3) {
	const auto d31 = p3 - p1;
	const auto d21 = p2 - p1;
	const float area = d31.x*d21.y - d31.y*d21.x;
	return area; }

inline float triangle2Area(const rmlv::vec2 p1, const rmlv::vec2 p2, const rmlv::vec2 p3) {
	const auto d31 = p3 - p1;
	const auto d21 = p2 - p1;
	const float area = d31.x*d21.y - d31.y*d21.x;
	return area; }

}  // close package namespace
}  // close enterprise namespace
