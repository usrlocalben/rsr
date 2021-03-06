#pragma once
#include <xmmintrin.h>

#include "src/rgl/rglv/rglv_interpolate.hxx"
#include "src/rml/rmlv/rmlv_mvec4.hxx"
#include "src/rml/rmlv/rmlv_soa.hxx"

namespace rqdq {
namespace rglv {

inline rmlv::mvec4f dFdx(const rmlv::mvec4f& a) {
	return a.yyww() - a.xxzz(); }

inline rmlv::mvec4f dFdy(const rmlv::mvec4f& a) {
	return a.xyxy() - a.zwzw(); }

inline rmlv::qfloat fwidth(const rmlv::qfloat& a) {
	return abs(dFdx(a)) + abs(dFdy(a)); }

inline rmlv::qfloat2 fwidth(const rmlv::qfloat2& a) {
	return rmlv::qfloat2{
		abs(dFdx(a.x)) + abs(dFdy(a.x)),
		abs(dFdx(a.y)) + abs(dFdy(a.y))
		}; }

inline rmlv::qfloat3 fwidth(const rmlv::qfloat3& a) {
	return rmlv::qfloat3{
		abs(dFdx(a.x)) + abs(dFdy(a.x)),
		abs(dFdx(a.y)) + abs(dFdy(a.y)),
		abs(dFdx(a.z)) + abs(dFdy(a.z))
		}; }


inline rmlv::qfloat3 hash3(const rmlv::qfloat& n) {
	/*utility from iq's shadertoy entry "Repelling"*/
	return fract(sin(rmlv::qfloat3{n, n + 1.0F, n + 2.0F}) * 43758.5453123F); }


inline rmlv::qfloat3 smoothstep(const float a, const rmlv::qfloat3& b, const rmlv::qfloat3& x) {
	const rmlv::mvec4f _a{ a };
	return rmlv::qfloat3{
		smoothstep(_a, b.x, x.x),
		smoothstep(_a, b.y, x.y),
		smoothstep(_a, b.z, x.z) }; }


inline rmlv::mvec4f ddx(const rmlv::mvec4f& a) {
	return rmlv::mvec4f(_mm_sub_ps(_mm_movehdup_ps(a.v), _mm_moveldup_ps(a.v))); }


inline rmlv::mvec4f ddy(const rmlv::mvec4f& a) {
	// r0 = a2 - a0
	// r1 = a3 - a1
	// r2 = a2 - a0
	// r3 = a3 - a1

	// SSE3 self-shuffles
	__m128 quadtop = _mm_shufd(a.v, _MM_SHUFFLE(0, 1, 0, 1));
	__m128 quadbot = _mm_shufd(a.v, _MM_SHUFFLE(2, 3, 2, 3));
	//__m128 quadtop = _mm_shuffle_ps( a, a, _MM_SHUFFLE(0,1,0,1) );
	//__m128 quadbot = _mm_shuffle_ps( a, a, _MM_SHUFFLE(2,3,2,3) );

	return rmlv::mvec4f(_mm_sub_ps(quadbot, quadtop)); }


}  // namespace rglv
}  // namespace rqdq
