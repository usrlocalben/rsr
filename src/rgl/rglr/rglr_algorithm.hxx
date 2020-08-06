#pragma once
#include "src/rgl/rglr/rglr_canvas.hxx"
#include "src/rml/rmlg/rmlg_irect.hxx"
#include "src/rml/rmlv/rmlv_mvec4.hxx"
#include "src/rml/rmlv/rmlv_soa.hxx"

#include "3rdparty/pixeltoaster/PixelToaster.h"

namespace rqdq {
namespace rglr {

void FillAll(QFloatCanvas& dst, float value);
void FillAll(QFloat3Canvas& dst, rmlv::vec3 value);
void FillAll(QFloat4Canvas& dst, rmlv::vec4 value);

void Fill(QFloatCanvas& dst, float value, rmlg::irect rect);
void Fill(QFloat4Canvas& dst, rmlv::vec4 value, rmlg::irect rect);
void Fill(QShort3Canvas& dst, rmlv::vec4 value, rmlg::irect rect);
void Stroke(TrueColorCanvas& dst, PixelToaster::TrueColorPixel color, rmlg::irect rect);
void Downsample(const QFloat3Canvas& src, FloatingPointCanvas& dst, rmlg::irect rect);
void Downsample(const QFloat4Canvas& src, FloatingPointCanvas& dst, rmlg::irect rect);
void Downsample(const QShort3Canvas& src, FloatingPointCanvas& dst, rmlg::irect rect);
void Copy(const QFloatCanvas& src, FloatCanvas& dst, const rmlg::irect rect);
void Copy(const QFloat3Canvas& src, FloatingPointCanvas& dst, rmlg::irect rect);
void Copy(const QFloat4Canvas& src, FloatingPointCanvas& dst, rmlg::irect rect);
void Copy(const QShort3Canvas& src, FloatingPointCanvas& dst, rmlg::irect rect);
void Copy(const QFloat3Canvas& src, QFloat4Canvas& dst, rmlg::irect rect);
void Copy(const QFloat4Canvas& src, QFloat4Canvas& dst, rmlg::irect rect);


template <class SHADER, class CONVERTER>
void FilterTile(const QFloat3Canvas& src1, TrueColorCanvas& dst, const rmlg::irect rect) {
	using rmlv::qfloat2, rmlv::qfloat3, rmlv::qfloat4;
	using rmlv::mvec4f, rmlv::mvec4i;

	const float iw = 1.0F / dst.width();
	const float ih = 1.0F / dst.height();
	const mvec4f fcdx{  iw*2 };
	const mvec4f fcdy{ -ih*2 };
	mvec4f fcxLeft{(               rect.left.x + 0.5F) / float(dst.width()) };
	fcxLeft += mvec4f{0,iw,0,iw};
	mvec4f fcy    {(dst.height() - rect.top.y  - 0.5F) / float(dst.height())};
	fcy -= mvec4f{0,0,ih,ih};

	auto* src1AddrLeft = src1.cdata();
	for (int y=rect.top.y; y<rect.bottom.y; y+=2, src1AddrLeft += src1.stride(), fcy+=fcdy) {
		auto destRow1Addr = &dst.data()[y * dst.stride() + rect.left.x];
		auto destRow2Addr = destRow1Addr + dst.stride();

		auto* src1Addr = src1AddrLeft;

		mvec4f fcx = fcxLeft;
		for (int x = rect.left.x; x < rect.right.x; x += 4, destRow1Addr+=4, destRow2Addr+=4) {
			mvec4i packed[2];
			for (int sub = 0; sub < 2; ++sub, ++src1Addr, fcx+=fcdx) {

				// load source1 color
				qfloat3 sc;
				QFloat3Canvas::Load(src1Addr, sc.x.v, sc.y.v, sc.z.v);

				// shade & convert
				qfloat3 fragColor = SHADER::ShadeCanvas({ fcx, fcy }, sc);
				packed[sub] = CONVERTER::to_tc(fragColor); }

			_mm_stream_si128(reinterpret_cast<__m128i*>(destRow1Addr), _mm_unpacklo_epi64(packed[0].v, packed[1].v));
			_mm_stream_si128(reinterpret_cast<__m128i*>(destRow2Addr), _mm_unpackhi_epi64(packed[0].v, packed[1].v)); }}}


template <class SHADER, class CONVERTER>
void FilterTile(const QFloat4Canvas& src1, TrueColorCanvas& dst, const rmlg::irect rect) {
	using rmlv::qfloat2, rmlv::qfloat3, rmlv::qfloat4;
	using rmlv::mvec4f, rmlv::mvec4i;

	const float iw = 1.0F / dst.width();
	const float ih = 1.0F / dst.height();
	const mvec4f fcdx{  iw*2 };
	const mvec4f fcdy{ -ih*2 };
	mvec4f fcxLeft{(               rect.left.x + 0.5F) / float(dst.width()) };
	fcxLeft += mvec4f{0,iw,0,iw};
	mvec4f fcy    {(dst.height() - rect.top.y  - 0.5F) / float(dst.height())};
	fcy -= mvec4f{0,0,ih,ih};

	auto* src1AddrLeft = src1.cdata();
	for (int y=rect.top.y; y<rect.bottom.y; y+=2, src1AddrLeft += src1.stride(), fcy+=fcdy) {
		auto destRow1Addr = &dst.data()[y * dst.stride() + rect.left.x];
		auto destRow2Addr = destRow1Addr + dst.stride();

		auto* src1Addr = src1AddrLeft;

		mvec4f fcx = fcxLeft;
		for (int x = rect.left.x; x < rect.right.x; x += 4, destRow1Addr+=4, destRow2Addr+=4) {
			mvec4i packed[2];
			for (int sub = 0; sub < 2; ++sub, ++src1Addr, fcx+=fcdx) {

				// load source1 color
				qfloat3 sc;
				QFloat4Canvas::Load(src1Addr, sc.x.v, sc.y.v, sc.z.v);

				// shade & convert
				qfloat3 fragColor = SHADER::ShadeCanvas({ fcx, fcy }, sc);
				packed[sub] = CONVERTER::to_tc(fragColor); }

			_mm_stream_si128(reinterpret_cast<__m128i*>(destRow1Addr), _mm_unpacklo_epi64(packed[0].v, packed[1].v));
			_mm_stream_si128(reinterpret_cast<__m128i*>(destRow2Addr), _mm_unpackhi_epi64(packed[0].v, packed[1].v)); }}}


template <class SHADER, class CONVERTER>
void Filter(const QFloat4Canvas& src1, const FloatingPointCanvas& src2, TrueColorCanvas& dst, const rmlg::irect rect) {
	using rmlv::qfloat2, rmlv::qfloat3, rmlv::qfloat4;
	using rmlv::mvec4f, rmlv::mvec4i;

	const float iw = 1.0F / dst.width();
	const float ih = 1.0F / dst.height();
	const mvec4f fcdx{  iw*2 };
	const mvec4f fcdy{ -ih*2 };
	mvec4f fcxLeft{(               rect.left.x + 0.5F) / float(dst.width()) };
	fcxLeft += mvec4f{0,iw,0,iw};
	mvec4f fcy    {(dst.height() - rect.top.y  - 0.5F) / float(dst.height())};
	fcy -= mvec4f{0,0,ih,ih};

	for (int y=rect.top.y, oy=0; y<rect.bottom.y; y+=2, oy+=2, fcy+=fcdy) {
		auto destRow1Addr = &dst.data()[y * dst.stride() + rect.left.x];
		auto destRow2Addr = destRow1Addr + dst.stride();

		auto* source1Addr = &src1.cdata()[y/2*src1.stride() + rect.left.x/2];
		auto* source2Addr = &src2.cdata()[y/2*src2.stride() + rect.left.x/2];

		mvec4f fcx = fcxLeft;
		for (int x = rect.left.x; x < rect.right.x; x += 4, destRow1Addr+=4, destRow2Addr+=4) {
			mvec4i packed[2];
			for (int sub = 0; sub < 2; sub++, source1Addr++, source2Addr++, fcx+=fcdx) {

				// load source1 color
				qfloat3 sc1;
				QFloat4Canvas::Load(source1Addr, sc1.x.v, sc1.y.v, sc1.z.v);

				qfloat3 sc2{ source2Addr->r, source2Addr->g, source2Addr->b };

				// shade & convert
				qfloat3 fragColor = SHADER::ShadeCanvas({ fcx, fcy }, sc1, sc2);
				packed[sub] = CONVERTER::to_tc(fragColor); }

			_mm_stream_si128(reinterpret_cast<__m128i*>(destRow1Addr), _mm_unpacklo_epi64(packed[0].v, packed[1].v));
			_mm_stream_si128(reinterpret_cast<__m128i*>(destRow2Addr), _mm_unpackhi_epi64(packed[0].v, packed[1].v)); }}}


/*
template <class SHADER, class CONVERTER>
void FilterDown(const QFloat4Canvas& src1, TrueColorCanvas& dst, const rmlg::irect rect) {
	using rmlv::qfloat2, rmlv::qfloat3, rmlv::qfloat4;
	using rmlv::mvec4f, rmlv::mvec4i;

	const float iw = 1.0F / dst.width();
	const float ih = 1.0F / dst.height();
	const mvec4f fcdx{  iw*2 };
	const mvec4f fcdy{ -ih*2 };
	mvec4f fcxLeft{(               rect.left.x + 0.5F) / float(dst.width()) };
	fcxLeft += mvec4f{0,iw,0,iw};
	mvec4f fcy    {(dst.height() - rect.top.y  - 0.5F) / float(dst.height())};
	fcy -= mvec4f{0,0,ih,ih};

	mvec4f[16*8] buf;
	
	for (int y=rect.top.y, oy=0; y<rect.bottom.y; y+=2, oy+=2, fcy+=fcdy) {
		auto* src1X = &src1.cdata()[(oy/2) * (src1.width()/2)];

		mvec4f fcx = fcxLeft;
		int bi=0;

		for (int x=rect.left.x; x < rect.right.x; x += 2, src1X++, fcx+=fcdx, bi++) {
			// load source1 color
			qfloat3 sc;
			QFloat4Canvas::Load(src1X, sc.x.v, sc.y.v, sc.z.v);

			// shade & convert
			qfloat3 fragColor = SHADER::ShadeCanvas({ fcx, fcy }, sc);

			// average quad, store in buffer
			__m128 r0 = fragColor.x.v;
			__m128 r1 = fragColor.y.v;
			__m128 r2 = fragColor.z.v;
			__m128 r3 = _mm_setzero_ps();
			_MM_TRANSPOSE4(r0, r1, r2, r3);
			__m128 ax;
			ax = _mm_add_ps(_mm_add_ps(_mm_add_ps(r0, r1), r2), r3);
			ax = _mm_mul_ps(ax, _mm_set1_ps(0.25F));
			buf[bi] = ax; }

		auto destX = &dst.data()[y/2*dst.stride() + rect.left.x/2];
		for (int i=0; i<bi; bi+=4) {
			__m128 r0 = buf[i];
			__m128 r1 = buf[i+1];
			__m128 r2 = buf[i+2];
			__m128 r3 = buf[i+3];
			_MM_TRANSPOSE4(r0, r1, r2, r3);
			mvec4i packed = CONVERTER::to_tc({ r0, r1, r2, r3 });
			_mm_stream_si128(reinterpret_cast<__m128i*>(destX[bi]), packed.v); }}}
*/


}  // namespace rglr
}  // namespace rqdq
