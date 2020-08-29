#pragma once
#include "src/rml/rmlv/rmlv_vec.hxx"
#include "src/rml/rmlv/rmlv_soa.hxx"

namespace rqdq {
namespace rmlg {

/**
 * area of parallelogram, or
 * area of triangle * 2
 *
 * result is positive if p1,p2,p3 have CCW winding
 */
auto Area(rmlv::vec2 p1, rmlv::vec2 p2, rmlv::vec2 p3) -> float;
auto Area(rmlv::qfloat2 p1, rmlv::qfloat2 p2, rmlv::qfloat2 p3) -> rmlv::qfloat;

inline
auto Area(rmlv::qfloat2 p1, rmlv::qfloat2 p2, rmlv::qfloat2 p3) -> rmlv::qfloat {
	auto d31 = p3 - p1;
	auto d21 = p2 - p1;
	return d31.x*d21.y - d31.y*d21.x; }

inline
auto Area(rmlv::vec2 p1, rmlv::vec2 p2, rmlv::vec2 p3) -> float {
	auto d31 = p3 - p1;
	auto d21 = p2 - p1;
	return  d31.x*d21.y - d31.y*d21.x; }

}  // namespace rmlg
}  // namespace rqdq
