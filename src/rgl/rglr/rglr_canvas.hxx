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


class FloatCanvas {
	int width_{1};
	int height_{1};
	int stride_{1};
	rcls::vector<float> buffer_;
	float* ptr_;

public:
	/*QFloatCanvas()  {
		buffer_.reserve(1);
		ptr_ = buffer_.data(); }

	QFloatCanvas(int w, int h) :
		width_(w),
		height_(h),
		stride_(w/2) {
		buffer_.reserve(w*h / 4);
		ptr_ = buffer_.data(); }

	QFloatCanvas(int w, int h, rmlv::qfloat* buf) :
		width_(w),
		height_(h),
		stride_(w/2),
		ptr_(buf) {}*/

	FloatCanvas(int w, int h, void* buf, int stride) :
		width_(w),
		height_(h),
		stride_(stride),
		ptr_(static_cast<float*>(buf)) {}

	auto data() { return ptr_; }
	auto cdata() const { return ptr_; }
	auto width() const { return width_; }
	auto height() const { return height_; }
	auto stride() const { return stride_; }
	auto rect() const -> rmlg::irect {
		return rmlg::irect{ rmlv::ivec2{0,0}, rmlv::ivec2{width_, height_} }; }
	float aspect() const {
		if (width_ == 0 || height_ == 0) {
			return 1.0F; }
		return float(width_) / float(height_); }

	void resize(int w, int h) {
		buffer_.reserve(w*h/4);
		ptr_ = buffer_.data();
		width_ = w;
		height_ = h;
		stride_ = w / 2; }

	void resize(rmlv::ivec2 dim) {
		resize(dim.x, dim.y); } };


class QFloatCanvas {
	int width_{2};
	int height_{2};
	int stride_{1};
	rcls::vector<rmlv::qfloat> buffer_;
	rmlv::qfloat* ptr_;

public:
	QFloatCanvas()  {
		buffer_.reserve(1);
		ptr_ = buffer_.data(); }

	QFloatCanvas(int w, int h) :
		width_(w),
		height_(h),
		stride_(w/2) {
		buffer_.reserve(w*h / 4);
		ptr_ = buffer_.data(); }

	QFloatCanvas(int w, int h, rmlv::qfloat* buf) :
		width_(w),
		height_(h),
		stride_(w/2),
		ptr_(buf) {}

	QFloatCanvas(int w, int h, rmlv::qfloat* buf, int stride) :
		width_(w),
		height_(h),
		stride_(stride),
		ptr_(buf) {}

	auto data() { return ptr_; }
	auto cdata() const { return ptr_; }
	auto width() const { return width_; }
	auto height() const { return height_; }
	auto stride() const { return stride_; }
	auto rect() const -> rmlg::irect {
		return rmlg::irect{ rmlv::ivec2{0,0}, rmlv::ivec2{width_, height_} }; }
	float aspect() const {
		if (width_ == 0 || height_ == 0) {
			return 1.0F; }
		return float(width_) / float(height_); }

	void resize(int w, int h) {
		buffer_.reserve(w*h/4);
		ptr_ = buffer_.data();
		width_ = w;
		height_ = h;
		stride_ = w / 2; }

	static inline void Store(__m128 vvvv, rmlv::qfloat* const dst) {
		_mm_store_ps(reinterpret_cast<float*>(dst), vvvv); }

	void resize(rmlv::ivec2 dim) {
		resize(dim.x, dim.y); } };


class QFloat3Canvas {
	int width_{0};
	int height_{0};
	int stride_{0};
	rcls::vector<rmlv::qfloat3> buffer_;
	rmlv::qfloat3* ptr_{nullptr};

public:
	QFloat3Canvas() = default;

	QFloat3Canvas(int w, int h) :
		width_(w),
		height_(h),
		stride_(w / 2) {
		buffer_.reserve(w*h/4);
		ptr_ = buffer_.data(); }

	QFloat3Canvas(int w, int h, rmlv::qfloat3* buf) :
		width_(w),
		height_(h),
		stride_(w / 2),
		ptr_(buf) {}

	QFloat3Canvas(int w, int h, rmlv::qfloat3* buf, int stride) :
		width_(w),
		height_(h),
		stride_(stride),
		ptr_(buf) {}

	auto data() { return ptr_; }
	auto cdata() const { return ptr_; }

	auto width() const { return width_; }
	auto height() const { return height_; }
	auto stride() const { return stride_; }
	rmlg::irect rect() const {
		return rmlg::irect{ rmlv::ivec2{0,0}, rmlv::ivec2{width_, height_} }; }
	float aspect() const {
		if (width_ == 0 || height_ == 0) {
			return 1.0F; }
		return float(width_) / float(height_); }

