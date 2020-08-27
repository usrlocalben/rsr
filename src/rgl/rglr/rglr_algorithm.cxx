#include "src/rgl/rglr/rglr_algorithm.hxx"

#include <algorithm>
#include <cstdint>

#include "src/rgl/rglr/rglr_canvas.hxx"
#include "src/rml/rmlg/rmlg_irect.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"

#include "3rdparty/pixeltoaster/PixelToaster.h"

namespace rqdq {
namespace {

auto float_to_word(float x) -> uint16_t {
	return static_cast<uint16_t>(x * 65535.0F); }

}

namespace rglr {

void Fill(QFloatCanvas& dst, const float value, const rmlg::irect rect) {
	const int rectHeightInQuads = rect.height() / 2;
	const int rectWidthInQuads = rect.width() / 2;

	const __m128 r0 = _mm_set1_ps(value);
	auto* p = dst.data() + rect.top.y/2*dst.stride() + rect.left.x/2;
	int rowIncr = dst.stride() - rectWidthInQuads;

	for (int yy=0; yy<rectHeightInQuads; yy++, p+=rowIncr) {
		for (int xx=0; xx<rectWidthInQuads; xx++) {
			_mm_store_ps(reinterpret_cast<float*>(p), r0);
			++p; }}}


// xxx this Fill ignores _rect_ and assumes that the canvas _dst_
// is a tile
void Fill(QFloatCanvas& dst, const float value) { //, const rmlg::irect) {
	const __m128 vvvv   = _mm_set1_ps(value);

	auto dstLeft = dst.data();

	const int regionHeightInQuads = dst.height() / 2;
	const int regionWidthInQuads = dst.width() / 2;

	for (int yy=0; yy<regionHeightInQuads; ++yy, dstLeft+=dst.stride()) {
		auto p = dstLeft;
		for (int xx=0; xx<regionWidthInQuads; ++xx, ++p) {
			QFloatCanvas::Store(vvvv, p); }}}


void Fill(QFloat4Canvas& dst, const rmlv::vec4 value) { //, const rmlg::irect) {
	const __m128 red   = _mm_set1_ps(value.x);
	const __m128 green = _mm_set1_ps(value.y);
	const __m128 blue  = _mm_set1_ps(value.z);
	const __m128 alpha = _mm_set1_ps(value.w);

	auto dstLeft = dst.data();

	const int regionHeightInQuads = dst.height() / 2;
	const int regionWidthInQuads = dst.width() / 2;

	for (int yy=0; yy<regionHeightInQuads; ++yy, dstLeft+=dst.stride()) {
		auto p = dstLeft;
		for (int xx=0; xx<regionWidthInQuads; ++xx, ++p) {
			QFloat4Canvas::Store(red, green, blue, alpha, p); }}}


void Fill(QFloat3Canvas& dst, const rmlv::vec3 value) { //, const rmlg::irect) {
	const __m128 red   = _mm_set1_ps(value.x);
	const __m128 green = _mm_set1_ps(value.y);
	const __m128 blue  = _mm_set1_ps(value.z);

	auto dstLeft = dst.data();

	const int regionHeightInQuads = dst.height() / 2;
	const int regionWidthInQuads = dst.width() / 2;

	for (int yy=0; yy<regionHeightInQuads; ++yy, dstLeft+=dst.stride()) {
		auto p = dstLeft;
		for (int xx=0; xx<regionWidthInQuads; ++xx, ++p) {
			QFloat3Canvas::Store(red, green, blue, p); }}}


void Fill(QShort3Canvas& dst, const rmlv::vec4 value, const rmlg::irect rect) {
	const int rectHeightInQuads = (rect.height()) / 2;
	const int rectWidthInQuads = (rect.width()) / 2;

	QShort3 foo;
	for (int i=0; i<4; i++) {
		foo.r[i] = float_to_word(value.x);
		foo.g[i] = float_to_word(value.y);
		foo.b[i] = float_to_word(value.z); }

	auto* p = dst.data() + rect.top.y/2*dst.stride2() + rect.left.x/2;
	int rowIncr = dst.stride2() - rectWidthInQuads;
	for (int yy=0; yy<rectHeightInQuads; ++yy, p+=rowIncr) {
		for (int xx=0; xx<rectWidthInQuads; ++xx) {
			*p++ = foo; }}}


void Stroke(TrueColorCanvas& dst, PixelToaster::TrueColorPixel color, const rmlg::irect rect) {
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


void Downsample(const QFloat4Canvas& src, FloatingPointCanvas& dst, const rmlg::irect rect) {
	const __m128 oneQuarter = _mm_set1_ps(0.25F);

	const int dstStride = dst.stride();
	const int srcStride = src.stride();

	auto dsty = dst.data() + rect.top.y/2*dstStride + rect.left.x/2;
	auto srcy = src.cdata();
	for (int y=rect.top.y; y<rect.bottom.y; y+=2, dsty+=dstStride, srcy+=srcStride) {
		auto dstpx = dsty;
		auto srcpx = srcy;

		for (int x = rect.left.x; x < rect.right.x; x += 2, ++srcpx, ++dstpx) {
			// _mm_prefetch(reinterpret_cast<const char*>(srcpx + (src.width() >> 1)), _MM_HINT_T0);

			__m128 r0, r1, r2, r3 = _mm_setzero_ps();
			QFloat4Canvas::Load(srcpx, r0, r1, r2);
			_MM_TRANSPOSE4_PS(r0, r1, r2, r3);

			__m128 ax;
			ax = _mm_add_ps(_mm_add_ps(_mm_add_ps(r0, r1), r2), r3);
			ax = _mm_mul_ps(ax, oneQuarter);

			_mm_stream_ps(reinterpret_cast<float*>(&dstpx->v), ax); }}}


void Downsample(const QFloat3Canvas& src, FloatingPointCanvas& dst, const rmlg::irect rect) {
	const __m128 oneQuarter = _mm_set1_ps(0.25F);

	const int dstStride = dst.stride();
	const int srcStride = src.stride();

	auto dsty = dst.data() + rect.top.y/2*dstStride + rect.left.x/2;
	auto srcy = src.cdata();
	for (int y=rect.top.y; y<rect.bottom.y; y+=2, dsty+=dstStride, srcy+=srcStride) {
		auto dstpx = dsty;
		auto srcpx = srcy;

		for (int x = rect.left.x; x < rect.right.x; x += 2, ++srcpx, ++dstpx) {
			// _mm_prefetch(reinterpret_cast<const char*>(srcpx + (src.width() >> 1)), _MM_HINT_T0);

			__m128 r0, r1, r2, r3 = _mm_setzero_ps();
			QFloat3Canvas::Load(srcpx, r0, r1, r2);
			_MM_TRANSPOSE4_PS(r0, r1, r2, r3);

			__m128 ax;
			ax = _mm_add_ps(_mm_add_ps(_mm_add_ps(r0, r1), r2), r3);
			ax = _mm_mul_ps(ax, oneQuarter);

			_mm_stream_ps(reinterpret_cast<float*>(&dstpx->v), ax); }}}


void Downsample(const QShort3Canvas& src, FloatingPointCanvas& dst, const rmlg::irect rect) {
	const __m128 oneQuarter = _mm_set1_ps(0.25F);
	const int dstStride = dst.stride();
	const int srcStride = src.stride2();
	auto dsty = dst.data() + (rect.top.y>>1)*dstStride + (rect.left.x>>1);
	auto srcy = src.cdata();
	for (int y = rect.top.y; y < rect.bottom.y; y += 2, dsty+=dstStride, srcy+=srcStride) {
		auto dstpx = dsty;
		auto srcpx = srcy;

		for (int x = rect.left.x; x < rect.right.x; x += 2, ++srcpx, ++dstpx) {
			__m128 r0, r1, r2, r3 = _mm_setzero_ps();
			QShort3Canvas::Load(srcpx, r0, r1, r2);
			_MM_TRANSPOSE4_PS(r0, r1, r2, r3);

			__m128 ax;
			ax = _mm_add_ps(_mm_add_ps(_mm_add_ps(r0, r1), r2), r3);
			ax = _mm_mul_ps(ax, oneQuarter);

			_mm_stream_ps(reinterpret_cast<float*>(&dstpx->v), ax); }}}


void Copy(const QFloat3Canvas& src, FloatingPointCanvas& dst, const rmlg::irect rect) {
	auto dstRow = &dst.data()[rect.top.y*dst.stride() + rect.left.x];
	auto srcRow = src.cdata();

	for (int y = rect.top.y; y < rect.bottom.y; y += 2) {
		auto dstXr1 = dstRow;
		auto dstXr2 = dstRow + dst.stride();
		auto srcX   = srcRow;

		for (int x = rect.left.x; x < rect.right.x; x += 4) {
			__m128 l0, l1, l2, l3 = _mm_setzero_ps();
			__m128 r0, r1, r2, r3 = _mm_setzero_ps();
			QFloat3Canvas::Load(srcX,   l0, l1, l2);
			QFloat3Canvas::Load(srcX+1, r0, r1, r2);
			_MM_TRANSPOSE4_PS(l0, l1, l2, l3);
			_MM_TRANSPOSE4_PS(r0, r1, r2, r3);

			_mm_stream_ps(reinterpret_cast<float*>(&dstXr1[0]), l0);
			_mm_stream_ps(reinterpret_cast<float*>(&dstXr1[1]), l1);
			_mm_stream_ps(reinterpret_cast<float*>(&dstXr1[2]), r0);
			_mm_stream_ps(reinterpret_cast<float*>(&dstXr1[3]), r1);

			_mm_stream_ps(reinterpret_cast<float*>(&dstXr2[0]), l2);
			_mm_stream_ps(reinterpret_cast<float*>(&dstXr2[1]), l3);
			_mm_stream_ps(reinterpret_cast<float*>(&dstXr2[2]), r2);
			_mm_stream_ps(reinterpret_cast<float*>(&dstXr2[3]), r3);

			dstXr1 += 4;
			dstXr2 += 4;
			srcX += 2; }

		dstRow += dst.stride() * 2;
		srcRow += src.stride(); }}


void Copy(const QFloatCanvas& src, FloatCanvas& dst, const rmlg::irect rect) {
	auto dstLeft = &dst.data()[rect.top.y*dst.stride() + rect.left.x];
	auto srcLeft = src.cdata();

	for (int y = rect.top.y; y < rect.bottom.y; y += 2, dstLeft+=dst.stride()*2, srcLeft+=src.stride()) {
		auto dstXr1 = dstLeft;
		auto dstXr2 = dstLeft + dst.stride();
		auto srcX   = srcLeft;

		for (int x = rect.left.x; x < rect.right.x; x+=4, dstXr1+=4, dstXr2+=4, srcX+=2) {
			__m128 l = _mm_load_ps(reinterpret_cast<float*>(srcX));
			__m128 r = _mm_load_ps(reinterpret_cast<float*>(srcX+1));

			__m128 top = _mm_movelh_ps(l, r);
			__m128 bot = _mm_movehl_ps(r, l);

			_mm_stream_ps(dstXr1, top);
			_mm_stream_ps(dstXr2, bot); }}}


void Copy(const QFloat4Canvas& src, FloatingPointCanvas& dst, const rmlg::irect rect) {
	auto dstRow = &dst.data()[rect.top.y*dst.stride() + rect.left.x];
	auto srcRow = src.cdata();

	for (int y = rect.top.y; y < rect.bottom.y; y += 2) {
		auto dstXr1 = dstRow;
		auto dstXr2 = dstRow + dst.stride();
		auto srcX   = srcRow;

		for (int x = rect.left.x; x < rect.right.x; x += 4) {
			__m128 l0, l1, l2, l3 = _mm_setzero_ps();
			__m128 r0, r1, r2, r3 = _mm_setzero_ps();
			QFloat4Canvas::Load(srcX,   l0, l1, l2);
			QFloat4Canvas::Load(srcX+1, r0, r1, r2);
			_MM_TRANSPOSE4_PS(l0, l1, l2, l3);
			_MM_TRANSPOSE4_PS(r0, r1, r2, r3);

			_mm_stream_ps(reinterpret_cast<float*>(&dstXr1[0]), l0);
			_mm_stream_ps(reinterpret_cast<float*>(&dstXr1[1]), l1);
			_mm_stream_ps(reinterpret_cast<float*>(&dstXr1[2]), r0);
			_mm_stream_ps(reinterpret_cast<float*>(&dstXr1[3]), r1);

			_mm_stream_ps(reinterpret_cast<float*>(&dstXr2[0]), l2);
			_mm_stream_ps(reinterpret_cast<float*>(&dstXr2[1]), l3);
			_mm_stream_ps(reinterpret_cast<float*>(&dstXr2[2]), r2);
			_mm_stream_ps(reinterpret_cast<float*>(&dstXr2[3]), r3);

			dstXr1 += 4;
			dstXr2 += 4;
			srcX += 2; }

		dstRow += dst.stride() * 2;
		srcRow += src.stride(); }}


void Copy(const QShort3Canvas& src, FloatingPointCanvas& dst, const rmlg::irect rect) {
	auto dstRowAddr = &dst.data()[rect.top.y*dst.stride()];
	auto srcRowAddr = src.cdata();

	for (int y = rect.top.y; y < rect.bottom.y; y += 2) {

		auto dstAddrR1 = &dstRowAddr[rect.left.x];
		auto dstAddrR2 = dstAddrR1 + dst.stride();
		auto srcAddr = srcRowAddr;

		for (int x = rect.left.x; x < rect.right.x; x += 4) {
			// BEGIN process 4x2 pixels (2 quads)
			__m128 l0, l1, l2, l3 = _mm_setzero_ps();
			__m128 r0, r1, r2, r3 = _mm_setzero_ps();
			QShort3Canvas::Load(srcAddr+0, l0, l1, l2);
			QShort3Canvas::Load(srcAddr+1, r0, r1, r2);
			_MM_TRANSPOSE4_PS(l0, l1, l2, l3);
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
		srcRowAddr += src.stride2(); }}


void Copy(const QFloat3Canvas& src, QFloat4Canvas& dst, const rmlg::irect rect) {
	auto dstRowAddr = &dst.data()[rect.top.y/2*dst.stride()];
	auto srcRowAddr = src.cdata();

	for (int y = rect.top.y; y < rect.bottom.y; y += 2) {

		auto dstAddr = &dstRowAddr[rect.left.x/2];
		auto srcAddr = srcRowAddr;

		for (int x = rect.left.x; x < rect.right.x; x += 2) {
			__m128 l0, l1, l2, l3;
			QFloat3Canvas::Load(srcAddr, l0, l1, l2);
			l3 = _mm_set1_ps(1.0F);

			_mm_stream_ps(reinterpret_cast<float*>(&(dstAddr->x.v)), l0);
			_mm_stream_ps(reinterpret_cast<float*>(&(dstAddr->y.v)), l1);
			_mm_stream_ps(reinterpret_cast<float*>(&(dstAddr->z.v)), l2);
			_mm_stream_ps(reinterpret_cast<float*>(&(dstAddr->w.v)), l3);

			dstAddr++;
			srcAddr++; }

		dstRowAddr += dst.stride();
		srcRowAddr += src.stride(); }}


void Copy(const QFloat4Canvas& src, QFloat4Canvas& dst, const rmlg::irect rect) {
	auto dstRowAddr = &dst.data()[rect.top.y/2*dst.stride()];
	auto srcRowAddr = src.cdata();

	for (int y = rect.top.y; y < rect.bottom.y; y += 2) {

		auto dstAddr = &dstRowAddr[rect.left.x/2];
		auto srcAddr = srcRowAddr;

		for (int x = rect.left.x; x < rect.right.x; x += 2) {
			__m128 l0, l1, l2, l3;
			QFloat4Canvas::Load(srcAddr, l0, l1, l2, l3);

			_mm_stream_ps(reinterpret_cast<float*>(&(dstAddr->x.v)), l0);
			_mm_stream_ps(reinterpret_cast<float*>(&(dstAddr->y.v)), l1);
			_mm_stream_ps(reinterpret_cast<float*>(&(dstAddr->z.v)), l2);
			_mm_stream_ps(reinterpret_cast<float*>(&(dstAddr->w.v)), l3);

			dstAddr++;
			srcAddr++; }

		dstRowAddr += dst.stride();
		srcRowAddr += src.stride(); }}


}  // namespace rglr
}  // namespace rqdq
