#pragma once
#include "src/rcl/rcls/rcls_aligned_containers.hxx"
#include "src/rml/rmlg/rmlg_irect.hxx"
#include "src/rml/rmlv/rmlv_soa.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"

#include "3rdparty/pixeltoaster/PixelToaster.h"
#include <immintrin.h>
#include <emmintrin.h>

namespace rqdq {
namespace rglr {

inline PixelToaster::TrueColorPixel to_tc_from_fpp(PixelToaster::FloatingPointPixel& fpp) {
	return {
		PixelToaster::integer8(fpp.r * 255.0),
		PixelToaster::integer8(fpp.g * 255.0),
		PixelToaster::integer8(fpp.b * 255.0)
		}; }


struct QFloatCanvas {
	QFloatCanvas()  {
		buffer.reserve(1);
		_ptr = buffer.data(); }

	QFloatCanvas(const int width, const int height)
		:_width(width), _height(height), _stride2(width / 2) {
		buffer.reserve(width * height / 4);
		_ptr = buffer.data(); }

	auto data() { return _ptr; }
	auto cdata() const { return _ptr; }
	auto width() const { return _width; }
	auto height() const { return _height; }
	auto stride2() const { return _stride2; }
	rmlg::irect rect() const {
		return rmlg::irect{ rmlv::ivec2{0,0}, rmlv::ivec2{_width, _height} }; }
	float aspect() const {
		if (_width == 0 || _height == 0) {
			return 1.0F; }
		return float(_width) / float(_height); }

	void resize(const int width, const int height) {
		buffer.reserve(width * height / 4);
		_ptr = buffer.data();
		_width = width;
		_height = height;
		_stride2 = width / 2; }

private:
	rcls::vector<rmlv::qfloat> buffer;
	rmlv::qfloat* _ptr;
	int _width{2};
	int _height{2};
	int _stride2{1};
	};


struct QFloat4Canvas {
	QFloat4Canvas() = default;

	QFloat4Canvas(const int width, const int height)
		:_width(width), _height(height), _stride2(width / 2) {
		buffer.reserve(width * height / 4);
		_ptr = buffer.data();
	}
	auto data() { return _ptr; }
	auto cdata() const { return _ptr; }

	auto width() const { return _width; }
	auto height() const { return _height; }
	auto stride2() const { return _stride2; }
	rmlg::irect rect() const {
		return rmlg::irect{ rmlv::ivec2{0,0}, rmlv::ivec2{_width, _height} }; }
	float aspect() const {
		if (_width == 0 || _height == 0) {
			return 1.0F; }
		return float(_width) / float(_height); }

	void resize(const int width, const int height) {
		buffer.reserve(width * height / 4);
		_ptr = buffer.data();
		_width = width;
		_height = height;
		_stride2 = width / 2; }

	rmlv::vec4 get_pixel(int x, int y) {
		const auto* data = reinterpret_cast<const float*>(_ptr);
		const int channels = 4;
		const int pixelsPerQuad = 4;
		const int elemsPerQuad = channels * pixelsPerQuad;
		const int widthInQuads = _width / 2;

		const int Yquad = y / 2;
		const int Xquad = x / 2;

		const int quadAddr = (Yquad * widthInQuads + Xquad) * elemsPerQuad;

		//int cellidx = y / 2 * _stride2 + x / 2;
		int sy = y % 2;
		int sx = x % 2;
		float r = data[ quadAddr +  0 + sy*2 + sx ];
		float g = data[ quadAddr +  4 + sy*2 + sx ];
		float b = data[ quadAddr +  8 + sy*2 + sx ];
		float a = data[ quadAddr + 12 + sy*2 + sx ];
		return rmlv::vec4{ r, g, b, a }; }

private:
	rcls::vector<rmlv::qfloat4> buffer;
	rmlv::qfloat4* _ptr{nullptr};
	int _width{0};
	int _height{0};
	int _stride2{0};
	};

//#define FOO_ALPHA_ALIGN

struct Int16QPixel {
	uint16_t r[4];
	uint16_t g[4];
	uint16_t b[4];
#ifdef FOO_ALPHA_ALIGN
	uint16_t a[4];
#endif

	static inline void Load(const Int16QPixel* const src, __m128& rrrr, __m128& gggg, __m128& bbbb) {
		const __m128 scale = _mm_set1_ps(1.0F / 255.0F);
#ifndef FOO_ALPHA_ALIGN
		__m128i irig = _mm_loadu_si128((__m128i*)&src->r);
		__m128i ibia = _mm_loadu_si128((__m128i*)&src->b);
#else
		__m128i irig = _mm_load_si128((__m128i*)&src->r);
		__m128i ibia = _mm_load_si128((__m128i*)&src->b);
#endif
		__m128i zero = _mm_setzero_si128();
		__m128i ir = _mm_unpacklo_epi16(irig, zero);
		__m128i ig = _mm_unpackhi_epi16(irig, zero);
		__m128i ib = _mm_unpacklo_epi16(ibia, zero);
		rrrr = _mm_mul_ps(_mm_cvtepi32_ps(ir), scale);
		gggg = _mm_mul_ps(_mm_cvtepi32_ps(ig), scale);
		bbbb = _mm_mul_ps(_mm_cvtepi32_ps(ib), scale); }