	void resize(int w, int h) {
		buffer_.reserve(w*h/4);
		ptr_ = buffer_.data();
		width_ = w;
		height_ = h;
		stride_ = w/2; }

	void resize(rmlv::ivec2 dim) {
		resize(dim.x, dim.y); }

	static inline void Load(const rmlv::qfloat3* const src, __m128& rrrr, __m128& gggg, __m128& bbbb) {
		rrrr = _mm_load_ps(reinterpret_cast<const float*>(&src->r));
		gggg = _mm_load_ps(reinterpret_cast<const float*>(&src->g));
		bbbb = _mm_load_ps(reinterpret_cast<const float*>(&src->b)); }

	/*static inline void Load(const rmlv::qfloat4* const src, __m128& rrrr, __m128& gggg, __m128& bbbb, __m128& aaaa) {
		rrrr = _mm_load_ps(reinterpret_cast<const float*>(&src->r));
		gggg = _mm_load_ps(reinterpret_cast<const float*>(&src->g));
		bbbb = _mm_load_ps(reinterpret_cast<const float*>(&src->b));
		aaaa = _mm_load_ps(reinterpret_cast<const float*>(&src->a)); }*/

	static inline void Store(__m128 rrrr, __m128 gggg, __m128 bbbb, rmlv::qfloat3* const dst) {
		_mm_store_ps(reinterpret_cast<float*>(&dst->r), rrrr);
		_mm_store_ps(reinterpret_cast<float*>(&dst->g), gggg);
		_mm_store_ps(reinterpret_cast<float*>(&dst->b), bbbb); }

	/*static inline void Store(__m128 rrrr, __m128 gggg, __m128 bbbb, __m128 aaaa, rmlv::qfloat4* const dst) {
		_mm_store_ps(reinterpret_cast<float*>(&dst->r), rrrr);
		_mm_store_ps(reinterpret_cast<float*>(&dst->g), gggg);
		_mm_store_ps(reinterpret_cast<float*>(&dst->b), bbbb);
		_mm_store_ps(reinterpret_cast<float*>(&dst->a), aaaa); }*/

	rmlv::vec3 get_pixel(int x, int y) {
		const auto* data = reinterpret_cast<const float*>(ptr_);
		const int channels = 3;
		const int pixelsPerQuad = 4;
		const int elemsPerQuad = channels * pixelsPerQuad;

		const int Yquad = y / 2;
		const int Xquad = x / 2;

		const int quadAddr = (Yquad * stride_ + Xquad) * elemsPerQuad;

		//int cellidx = y / 2 * _stride2 + x / 2;
		int sy = y % 2;
		int sx = x % 2;
		float r = data[ quadAddr +  0 + sy*2 + sx ];
		float g = data[ quadAddr +  4 + sy*2 + sx ];
		float b = data[ quadAddr +  8 + sy*2 + sx ];
		// float a = data[ quadAddr + 12 + sy*2 + sx ];
		return rmlv::vec3{ r, g, b }; }};


class QFloat4Canvas {
	int width_{0};
	int height_{0};
	int stride_{0};
	rcls::vector<rmlv::qfloat4> buffer_;
	rmlv::qfloat4* ptr_{nullptr};

public:
	QFloat4Canvas() = default;

	QFloat4Canvas(int w, int h) :
		width_(w),
		height_(h),
		stride_(w / 2) {
		buffer_.reserve(w*h/4);
		ptr_ = buffer_.data(); }

	QFloat4Canvas(int w, int h, rmlv::qfloat4* buf) :
		width_(w),
		height_(h),
		stride_(w / 2),
		ptr_(buf) {}

	QFloat4Canvas(int w, int h, rmlv::qfloat4* buf, int stride) :
		width_(w),
		height_(h),
		stride_(stride),
		ptr_(buf) {}

	auto data() { return ptr_; }
	auto cdata() const { return ptr_; }

	auto width() const { return width_; }
	auto height() const { return height_; }
	auto stride() const { return stride_; }
	rmlg::irect rect() const {
		return rmlg::irect{ rmlv::ivec2{0,0}, rmlv::ivec2{width_, height_} }; }
	float aspect() const {
		if (width_ == 0 || height_ == 0) {
			return 1.0F; }
		return float(width_) / float(height_); }

	void resize(int w, int h) {
		buffer_.reserve(w*h/4);
		ptr_ = buffer_.data();
		width_ = w;
		height_ = h;
		stride_ = w/2; }

