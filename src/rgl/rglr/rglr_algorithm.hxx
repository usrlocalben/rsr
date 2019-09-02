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
void Stroke(TrueColorCanvas& dst, PixelToaster::TrueColorPixel color, rmlg::irect rect);
void Downsample(QFloat4Canvas& src, FloatingPointCanvas& dst, rmlg::irect rect);
void Copy(QFloat4Canvas& src, FloatingPointCanvas& dst, rmlg::irect rect);

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

				// shade & convert
				qfloat3 fragColor = SHADER::ShadeCanvas(q, source1Color);
				packed[sub] = CONVERTER::to_tc(fragColor); }

			_mm_stream_si128(reinterpret_cast<__m128i*>(destRow1Addr), _mm_unpacklo_epi64(packed[0].v, packed[1].v));
			_mm_stream_si128(reinterpret_cast<__m128i*>(destRow2Addr), _mm_unpackhi_epi64(packed[0].v, packed[1].v)); }}}


}  // namespace rglr
}  // namespace rqdq
