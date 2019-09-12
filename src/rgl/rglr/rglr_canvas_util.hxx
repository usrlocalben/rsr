#pragma once
#include "src/rgl/rglr/rglr_canvas.hxx"
#include "src/rml/rmlg/rmlg_irect.hxx"
#include "src/rml/rmlv/rmlv_mvec4.hxx"
#include "src/rml/rmlv/rmlv_soa.hxx"

#include "3rdparty/ryg-srgb/ryg-srgb.h"

namespace rqdq {
namespace rglr {

TrueColorCanvas make_subcanvas(TrueColorCanvas& src, rmlg::irect rect);


inline rmlv::mvec4i to_tc_ryg(const rmlv::qfloat4 color) {
	using ryg::float_to_srgb8_var2_SSE2;
	return rmlv::shl<16>(float_to_srgb8_var2_SSE2(color.x.v)) |
	       rmlv::shl< 8>(float_to_srgb8_var2_SSE2(color.y.v)) |
	                     float_to_srgb8_var2_SSE2(color.z.v); };


inline rmlv::mvec4i to_tc_basic(const rmlv::qfloat4 color) {
	auto fix = [](__m128 a) {
		__m128 r0 = _mm_min_ps(a, _mm_set1_ps(1.0f));
		r0 = _mm_max_ps(r0, _mm_setzero_ps());
		r0 = _mm_mul_ps(r0, _mm_set1_ps(255.0f));
		return rmlv::ftoi(r0); };
	return rmlv::shl<16>(fix(color.x.v)) |
		   rmlv::shl< 8>(fix(color.y.v)) |
				         fix(color.z.v); };


inline rmlv::mvec4i to_tc_ryg(const rmlv::qfloat3 color) {
	using ryg::float_to_srgb8_var2_SSE2;
	return rmlv::shl<16>(float_to_srgb8_var2_SSE2(color.x.v)) |
	       rmlv::shl< 8>(float_to_srgb8_var2_SSE2(color.y.v)) |
	                     float_to_srgb8_var2_SSE2(color.z.v); };


inline rmlv::mvec4i to_tc_basic(const rmlv::qfloat3 color) {
	const __m128 s = _mm_set1_ps(255.9999);

	/* method 1, clamp with min, merge with shift/or */
	__m128i x = _mm_cvttps_epi32(_mm_min_ps(_mm_mul_ps(color.x.v, s), s));
	__m128i y = _mm_cvttps_epi32(_mm_min_ps(_mm_mul_ps(color.y.v, s), s));
	__m128i z = _mm_cvttps_epi32(_mm_min_ps(_mm_mul_ps(color.z.v, s), s));
	x = _mm_slli_epi32(x, 16);
	y = _mm_slli_epi32(y, 8);
	__m128i out = _mm_or_si128(_mm_or_si128(x, y), z);

	/* method 2 (broken), megge and clamp using packs/packus */
	/*
	__m128 fx = _mm_mul_ps(color.x.v, s);
	__m128 fy = _mm_mul_ps(color.y.v, s);
	__m128 fz = _mm_mul_ps(color.z.v, s);
	__m128 fa = _mm_setzero_ps();
	_MM_TRANSPOSE4_PS(fx, fy, fz, fa);

	__m128i p1 = _mm_cvttps_epi32(fx);
	__m128i p2 = _mm_cvttps_epi32(fy);
	__m128i p3 = _mm_cvttps_epi32(fz);
	__m128i p4 = _mm_cvttps_epi32(fa);

	__m128i p1p2 = _mm_packs_epi32(p1, p2);
	__m128i p3p4 = _mm_packs_epi32(p3, p4);
	__m128i out = _mm_packus_epi16(p1p2, p3p4);
	*/

	return out; }

/*
	0000000000000000
	---r---r---r---r
	---g---g---g---g
	---b---b---b---b

new:
	unpack*4   1*4
	move*4     1*4
	cvt*4      3*4
	packus32   1
	packus32   1
	packus16   1
	           (23)

old:
	min*3   3*3
	cvt*3   3*3
	shl*2   1*2
	or*2    1*2
            (22)


transpose: (unpackX4, moveX4)
   	0000---r---g---b
   	0000---r---g---b
   	0000---r---g---b
   	0000---r---g---b

	packus32 00-r-g-b00-r-g-b
	packus32 00-r-g-b00-r-g-b
	packus16 0rgb0rgb0rgb0rgb

	packs x,y = -r-r-r-r-g-g-g-g

	0xyz0xyz0xyz0xyz
*/
/*
	auto fix = [](__m128 r0) {
		r0 = _mm_mul_ps(r0, s);
		r0 = _mm_min_ps(r0, s);

		r0 = 
		= _mm_min_ps(a, _mm_set1_ps(1.0f));
		// r0 = _mm_max_ps(r0, _mm_setzero_ps());
		__m128 r0;
		r0 = _mm_mul_ps(a, _mm_set1_ps(255.999));
		return rmlv::ftoi(r0); };
	return rmlv::shl<16>(fix(color.x.v)) |
		   rmlv::shl< 8>(fix(color.y.v)) |
				         fix(color.z.v); };
*/

struct LinearColor {
	static inline rmlv::mvec4i to_tc(const rmlv::qfloat3 color) {
		return to_tc_basic(color); }
	static inline rmlv::mvec4i to_tc(const rmlv::qfloat4 color) {
		return to_tc_basic(color); } };


struct sRGB {
	static inline rmlv::mvec4i to_tc(const rmlv::qfloat3 color) {
		return to_tc_ryg(color); }
	static inline rmlv::mvec4i to_tc(const rmlv::qfloat4 color) {
		return to_tc_ryg(color); } };


}  // namespace rglr
}  // namespace rqdq