	void resize(rmlv::ivec2 dim) {
		resize(dim.x, dim.y); }

	static inline void Load(const rmlv::qfloat4* const src, __m128& rrrr, __m128& gggg, __m128& bbbb) {
		rrrr = _mm_load_ps(reinterpret_cast<const float*>(&src->r));
		gggg = _mm_load_ps(reinterpret_cast<const float*>(&src->g));
		bbbb = _mm_load_ps(reinterpret_cast<const float*>(&src->b)); }

	static inline void Load(const rmlv::qfloat4* const src, __m128& rrrr, __m128& gggg, __m128& bbbb, __m128& aaaa) {
		rrrr = _mm_load_ps(reinterpret_cast<const float*>(&src->r));
		gggg = _mm_load_ps(reinterpret_cast<const float*>(&src->g));
		bbbb = _mm_load_ps(reinterpret_cast<const float*>(&src->b));
		aaaa = _mm_load_ps(reinterpret_cast<const float*>(&src->a)); }

	static inline void Store(__m128 rrrr, __m128 gggg, __m128 bbbb, rmlv::qfloat4* const dst) {
		_mm_store_ps(reinterpret_cast<float*>(&dst->r), rrrr);
		_mm_store_ps(reinterpret_cast<float*>(&dst->g), gggg);
		_mm_store_ps(reinterpret_cast<float*>(&dst->b), bbbb); }

	static inline void Store(__m128 rrrr, __m128 gggg, __m128 bbbb, __m128 aaaa, rmlv::qfloat4* const dst) {
		_mm_store_ps(reinterpret_cast<float*>(&dst->r), rrrr);
		_mm_store_ps(reinterpret_cast<float*>(&dst->g), gggg);
		_mm_store_ps(reinterpret_cast<float*>(&dst->b), bbbb);
		_mm_store_ps(reinterpret_cast<float*>(&dst->a), aaaa); }

	rmlv::vec4 get_pixel(int x, int y) {
		const auto* data = reinterpret_cast<const float*>(ptr_);
		const int channels = 4;
		const int pixelsPerQuad = 4;
		const int elemsPerQuad = channels * pixelsPerQuad;

		const int Yquad = y / 2;
		const int Xquad = x / 2;

		const int quadAddr = (Yquad * stride_ + Xquad) * elemsPerQuad;

		//int cellidx = y / 2 * _stride2 + x / 2;
		int sy = y % 2;
		int sx = x % 2;
		float r = data[ quadAddr +  0 + sy*2 + sx ];
		float g = data[ quadAddr +  4 + sy*2 + sx ];
		float b = data[ quadAddr +  8 + sy*2 + sx ];
		float a = data[ quadAddr + 12 + sy*2 + sx ];
		return rmlv::vec4{ r, g, b, a }; }};

struct QShort3 {
	uint16_t r[4];
	uint16_t g[4];
	uint16_t b[4]; };


struct QShort4 {
	uint16_t r[4];
	uint16_t g[4];
	uint16_t b[4];
	uint16_t a[4]; };


struct QShort3Canvas {
	static constexpr float scale{255.0F};
	QShort3Canvas() = default;

	QShort3Canvas(const int width, const int height) :
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

	void resize(int width, int height) {
		buffer_.reserve(width * height / 4);
		ptr_ = buffer_.data();
		width_ = width;
		height_ = height;
		stride2_ = width / 2; }

	void resize(rmlv::ivec2 dim) {
		resize(dim.x, dim.y); }

	static inline void Load(const QShort3* const src, __m128& rrrr, __m128& gggg, __m128& bbbb) {
		const __m128 factor = _mm_set1_ps(1.0F / scale);
		__m128i irig = _mm_loadu_si128((__m128i*)&src->r);
		__m128i ibia = _mm_loadu_si128((__m128i*)&src->b);
		__m128i zero = _mm_setzero_si128();
		__m128i ir = _mm_unpacklo_epi16(irig, zero);
		__m128i ig = _mm_unpackhi_epi16(irig, zero);
		__m128i ib = _mm_unpacklo_epi16(ibia, zero);
		rrrr = _mm_mul_ps(_mm_cvtepi32_ps(ir), factor);
		gggg = _mm_mul_ps(_mm_cvtepi32_ps(ig), factor);
		bbbb = _mm_mul_ps(_mm_cvtepi32_ps(ib), factor); }

