#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <xmmintrin.h>

#include "src/rml/rmlv/rmlv_soa.hxx"

namespace rqdq {
namespace rglv {

/**
 * barycentric coordinates are stored in a qfloat3
 * so that they may be used with e.g. fwidth(), smoothstep()
 *
 * v0 = x, v1 = y, v2 = z
 */
using BaryCoord = rmlv::qfloat3;


#if 1
struct VertexFloat1 { rmlv::qfloat  v0, v1, v2; };
struct VertexFloat2 { rmlv::qfloat2 v0, v1, v2; };
struct VertexFloat3 { rmlv::qfloat3 v0, v1, v2; };
#else
struct VertexFloat1 { float  v0, v1, v2; };
struct VertexFloat2 { rmlv::vec2 v0, v1, v2; };
struct VertexFloat3 { rmlv::vec3 v0, v1, v2; };
#endif


inline auto Interpolate(BaryCoord bc, VertexFloat1 uniform) {
	return rmlv::qfloat{
		bc.x*uniform.v0 + bc.y*uniform.v1 + bc.z*uniform.v2 }; }

inline auto Interpolate(BaryCoord bc, VertexFloat2 uniform) {
	return rmlv::qfloat2{
		bc.x*uniform.v0.x + bc.y*uniform.v1.x + bc.z*uniform.v2.x,
		bc.x*uniform.v0.y + bc.y*uniform.v1.y + bc.z*uniform.v2.y }; }

inline auto Interpolate(BaryCoord bc, VertexFloat3 uniform) {
	return rmlv::qfloat3{
		bc.x*uniform.v0.x + bc.y*uniform.v1.x + bc.z*uniform.v2.x,
		bc.x*uniform.v0.y + bc.y*uniform.v1.y + bc.z*uniform.v2.y,
		bc.x*uniform.v0.z + bc.y*uniform.v1.z + bc.z*uniform.v2.z }; }


}  // namespace rglv
}  // namespace rqdq
