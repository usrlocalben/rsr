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
	auto fix = [](__m128 a) {
		__m128 r0 = _mm_min_ps(a, _mm_set1_ps(1.0f));
		r0 = _mm_max_ps(r0, _mm_setzero_ps());
		r0 = _mm_mul_ps(r0, _mm_set1_ps(255.0f));
		return rmlv::ftoi(r0); };
	return rmlv::shl<16>(fix(color.x.v)) |
		   rmlv::shl< 8>(fix(color.y.v)) |
				         fix(color.z.v); };


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
