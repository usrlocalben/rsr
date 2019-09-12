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

	const float dw = 1.0F / dst.width();
	const float dh = 1.0F / dst.height();

	const qfloat2 dqx{ dw*2, 0 };
	const qfloat2 dqy{ 0, -dh*2 };
	qfloat2 q_row{
		mvec4f{rect.left.x                     / float(dst.width()) } + mvec4f{0,dw,0,dw},
		mvec4f{(dst.height() - 1 - rect.top.y) / float(dst.height())} - mvec4f{0,0,dh,dh}
		};

	const int srcStride2 = src1.stride2();
	const int dstStride = dst.stride();

	auto* src1y = &src1.cdata()[rect.top.y/2 * srcStride2 + rect.left.x/2];
	auto* dstRow = &dst.data()[rect.top.y*dstStride];

	for (int y = rect.top.y; y < rect.bottom.y; y += 2, q_row+=dqy, src1y+=srcStride2, dstRow+=dstStride*2) {
		auto* src1x = src1y;
		qfloat2 q{ q_row };
		for (int x = rect.left.x; x < rect.right.x; x+=4) {
			mvec4i packed0, packed1;
			qfloat3 sc;

			QFloat4Canvas::Load(src1x++, sc.x.v, sc.y.v, sc.z.v);
			sc = SHADER::ShadeCanvas(q, sc);
			packed0 = CONVERTER::to_tc(sc);
			q += dqx;

			QFloat4Canvas::Load(src1x++, sc.x.v, sc.y.v, sc.z.v);
			sc = SHADER::ShadeCanvas(q, sc);
			packed1 = CONVERTER::to_tc(sc);
			q += dqx;

			_mm_stream_si128(reinterpret_cast<__m128i*>(dstRow+x),
			                 _mm_unpacklo_epi64(packed0.v, packed1.v));
			_mm_stream_si128(reinterpret_cast<__m128i*>(dstRow+x+dstStride),
			                 _mm_unpackhi_epi64(packed0.v, packed1.v)); }}}


template <class SHADER, class CONVERTER>
void Filter(const QShort3Canvas& src1, TrueColorCanvas& dst, const rmlg::irect rect) {
	using rmlv::qfloat2, rmlv::qfloat3, rmlv::qfloat4;
	using rmlv::mvec4f, rmlv::mvec4i;

	const float dw = 1.0F / dst.width();
	const float dh = 1.0F / dst.height();

	const qfloat2 dqx{ dw*2, 0 };
	const qfloat2 dqy{ 0, -dh*2 };
	qfloat2 q_row{
		mvec4f{rect.left.x                     / float(dst.width()) } + mvec4f{0,dw,0,dw},
		mvec4f{(dst.height() - 1 - rect.top.y) / float(dst.height())} - mvec4f{0,0,dh,dh}
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
				qfloat3 source1Color;
				QShort3Canvas::Load(source1Addr, source1Color.x.v, source1Color.y.v, source1Color.z.v);

				// shade & convert
				qfloat3 fragColor = SHADER::ShadeCanvas(q, source1Color);
				packed[sub] = CONVERTER::to_tc(fragColor); }

			_mm_stream_si128(reinterpret_cast<__m128i*>(destRow1Addr), _mm_unpacklo_epi64(packed[0].v, packed[1].v));
			_mm_stream_si128(reinterpret_cast<__m128i*>(destRow2Addr), _mm_unpackhi_epi64(packed[0].v, packed[1].v)); }}}


}  // namespace rglr
}  // namespace rqdq
