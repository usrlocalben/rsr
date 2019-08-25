#pragma once
#include "src/rgl/rglr/rglr_canvas.hxx"
#include "src/rml/rmlg/rmlg_irect.hxx"
#include "src/rml/rmlv/rmlv_soa.hxx"

#include "3rdparty/ryg-srgb/ryg-srgb.h"

namespace rqdq {
namespace rglr {

/*constants used for preparing 2x2 SoA gangs*/
const rmlv::mvec4f FQX{_mm_setr_ps(0,1,0,1)};
const rmlv::mvec4f FQY{_mm_setr_ps(0,0,1,1)};

void fillRect(rmlg::irect rect, float value, QFloatCanvas& dst);
void fillRect(rmlg::irect rect, rmlv::vec4 value, QFloat4Canvas& dst);

void strokeRect(rmlg::irect rect, PixelToaster::TrueColorPixel color, TrueColorCanvas& dst);

void downsampleRect(rmlg::irect rect, QFloat4Canvas& src, FloatingPointCanvas& dst);

void copyRect(rmlg::irect rect, QFloat4Canvas& src, FloatingPointCanvas& dst);

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


template <class PGM, class CONVERTER>
void copyRect(
	const rmlg::irect rect,
	const QFloat4Canvas& src1,
	TrueColorCanvas& dst
) {
	using rmlv::qfloat2;
	using rmlv::qfloat3;
	using rmlv::qfloat4;
	using rmlv::mvec4f;
	using rmlv::mvec4i;

	const float dw = 1.0f / dst.width();
	const float dh = 1.0f / dst.height();

	const qfloat2 dqx{ dw*2, 0 };
	const qfloat2 dqy{ 0, -dh*2 };
	qfloat2 q_row{
		mvec4f{rect.left.x / float(dst.width())} +mvec4f{dw}*rglr::FQX,
		mvec4f{(dst.height() - 1 - rect.top.y) / float(dst.height())} - mvec4f{dh}*rglr::FQY
	};

	for (int y = rect.top.y; y < rect.bottom.y; y += 2, q_row+=dqy) {

		auto destRow1Addr = &dst.data()[y * dst.stride() + rect.left.x];
		auto destRow2Addr = destRow1Addr + dst.stride();

		auto* source1Addr = &src1.cdata()[(y >> 1) * (dst.width() >> 1) + (rect.left.x >> 1)];

		qfloat2 q{ q_row };
		for (int x = rect.left.x; x < rect.right.x; x += 4, destRow1Addr+=4, destRow2Addr+=4) {
			mvec4i packed[2];
			for (int sub = 0; sub < 2; sub++, source1Addr++, q+=dqx) {

				// load source1 color
				__m128 scr = _mm_load_ps(reinterpret_cast<const float*>(&source1Addr->r));
				__m128 scg = _mm_load_ps(reinterpret_cast<const float*>(&source1Addr->g));
				__m128 scb = _mm_load_ps(reinterpret_cast<const float*>(&source1Addr->b));
				qfloat3 source1Color{ scr, scg, scb };

				// shade
				qfloat3 fragColor = PGM::ShadeCanvas(q, source1Color);

				// convert
				packed[sub] = CONVERTER::to_tc(fragColor); }

			_mm_stream_si128(reinterpret_cast<__m128i*>(destRow1Addr), _mm_unpacklo_epi64(packed[0].v, packed[1].v));
			_mm_stream_si128(reinterpret_cast<__m128i*>(destRow2Addr), _mm_unpackhi_epi64(packed[0].v, packed[1].v)); }}}


}  // namespace rglr
}  // namespace rqdq