	static inline void Store(__m128 rrrr, __m128 gggg, __m128 bbbb, Int16QPixel* dst) {
		const __m128 scale = _mm_set1_ps(255.0F);
		__m128i ir = _mm_cvtps_epi32(_mm_mul_ps(rrrr, scale));
		__m128i ig = _mm_cvtps_epi32(_mm_mul_ps(gggg, scale));
		__m128i ib = _mm_cvtps_epi32(_mm_mul_ps(bbbb, scale));
		__m128i irig = _mm_packs_epi32(ir, ig);
		__m128i ibxx = _mm_packs_epi32(ib, ib);
#ifndef FOO_ALPHA_ALIGN
		_mm_storeu_si128((__m128i*)&dst->r, irig);
		_mm_storeu_si64((__m128i*)&dst->b, ibxx);
#else
		_mm_store_si128((__m128i*)&dst->r, irig);
		_mm_store_si128((__m128i*)&dst->b, ibxx);
#endif
	}};


struct Int16Canvas {
	Int16Canvas() = default;

	Int16Canvas(const int width, const int height) :
		width_(width),
		height_(height),
		stride2_(width / 2) {
		buffer_.reserve(width * height / 4);
		ptr_ = buffer_.data(); }

	auto data() { return ptr_; }
	auto cdata() const { return ptr_; }

	auto width() const { return width_; }
	auto height() const { return height_; }
	auto stride2() const { return stride2_; }
	rmlg::irect rect() const {
		return rmlg::irect{ rmlv::ivec2{0,0}, rmlv::ivec2{width_, height_} }; }
	float aspect() const {
		if (width_ == 0 || height_ == 0) {
			return 1.0F; }
		return float(width_) / float(height_); }

	void resize(const int width, const int height) {
		buffer_.reserve(width * height / 4);
		ptr_ = buffer_.data();
		width_ = width;
		height_ = height;
		stride2_ = width / 2; }

	/*rmlv::vec4 get_pixel(int x, int y) {
		const auto* data = reinterpret_cast<const float*>(ptr_);
		const int channels = 4;
		const int pixelsPerQuad = 4;
		const int elemsPerQuad = channels * pixelsPerQuad;
		const int widthInQuads = width_ / 2;

		const int Yquad = y / 2;
		const int Xquad = x / 2;

		const int quadAddr = (Yquad * widthInQuads + Xquad) * elemsPerQuad;

		//int cellidx = y / 2 * stride2_ + x / 2;
		int sy = y % 2;
		int sx = x % 2;
		float r = data[ quadAddr +  0 + sy*2 + sx ];
		float g = data[ quadAddr +  4 + sy*2 + sx ];
		float b = data[ quadAddr +  8 + sy*2 + sx ];
		float a = data[ quadAddr + 12 + sy*2 + sx ];
		return rmlv::vec4{ r, g, b, a }; }*/

private:
	rcls::vector<Int16QPixel> buffer_;
	Int16QPixel* ptr_{nullptr};
	int width_{0};
	int height_{0};
	int stride2_{0}; };


struct TrueColorCanvas {
	TrueColorCanvas(
		PixelToaster::TrueColorPixel * ptr,
	    const int width,
	    const int height,
	    const int stride = 0
	) :
		_ptr(ptr),
		_width(width),
		_height(height),
		_stride(stride == 0 ? width : stride),
		_aspect(float(width) / float(height)) {}

	auto data() { return _ptr; }
	auto cdata() const { return _ptr; }
	auto width() const { return _width; }
	auto height() const { return _height; }
	auto stride() const { return _stride; }
	auto aspect() const { return _aspect; }
	rmlg::irect rect() const {
		return rmlg::irect{ rmlv::ivec2{0,0}, rmlv::ivec2{_width, _height} }; }
private:
	PixelToaster::TrueColorPixel * _ptr;
	const int _width;
	const int _height;
	const int _stride;
	const float _aspect;
	};


struct FloatingPointCanvas {
	FloatingPointCanvas(
		PixelToaster::FloatingPointPixel* ptr,
		const int width,
		const int height,
		const int stride = 0
	) :_ptr(ptr),
		_width(width),
		_height(height),
		_stride(stride == 0 ? width : stride),
		_aspect(float(width) / float(height)) {}

	FloatingPointCanvas() = default;

	auto data() { return _ptr; }
	auto cdata() const { return _ptr; }
	auto width() const { return _width; }
	auto height() const { return _height; }
	auto stride() const { return _stride; }
	auto aspect() const { return _aspect; }
	rmlg::irect rect() const {
		return rmlg::irect{ rmlv::ivec2{0,0}, rmlv::ivec2{_width, _height} }; }
private:
	PixelToaster::FloatingPointPixel * _ptr = nullptr;
	int _width = 0;
	int _height = 0;
	int _stride = 0;
	float _aspect = 1.0F; };


}  // namespace rglr
}  // namespace rqdq