	static inline void Store(__m128 rrrr, __m128 gggg, __m128 bbbb, QShort3* dst) {
		const __m128 factor = _mm_set1_ps(scale);
		__m128i ir = _mm_cvtps_epi32(_mm_mul_ps(rrrr, factor));
		__m128i ig = _mm_cvtps_epi32(_mm_mul_ps(gggg, factor));
		__m128i ib = _mm_cvtps_epi32(_mm_mul_ps(bbbb, factor));
		__m128i irig = _mm_packs_epi32(ir, ig);
		__m128i ibxx = _mm_packs_epi32(ib, ib);
		_mm_storeu_si128((__m128i*)&dst->r, irig);
		_mm_storeu_si64((__m128i*)&dst->b, ibxx); }

private:
	rcls::vector<QShort3> buffer_;
	QShort3* ptr_{nullptr};
	int width_{0};
	int height_{0};
	int stride2_{0}; };


struct QShort4Canvas {
	static constexpr float scale{255.0F};
	QShort4Canvas() = default;

	QShort4Canvas(const int width, const int height) :
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

	void resize(int width, int height) {
		buffer_.reserve(width * height / 4);
		ptr_ = buffer_.data();
		width_ = width;
		height_ = height;
		stride2_ = width / 2; }

	void resize(rmlv::ivec2 dim) {
		resize(dim.x, dim.y); }

	static inline void Load(const QShort4* const src, __m128& rrrr, __m128& gggg, __m128& bbbb) {
		const __m128 factor = _mm_set1_ps(1.0F / scale);
		__m128i irig = _mm_load_si128((__m128i*)&src->r);
		__m128i ibia = _mm_load_si128((__m128i*)&src->b);
		__m128i zero = _mm_setzero_si128();
		__m128i ir = _mm_unpacklo_epi16(irig, zero);
		__m128i ig = _mm_unpackhi_epi16(irig, zero);
		__m128i ib = _mm_unpacklo_epi16(ibia, zero);
		rrrr = _mm_mul_ps(_mm_cvtepi32_ps(ir), factor);
		gggg = _mm_mul_ps(_mm_cvtepi32_ps(ig), factor);
		bbbb = _mm_mul_ps(_mm_cvtepi32_ps(ib), factor); }

	static inline void Store(__m128 rrrr, __m128 gggg, __m128 bbbb, QShort4* dst) {
		const __m128 factor = _mm_set1_ps(scale);
		__m128i ir = _mm_cvtps_epi32(_mm_mul_ps(rrrr, factor));
		__m128i ig = _mm_cvtps_epi32(_mm_mul_ps(gggg, factor));
		__m128i ib = _mm_cvtps_epi32(_mm_mul_ps(bbbb, factor));
		__m128i irig = _mm_packs_epi32(ir, ig);
		__m128i ibxx = _mm_packs_epi32(ib, ib);
		_mm_store_si128((__m128i*)&dst->r, irig);
		_mm_store_si128((__m128i*)&dst->b, ibxx); }

private:
	rcls::vector<QShort4> buffer_;
	QShort4* ptr_{nullptr};
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
	FloatingPointCanvas(PixelToaster::FloatingPointPixel* ptr,
	                    int width, int height, int stride = 0) :
		userPtr_(ptr),
		ptr_(ptr),
		width_(width),
		height_(height),
		stride_(stride == 0 ? width : stride),
		aspect_(float(width) / float(height)) {}

	FloatingPointCanvas() {
		resize(1, 1); }

	void resize(int w, int h) {
		if (userPtr_ != nullptr) {
			throw std::runtime_error("can't resize canvas with user ptr"); }
		buf.reserve(w*h);
		width_ = w;
		height_ = h;
		stride_ = w;
		aspect_ = (float)w / h;
		ptr_ = buf.data(); }

	void resize(rmlv::ivec2 dim) {
		resize(dim.x, dim.y); }

	auto data() { return ptr_; }
	auto cdata() const { return ptr_; }
	auto width() const { return width_; }
	auto height() const { return height_; }
	auto stride() const { return stride_; }
	auto aspect() const { return aspect_; }
	rmlg::irect rect() const {
		return rmlg::irect{ rmlv::ivec2{0,0}, rmlv::ivec2{width_, height_} }; }
private:
	PixelToaster::FloatingPointPixel* userPtr_{nullptr};
	PixelToaster::FloatingPointPixel* ptr_{nullptr};
	int width_{0};
	int height_{0};
	int stride_{0};
	float aspect_{1.0F};
	rcls::vector<PixelToaster::FloatingPointPixel> buf; };


}  // namespace rglr
}  // namespace rqdq
