#include "src/rgl/rglr/rglr_algorithm.hxx"

#include <algorithm>

#include "src/rgl/rglr/rglr_canvas.hxx"
#include "src/rml/rmlg/rmlg_irect.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"

#include "3rdparty/pixeltoaster/PixelToaster.h"

namespace rqdq {
namespace rglr {

void Fill(QFloatCanvas& dst, const float value, const rmlg::irect rect) {
	const int rectHeightInQuads = rect.height() / 2;
	const int rectWidthInQuads = rect.width() / 2;

	const __m128 r0 = _mm_set1_ps(value);
	auto* p = dst.data() + rect.top.y/2*dst.stride2() + rect.left.x/2;
	int rowIncr = dst.stride2() - rectWidthInQuads;

	for (int yy=0; yy<rectHeightInQuads; yy++, p+=rowIncr) {
		for (int xx=0; xx<rectWidthInQuads; xx++) {
			_mm_store_ps(reinterpret_cast<float*>(p), r0);
			++p; }}}


void Fill(QFloat4Canvas& dst, const rmlv::vec4 value, const rmlg::irect rect) {
	const int rectHeightInQuads = (rect.height()) / 2;
	const int rectWidthInQuads = (rect.width()) / 2;

	const __m128 red   = _mm_set1_ps(value.x);
	const __m128 green = _mm_set1_ps(value.y);
	const __m128 blue  = _mm_set1_ps(value.z);
	const __m128 alpha = _mm_set1_ps(value.w);

	auto* p = dst.data() + rect.top.y/2*dst.stride2() + rect.left.x/2;
	int rowIncr = dst.stride2() - rectWidthInQuads;

	for (int yy=0; yy<rectHeightInQuads; ++yy, p+=rowIncr) {
		for (int xx=0; xx<rectWidthInQuads; ++xx) {
			_mm_store_ps(reinterpret_cast<float*>(&p->r), red);
			_mm_store_ps(reinterpret_cast<float*>(&p->g), green);
			_mm_store_ps(reinterpret_cast<float*>(&p->b), blue);
			_mm_store_ps(reinterpret_cast<float*>(&p->a), alpha);
			++p; }}}


void Stroke(rglr::TrueColorCanvas& dst, PixelToaster::TrueColorPixel color, const rmlg::irect rect) {
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


void Downsample(QFloat4Canvas& src, FloatingPointCanvas& dst, const rmlg::irect rect) {
	const __m128 one_quarter = _mm_set1_ps(0.25F);
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


void Copy(QFloat4Canvas& src, FloatingPointCanvas& dst, const rmlg::irect rect) {
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


}  // namespace rglr
}  // namespace rqdq
