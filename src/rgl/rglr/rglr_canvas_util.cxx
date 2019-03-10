#include "src/rgl/rglr/rglr_canvas_util.hxx"

#include <algorithm>

#include "src/rgl/rglr/rglr_canvas.hxx"
#include "src/rml/rmlg/rmlg_irect.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"

#include "3rdparty/pixeltoaster/PixelToaster.h"

namespace rqdq {
namespace rglr {

rglr::TrueColorCanvas make_subcanvas(rglr::TrueColorCanvas& src, const rmlg::irect rect) {
	auto left = rect.left.x < 0 ? src.width() + rect.left.x : rect.left.x;
	auto top = rect.top.y < 0 ? src.height() + rect.top.y : rect.top.y;

	auto width = rect.width() < 0 ? src.width() + rect.width() : rect.width();
	auto height = rect.height() < 0 ? src.height() + rect.height() : rect.height();

	// clip to src bounds
	width = std::min(src.width() - left, width);
	height = std::min(src.height() - top, height);

	return rglr::TrueColorCanvas(src.data() + (top * src.stride()) + left, width, height, src.stride()); }


void fillRect(const rmlg::irect rect, const float value, QFloatCanvas& dst) {
	const int rect_height_in_quads = (rect.height()) / 2;
	const int rect_width_in_quads = (rect.width()) / 2;

	int row_offset = rect.top.y / 2 * dst.stride2();
	int col_offset = rect.left.x / 2;

	const __m128 r0 = _mm_set1_ps(value);
	for (int yy = 0; yy < rect_height_in_quads; yy++) {
		for (int xx = 0; xx < rect_width_in_quads; xx++) {
			dst.data()[row_offset + col_offset + xx] = r0; }
		row_offset += dst.stride2(); }}


void fillRect(const rmlg::irect rect, const rmlv::vec4 value, QFloat4Canvas& dst) {
	const int rect_height_in_quads = (rect.height()) / 2;
	const int rect_width_in_quads = (rect.width()) / 2;

	int row_offset = (rect.top.y / 2) * dst.stride2();
	int col_offset = rect.left.x / 2;

	const __m128 red = _mm_set1_ps(value.x);
	const __m128 green = _mm_set1_ps(value.y);
	const __m128 blue = _mm_set1_ps(value.z);
	const __m128 alpha = _mm_set1_ps(value.w);

	auto row_dst = &dst.data()[row_offset + col_offset];
	for (int yy = 0; yy < rect_height_in_quads; yy++) {
		auto _dst = row_dst;
		for (int xx = 0; xx < rect_width_in_quads; xx++) {
			_mm_store_ps(reinterpret_cast<float*>(&_dst->r), red);
			_mm_store_ps(reinterpret_cast<float*>(&_dst->g), green);
			_mm_store_ps(reinterpret_cast<float*>(&_dst->b), blue);
			_mm_store_ps(reinterpret_cast<float*>(&_dst->a), alpha);
			_dst++; }
		row_dst += dst.stride2(); }}


void strokeRect(const rmlg::irect rect, PixelToaster::TrueColorPixel color, rglr::TrueColorCanvas& dst) {
	// top row
	int yy = rect.top.y;
	for (int xx = rect.left.x; xx < rect.right.x; xx++) {
		dst.data()[yy * dst.stride() + xx] = color; }

	// left & right columns between
	for (yy += 1; yy < rect.bottom.y - 1; yy++) {
		dst.data()[yy * dst.stride() + rect.left.x] = color;
		dst.data()[yy * dst.stride() + rect.right.x - 1] = color; }

	// bottom row
	for (int xx = rect.left.x; xx < rect.right.x; xx++) {
		dst.data()[yy * dst.stride() + xx] = color; }}


void downsampleRect(const rmlg::irect rect, QFloat4Canvas& src, FloatingPointCanvas& dst) {
	const __m128 one_quarter = _mm_set1_ps(0.25f);
	for (int y = rect.top.y; y < rect.bottom.y; y += 2) {
		auto dstpx = &dst.data()[(y >> 1) * dst.stride() + (rect.left.x >> 1)];
		auto srcpx = &src.data()[(y >> 1) * (src.width() >> 1) + (rect.left.x >> 1)];

		for (int x = rect.left.x; x < rect.right.x; x += 2) {
			_mm_prefetch(reinterpret_cast<const char*>(srcpx + (src.width() >> 1)), _MM_HINT_T0);
			__m128 r0 = _mm_load_ps(reinterpret_cast<const float*>(&srcpx->r.v));
			__m128 r1 = _mm_load_ps(reinterpret_cast<const float*>(&srcpx->g.v));
			__m128 r2 = _mm_load_ps(reinterpret_cast<const float*>(&srcpx->b.v));
			//__m128 r3 = _mm_load_ps(reinterpret_cast<const float*>(&srcpx->a.v));
			__m128 r3 = _mm_setzero_ps();
			_MM_TRANSPOSE4_PS(r0, r1, r2, r3);
			__m128 sum = _mm_add_ps(_mm_add_ps(_mm_add_ps(r0, r1), r2), r3);
			__m128 final = _mm_mul_ps(sum, one_quarter);
			_mm_stream_ps(reinterpret_cast<float*>(&dstpx->v), final);
			//dstpx->v = _mm_mul_ps(one_quarter, sum);
			srcpx++;
			dstpx++; }}}


void copyRect(const rmlg::irect rect, QFloat4Canvas& src, FloatingPointCanvas& dst) {
	auto dstRowAddr = &dst.data()[rect.top.y*dst.stride()];
	auto srcRowAddr = &src.data()[(rect.top.y >> 1) * (src.width() >> 1)];

	for (int y = rect.top.y; y < rect.bottom.y; y += 2) {

		auto dstAddrR1 = &dstRowAddr[rect.left.x];
		auto dstAddrR2 = dstAddrR1 + dst.stride();
		auto srcAddr = &srcRowAddr[rect.left.x / 2];

		for (int x = rect.left.x; x < rect.right.x; x += 4) {
			// BEGIN process 4x2 pixels (2 quads)
			__m128 l0 = _mm_load_ps(reinterpret_cast<const float*>(&srcAddr[0].r));
			__m128 l1 = _mm_load_ps(reinterpret_cast<const float*>(&srcAddr[0].g));
			__m128 l2 = _mm_load_ps(reinterpret_cast<const float*>(&srcAddr[0].b));
			__m128 l3 = _mm_setzero_ps();
			_MM_TRANSPOSE4_PS(l0, l1, l2, l3);

			__m128 r0 = _mm_load_ps(reinterpret_cast<const float*>(&srcAddr[1].r));
			__m128 r1 = _mm_load_ps(reinterpret_cast<const float*>(&srcAddr[1].g));
			__m128 r2 = _mm_load_ps(reinterpret_cast<const float*>(&srcAddr[1].b));
			__m128 r3 = _mm_setzero_ps();
			_MM_TRANSPOSE4_PS(r0, r1, r2, r3);

			_mm_stream_ps(reinterpret_cast<float*>(&dstAddrR1[0]), l0);
			_mm_stream_ps(reinterpret_cast<float*>(&dstAddrR1[1]), l1);
			_mm_stream_ps(reinterpret_cast<float*>(&dstAddrR1[2]), r0);
			_mm_stream_ps(reinterpret_cast<float*>(&dstAddrR1[3]), r1);

			_mm_stream_ps(reinterpret_cast<float*>(&dstAddrR2[0]), l2);
			_mm_stream_ps(reinterpret_cast<float*>(&dstAddrR2[1]), l3);
			_mm_stream_ps(reinterpret_cast<float*>(&dstAddrR2[2]), r2);
			_mm_stream_ps(reinterpret_cast<float*>(&dstAddrR2[3]), r3);
			//  END  process 4x2 pixels (2 quads)

			dstAddrR1 += 4;
			dstAddrR2 += 4;
			srcAddr += 2; }

		dstRowAddr += dst.stride() * 2;
		srcRowAddr += src.width() / 2; }}


/*
void copy_canvas_slow(FloatingPointCanvas& src, rglr::TrueColorCanvas& dst) {
	for (int y = 0; y < src.height(); y++) {
		for (int x = 0; x < src.width(); x++) {
			dst.data()[y*dst.stride() + x] = to_tc_from_fpp(src.data()[y*src.stride() + x]); }}}
*/

/*
PixelToaster::FloatingPointPixel to_fp(const rmlv::vec4& v) {
	return PixelToaster::FloatingPointPixel{ v.x, v.y, v.z }; }
*/

/*
void canvas_shader_rows_x(
	const int y0, const int y1,
	const std::string& shaderName,
	const QFloat4Canvas& src1,
	const FloatingPointCanvas& src2,
	rglr::TrueColorCanvas& dst
) {
	if (shaderName == "Glow") {
		canvas_shader_rows(y0, y1, GlowShader(), src1, src2, dst);
	}
	else if (shaderName == "Dof") {
		// canvas_shader_rows(y0, y1, DofShader(), src1, src2, dst);
	}
}
*/


}  // namespace rglr
}  // namespace rqdq
