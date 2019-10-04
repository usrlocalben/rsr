#pragma once
#include "src/rgl/rglr/rglr_canvas.hxx"
#include "src/rml/rmlg/rmlg_irect.hxx"
#include "src/rml/rmlv/rmlv_mvec4.hxx"
#include "src/rml/rmlv/rmlv_soa.hxx"

#include "3rdparty/pixeltoaster/PixelToaster.h"

namespace rqdq {
namespace rglr {

void Fill(QFloatCanvas& dst, float value, rmlg::irect rect);
void Fill(QFloat4Canvas& dst, rmlv::vec4 value, rmlg::irect rect);
void Fill(QShort3Canvas& dst, rmlv::vec4 value, rmlg::irect rect);
void Stroke(TrueColorCanvas& dst, PixelToaster::TrueColorPixel color, rmlg::irect rect);
void Downsample(const QFloat4Canvas& src, FloatingPointCanvas& dst, rmlg::irect rect);
void Downsample(const QShort3Canvas& src, FloatingPointCanvas& dst, rmlg::irect rect);
void Copy(const QFloat4Canvas& src, FloatingPointCanvas& dst, rmlg::irect rect);
void Copy(const QShort3Canvas& src, FloatingPointCanvas& dst, rmlg::irect rect);

template <class SHADER, class CONVERTER>
void Filter(const QFloat4Canvas& src1, TrueColorCanvas& dst, const rmlg::irect rect) {
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

	int oy=0;
	for (int y = rect.top.y; y < rect.bottom.y; y += 2, oy+=2, fcy+=fcdy) {
		auto destRow1Addr = &dst.data()[y * dst.stride() + rect.left.x];
		auto destRow2Addr = destRow1Addr + dst.stride();

		auto* source1Addr = &src1.cdata()[(oy >> 1) * (src1.width() >> 1)];

		mvec4f fcx = fcxLeft;
		for (int x = rect.left.x; x < rect.right.x; x += 4, destRow1Addr+=4, destRow2Addr+=4) {
			mvec4i packed[2];
			for (int sub = 0; sub < 2; sub++, source1Addr++, fcx+=fcdx) {

				// load source1 color
				qfloat3 sc;
				QFloat4Canvas::Load(source1Addr, sc.x.v, sc.y.v, sc.z.v);

				// shade & convert
				qfloat3 fragColor = SHADER::ShadeCanvas({ fcx, fcy }, sc);
				packed[sub] = CONVERTER::to_tc(fragColor); }

			_mm_stream_si128(reinterpret_cast<__m128i*>(destRow1Addr), _mm_unpacklo_epi64(packed[0].v, packed[1].v));
			_mm_stream_si128(reinterpret_cast<__m128i*>(destRow2Addr), _mm_unpackhi_epi64(packed[0].v, packed[1].v)); }}}


template <class SHADER, class CONVERTER>
void Filter(const QShort3Canvas& src1, TrueColorCanvas& dst, const rmlg::irect rect) {
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

	int oy=0;
	for (int y = rect.top.y; y < rect.bottom.y; y += 2, oy+=2, fcy+=fcdy) {
		auto destRow1Addr = &dst.data()[y * dst.stride() + rect.left.x];
		auto destRow2Addr = destRow1Addr + dst.stride();

		auto* source1Addr = &src1.cdata()[(y >> 1) * (dst.width() >> 1)];

		mvec4f fcx = fcxLeft;
		for (int x = rect.left.x; x < rect.right.x; x += 4, destRow1Addr+=4, destRow2Addr+=4) {
			mvec4i packed[2];
			for (int sub = 0; sub < 2; sub++, source1Addr++, fcx+=fcdx) {

				// load source1 color
				qfloat3 source1Color;
				QShort3Canvas::Load(source1Addr, source1Color.x.v, source1Color.y.v, source1Color.z.v);

				// shade & convert
				qfloat3 fragColor = SHADER::ShadeCanvas({ fcx, fcy }, source1Color);
				packed[sub] = CONVERTER::to_tc(fragColor); }

			_mm_stream_si128(reinterpret_cast<__m128i*>(destRow1Addr), _mm_unpacklo_epi64(packed[0].v, packed[1].v));
			_mm_stream_si128(reinterpret_cast<__m128i*>(destRow2Addr), _mm_unpackhi_epi64(packed[0].v, packed[1].v)); }}}


}  // namespace rglr
}  // namespace rqdq
