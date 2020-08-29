#pragma once
#include "src/rml/rmlv/rmlv_mmutil.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"

#include <cstdint>
#include <tmmintrin.h>
#include <intrin.h>
#include "3rdparty/sse-pow/sse_pow.h"

//#define USE_SSE3

namespace rqdq {
namespace rmlv {

                               // ------------
                               // class mvec4i
                               // ------------

struct alignas(16) mvec4i {

	// DATA
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4201)
#endif
	union {
		__m128i v;
		__m128 f;
		struct { int x, y, z, w; };
		uint32_t ui[4];
		int32_t si[4]; };
#ifdef _MSC_VER
#pragma warning(pop)
#endif

	// CLASS METHODS
	static auto zero() -> mvec4i;

	// CREATORS
	mvec4i() = default;
	mvec4i(mvec4i&&) = default;
	mvec4i(const mvec4i&) = default;

	mvec4i(int, int, int, int) noexcept;
	mvec4i(int) noexcept;
	mvec4i(__m128i) noexcept;

	// MANIPULATORS
	auto operator=(mvec4i&&) -> mvec4i& = default;
	auto operator=(const mvec4i&) -> mvec4i& = default;

	auto operator+=(const mvec4i&) -> mvec4i&;
	auto operator-=(const mvec4i&) -> mvec4i&;
	auto operator*=(const mvec4i&) -> mvec4i&;

	auto operator|=(const mvec4i&) -> mvec4i&;
	auto operator&=(const mvec4i&) -> mvec4i&; };

                               // ------------
                               // class mvec4f
                               // ------------

struct alignas(16) mvec4f {

	// DATA
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4201)
#endif
	union {
		__m128 v;
		struct { float x, y, z, w; };
		struct { float r, g, b, a; };
		float lane[4]; };
#ifdef _MSC_VER
#pragma warning(pop)
#endif

	// CLASS METHODS
	static auto zero() -> mvec4f;
	static auto all_ones() -> mvec4f;

	// CREATORS
	mvec4f() = default;
	mvec4f(mvec4f&&) = default;
	mvec4f(const mvec4f&) = default;

	mvec4f(float, float, float, float) noexcept;
	mvec4f(float) noexcept;
	mvec4f(__m128 m) noexcept;
	explicit mvec4f(bool a) noexcept;

	// MANIPULATORS
	auto operator=(mvec4f&&) -> mvec4f& = default;
	auto operator=(const mvec4f&) -> mvec4f& = default;

	auto operator+=(const mvec4f&) -> mvec4f&;
	auto operator-=(const mvec4f&) -> mvec4f&;
	auto operator*=(const mvec4f&) -> mvec4f&;
	auto operator/=(const mvec4f&) -> mvec4f&;

	auto operator+=(float rhs) -> mvec4f&;
	auto operator-=(float rhs) -> mvec4f&;
	auto operator*=(float rhs) -> mvec4f&;
	auto operator/=(float rhs) -> mvec4f&;

	// ACCESSORS
	auto as_vec2() const -> vec2;
	auto as_vec3() const -> vec3;
	auto as_vec4() const -> vec4;

	auto xxxx() const -> mvec4f;
	auto yyyy() const -> mvec4f;
	auto zzzz() const -> mvec4f;
	auto wwww() const -> mvec4f;

	auto yzxw() const -> mvec4f;
	auto zxyw() const -> mvec4f;

	auto xyxy() const -> mvec4f;
	auto zwzw() const -> mvec4f;

	auto yyww() const -> mvec4f;
	auto xxzz() const -> mvec4f;

	auto xyz0() const -> mvec4f;

	auto get_x() const -> float;
	auto get_y() const -> float;
	auto get_z() const -> float;
	auto get_w() const -> float; };

// FREE OPERATORS
auto operator-(const mvec4f&) -> mvec4f;
auto operator*(const float, const mvec4f&) -> mvec4f;
auto operator+(const mvec4f&, const mvec4f&) -> mvec4f;
auto operator-(const mvec4f&, const mvec4f&) -> mvec4f;
auto operator*(const mvec4f&, const mvec4f&) -> mvec4f;
auto operator/(const mvec4f&, const mvec4f&) -> mvec4f;
auto operator+(const mvec4f&, float) -> mvec4f;
auto operator-(const mvec4f&, float) -> mvec4f;
auto operator*(const mvec4f&, float) -> mvec4f;
auto operator/(const mvec4f&, float) -> mvec4f;

auto operator&(const mvec4f&, const mvec4f&) -> mvec4f;
auto operator|(const mvec4f&, const mvec4f&) -> mvec4f;

auto operator-(float, const mvec4f&) -> mvec4f;
auto operator+(float, const mvec4f&) -> mvec4f;

auto operator+(const mvec4i&, const mvec4i&) -> mvec4i;
auto operator-(const mvec4i&, const mvec4i&) -> mvec4i;
auto operator*(const mvec4i&, const mvec4i&) -> mvec4i;

auto operator|(const mvec4i&, const mvec4i&) -> mvec4i;
auto operator&(const mvec4i&, const mvec4i&) -> mvec4i;

// FREE FUNCTIONS
auto store_bytes(uint8_t* dst, mvec4i a) -> void;
auto load_interleaved_lut(const float *bp, mvec4i ofs, mvec4f& v0, mvec4f& v1, mvec4f& v2, mvec4f& v3) -> void;

auto abs(const mvec4f&) -> mvec4f;
auto sqrt(const mvec4f&) -> mvec4f;
auto rsqrt(const mvec4f&) -> mvec4f;
// auto cross(const mvec4f&, const mvec4f&) -> mvec4f;

/*
auto hadd(const mvec4f&) -> float;
auto hmax(const mvec4f&) -> float;
auto hmin(const mvec4f&) -> float;
*/

auto mix(const mvec4f&, const mvec4f&, float) -> mvec4f;
auto mix(const mvec4f&, const mvec4f&, const mvec4f&) -> mvec4f;

auto vmin(const mvec4f&, const mvec4f&) -> mvec4f;
auto vmax(const mvec4f&, const mvec4f&) -> mvec4f;

/**
 * select*() based on ryg's helpersse
 * sse2 equivalents first found in "sseplus" sourcecode
 * http://sseplus.sourceforge.net/group__emulated___s_s_e2.html#g3065fcafc03eed79c9d7539435c3257c
 *
 * i split them into bits and sign so i can save a step
 * when I know that I have a complete bitmask or not
 */
auto selectbits(mvec4f a, mvec4f b, mvec4i mask) -> mvec4f;
auto selectbits(mvec4f a, mvec4f b, mvec4f mask) -> mvec4f;

auto select_by_sign(mvec4f a, mvec4f b, mvec4i mask) -> mvec4f;
auto select_by_sign(mvec4f a, mvec4f b, mvec4f mask) -> mvec4f;

auto andnot(const mvec4i&, const mvec4i&) -> mvec4i;

auto itof(const mvec4i&)       -> mvec4f;
auto ftoi_round(const mvec4f&) -> mvec4i;
auto ftoi(const mvec4f&)       -> mvec4i;

auto cmplt(const mvec4i&, const mvec4i&) -> mvec4i;
// auto cmple(const mvec4i&, const mvec4i&) -> mvec4i;
// auto cmpge(const mvec4i&, const mvec4i&) -> mvec4i;
auto cmpeq(const mvec4i&, const mvec4i&) -> mvec4i;
auto cmpgt(const mvec4i&, const mvec4i&) -> mvec4i;

auto cmplt(const mvec4f&, const mvec4f&) -> mvec4f;
auto cmple(const mvec4f&, const mvec4f&) -> mvec4f;
auto cmpge(const mvec4f&, const mvec4f&) -> mvec4f;
auto cmpgt(const mvec4f&, const mvec4f&) -> mvec4f;

auto float2bits(const mvec4f&) -> mvec4i;
auto bits2float(const mvec4i&) -> mvec4f;

auto movemask(const mvec4f&) -> int;


// vec4 blendv(const vec4& a, const vec4& b, const mvec4i& mask) {
// 	/*sse 4.1 blendv wrapper*/
// 	return _mm_blendv_ps( a.v, b.v, bits2float(mask).v ); }

auto oneover(mvec4f) -> mvec4f;

auto pow(mvec4f, mvec4f) -> mvec4f;

template<int N> auto shl(mvec4i) -> mvec4i;
template<int N> auto sar(mvec4i) -> mvec4i;
template<int N> auto shr(mvec4i) -> mvec4i;

auto wrap1(mvec4f) -> mvec4f;

template<bool high_precision>
auto nick_sin(mvec4f) -> mvec4f;

auto sin1hp(mvec4f) -> mvec4f;
auto sin1lp(mvec4f) -> mvec4f;
auto sin(mvec4f) -> mvec4f;
auto cos(mvec4f) -> mvec4f;

auto sincos(mvec4f x, mvec4f &os, mvec4f& oc) -> void;
	// sin() & cos(), saving one mul

auto smoothstep(mvec4f a, mvec4f b, mvec4f t) -> mvec4f;
auto saturate(mvec4f) -> mvec4f;
	// clamp a to the range [0, 1.0]

auto fract(mvec4f) -> mvec4f;
	// fractional part of input floats

// ============================================================================
//                             INLINE DEFINITIONS
// ============================================================================

                               // ------------
                               // class mvec4i
                               // ------------

// CLASS METHODS
inline
auto mvec4i::zero() -> mvec4i {
	return mvec4i{_mm_setzero_si128()}; }

// CREATORS
inline
mvec4i::mvec4i(int a, int b, int c, int d) noexcept :
	v(_mm_set_epi32(d, c, b, a)) {}

inline
mvec4i::mvec4i(int a) noexcept :
	v(_mm_set_epi32(a, a, a, a)) {}

inline
mvec4i::mvec4i(__m128i m) noexcept :
	v(m) {}

// MANIPULATORS
inline
auto mvec4i::operator+=(const mvec4i& rhs) -> mvec4i& {
	v = _mm_add_epi32(v, rhs.v);
	return *this; }

inline
auto mvec4i::operator-=(const mvec4i& rhs) -> mvec4i& {
	v = _mm_sub_epi32(v, rhs.v);
	return *this; }

inline
auto mvec4i::operator*=(const mvec4i& rhs) -> mvec4i& {
	v = sse2_mul32(v, rhs.v);
	return *this; }

inline
auto mvec4i::operator|=(const mvec4i& rhs) -> mvec4i& {
	v = _mm_or_si128(v, rhs.v);
	return *this; }

inline
auto mvec4i::operator&=(const mvec4i& rhs) -> mvec4i& {
	v = _mm_and_si128(v, rhs.v);
	return *this; }

                                // ------------
                                // class mvec4f
                                // ------------

// CLASS METHODS
inline
auto mvec4f::zero() -> mvec4f {
	return _mm_setzero_ps(); }

inline
auto mvec4f::all_ones() -> mvec4f {
	__m128 a0 = _mm_setzero_ps();
	return _mm_cmpeq_ps(a0, a0); }

/*
	compute 1.0f, but does not work on clang
	__m128i t = _mm_cmpeq_epi16(t, t);
	t = _mm_slli_epi32(t, 25);
	t = _mm_srli_epi32(t, 2);
	return _mm_castsi128_ps(t); }
*/

// CREATORS
inline
mvec4f::mvec4f(float a, float b, float c, float d) noexcept :
	v(_mm_set_ps(d, c, b, a)) {}

inline
mvec4f::mvec4f(float a) noexcept :
	v(_mm_set1_ps(a)) {}

inline
mvec4f::mvec4f(__m128 m) noexcept :
	v(m) {}

inline
mvec4f::mvec4f(bool a) noexcept :
	v(a ? all_ones().v : zero().v) {}

// MANIPULATORS
inline auto mvec4f::operator+=(const mvec4f& rhs) -> mvec4f& { v = _mm_add_ps(v, rhs.v); return *this; }
inline auto mvec4f::operator-=(const mvec4f& rhs) -> mvec4f& { v = _mm_sub_ps(v, rhs.v); return *this; }
inline auto mvec4f::operator*=(const mvec4f& rhs) -> mvec4f& { v = _mm_mul_ps(v, rhs.v); return *this; }
inline auto mvec4f::operator/=(const mvec4f& rhs) -> mvec4f& { v = _mm_div_ps(v, rhs.v); return *this; }

inline auto mvec4f::operator+=(float rhs) -> mvec4f& { v = _mm_add_ps(v, _mm_set1_ps(rhs)); return *this; }
inline auto mvec4f::operator-=(float rhs) -> mvec4f& { v = _mm_sub_ps(v, _mm_set1_ps(rhs)); return *this; }
inline auto mvec4f::operator*=(float rhs) -> mvec4f& { v = _mm_mul_ps(v, _mm_set1_ps(rhs)); return *this; }
inline auto mvec4f::operator/=(float rhs) -> mvec4f& { v = _mm_div_ps(v, _mm_set1_ps(rhs)); return *this; }

// ACCESSORS
inline
auto mvec4f::as_vec2() const -> vec2 {
	alignas(16) float tmp[4];
	_mm_store_ps(tmp, v);
	return vec2{ tmp[0], tmp[1] }; }

inline
auto mvec4f::as_vec3() const -> vec3 {
	alignas(16) float tmp[4];
	_mm_store_ps(tmp, v);
	return vec3{ tmp[0], tmp[1], tmp[2] }; }

inline
auto mvec4f::as_vec4() const -> vec4 {
	vec4 tmp;
	_mm_store_ps(reinterpret_cast<float*>(&tmp), v);
	return tmp; }

inline auto mvec4f::xxxx() const -> mvec4f { return _mm_shuffle_ps(v, v, _MM_SHUFFLE(0, 0, 0, 0)); }
inline auto mvec4f::yyyy() const -> mvec4f { return _mm_shuffle_ps(v, v, _MM_SHUFFLE(1, 1, 1, 1)); }
inline auto mvec4f::zzzz() const -> mvec4f { return _mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 2, 2, 2)); }
inline auto mvec4f::wwww() const -> mvec4f { return _mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 3, 3, 3)); }

inline auto mvec4f::yzxw() const -> mvec4f { return _mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 0, 2, 1)); }
inline auto mvec4f::zxyw() const -> mvec4f { return _mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 1, 0, 2)); }

inline auto mvec4f::xyxy() const -> mvec4f { return _mm_shuffle_ps(v, v, _MM_SHUFFLE(1, 0, 1, 0)); }
inline auto mvec4f::zwzw() const -> mvec4f { return _mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 2, 3, 2)); }

inline auto mvec4f::yyww() const -> mvec4f { return _mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 3, 1, 1)); }
inline auto mvec4f::xxzz() const -> mvec4f { return _mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 2, 0, 0)); }

inline auto mvec4f::xyz0() const -> mvec4f {
	__m128 xyz0mask = _mm_cmpeq_ps(_mm_setzero_ps(), _mm_set_ps(1, 0, 0, 0));
	return _mm_and_ps(v, xyz0mask); }

inline auto mvec4f::get_x() const -> float { return _mm_cvtss_f32(v); }
inline auto mvec4f::get_y() const -> float { return _mm_cvtss_f32(yyyy().v); }
inline auto mvec4f::get_z() const -> float { return _mm_cvtss_f32(zzzz().v); }
inline auto mvec4f::get_w() const -> float { return _mm_cvtss_f32(wwww().v); }


// FREE OPERATORS

inline auto operator+(const mvec4f& lhs, const mvec4f& rhs) -> mvec4f { return _mm_add_ps(lhs.v, rhs.v); }
inline auto operator-(const mvec4f& lhs, const mvec4f& rhs) -> mvec4f { return _mm_sub_ps(lhs.v, rhs.v); }
inline auto operator*(const mvec4f& lhs, const mvec4f& rhs) -> mvec4f { return _mm_mul_ps(lhs.v, rhs.v); }
inline auto operator/(const mvec4f& lhs, const mvec4f& rhs) -> mvec4f { return _mm_div_ps(lhs.v, rhs.v); }

inline auto operator+(const mvec4f& lhs, float rhs) -> mvec4f { return _mm_add_ps(lhs.v, _mm_set1_ps(rhs)); }
inline auto operator-(const mvec4f& lhs, float rhs) -> mvec4f { return _mm_sub_ps(lhs.v, _mm_set1_ps(rhs)); }
inline auto operator*(const mvec4f& lhs, float rhs) -> mvec4f { return _mm_mul_ps(lhs.v, _mm_set1_ps(rhs)); }
inline auto operator/(const mvec4f& lhs, float rhs) -> mvec4f { return _mm_div_ps(lhs.v, _mm_set1_ps(rhs)); }

inline
auto operator*(const float a, const mvec4f& b) -> mvec4f {
	return _mm_mul_ps(_mm_set1_ps(a), b.v); }

inline
auto operator-(const mvec4f& a) -> mvec4f {
	__m128 signmask = _mm_castsi128_ps(_mm_set1_epi32(0x80000000));
	return _mm_xor_ps(a.v, signmask); }

inline auto operator+(float lhs, const mvec4f& rhs) -> mvec4f { return mvec4f{ lhs } + rhs; }
inline auto operator-(float lhs, const mvec4f& rhs) -> mvec4f { return mvec4f{ lhs } - rhs; }
inline auto operator+(const mvec4i& a, const mvec4i& b) -> mvec4i { return mvec4i(_mm_add_epi32(a.v, b.v)); }
inline auto operator-(const mvec4i& a, const mvec4i& b) -> mvec4i { return mvec4i(_mm_sub_epi32(a.v, b.v)); }
inline auto operator*(const mvec4i& a, const mvec4i& b) -> mvec4i { return mvec4i(sse2_mul32(a.v, b.v)); }

inline auto operator|(const mvec4f& a, const mvec4f& b) -> mvec4f { return _mm_or_ps(a.v, b.v); }
inline auto operator&(const mvec4f& a, const mvec4f& b) -> mvec4f { return _mm_and_ps(a.v, b.v); }
inline auto operator|(const mvec4i& a, const mvec4i& b) -> mvec4i { return mvec4i(_mm_or_si128(a.v, b.v)); }
inline auto operator&(const mvec4i& a, const mvec4i& b) -> mvec4i { return mvec4i(_mm_and_si128(a.v, b.v)); }



// FREE FUNCTIONS
/*
inline void store_bytes(uint8_t* dst, const mvec4i a) {
	alignas(16) uint8_t data[16];
	_mm_store_si128(reinterpret_cast<__m128i*>(data), a.v);
	dst[0] = data[0];
	dst[1] = data[4];
	dst[2] = data[8];
	dst[3] = data[12]; }
*/

inline
auto store_bytes(uint8_t* dst, mvec4i a) -> void {
	//alignas(16) uint8_t data[16];
	//_mm_store_si128(reinterpret_cast<__m128i*>(data), a.v);
	dst[0] = static_cast<uint8_t>(a.ui[0]);
	dst[1] = static_cast<uint8_t>(a.ui[1]);
	dst[2] = static_cast<uint8_t>(a.ui[2]);
	dst[3] = static_cast<uint8_t>(a.ui[3]); }

/*
inline void store_bytes(uint8_t* dst, const mvec4i a) {
	//XXX does this work on core2duox64?
	__m128i control = _mm_set_epi8(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 8, 4, 0);
	__m128i packed = _mm_shuffle_epi8(a.v, control);
	_mm_storeu_si128(reinterpret_cast<__m128i*>(dst), packed); }
*/

inline
auto load_lut(const float* bp, mvec4i ofs) -> mvec4f {
	mvec4f out;
	out.x = bp[ofs.si[0]];
	out.y = bp[ofs.si[1]];
	out.z = bp[ofs.si[2]];
	out.w = bp[ofs.si[3]];
	return out; }


inline
auto load_interleaved_lut(const float *bp,
                          mvec4i ofs,
                          mvec4f& v0, mvec4f& v1, mvec4f& v2, mvec4f& v3) -> void {
	alignas(16) int idx[4];
	_mm_store_si128(reinterpret_cast<__m128i*>(idx), ofs.v);
	v0 = _mm_load_ps(bp + idx[0]);
	v1 = _mm_load_ps(bp + idx[1]);
	v2 = _mm_load_ps(bp + idx[2]);
	v3 = _mm_load_ps(bp + idx[3]);
	_MM_TRANSPOSE4_PS(v0.v, v1.v, v2.v, v3.v); }

inline
auto abs(const mvec4f& a) -> mvec4f {
	__m128 signmask = _mm_castsi128_ps(_mm_set1_epi32(0x80000000));
	return _mm_andnot_ps(signmask, a.v); }

inline
auto sqrt(const mvec4f& a) -> mvec4f {
	return _mm_sqrt_ps(a.v); }

inline
auto rsqrt(const mvec4f& a) -> mvec4f {
	return _mm_rsqrt_ps(a.v); }

// inline
// auto cross(const mvec4f &a, const mvec4f &b) -> mvec4f {
//  	return a.yzxw()*b.zxyw() - a.zxyw()*b.yzxw(); }

/*
inline
auto hadd(const mvec4f& a) -> float {
	__m128 r0 = _mm_shuffle_ps(a.v, a.v, _MM_SHUFFLE(0, 3, 2, 1));
	__m128 r1 = _mm_add_ps(a.v, r0);
	r0 = _mm_shuffle_ps(r1, r1, _MM_SHUFFLE(2, 2, 2, 2));
	r1 = _mm_add_ss(r0, r1);
	return _mm_cvtss_f32(r1); }

inline
auto hmax(const mvec4f& a) -> float {
	__m128 r0 = _mm_shuffle_ps(a.v, a.v, _MM_SHUFFLE(0, 3, 2, 1));
	__m128 r1 = _mm_max_ps(a.v, r0);
	r0 = _mm_shuffle_ps(r1, r1, _MM_SHUFFLE(2, 2, 2, 2));
	r1 = _mm_max_ss(r0, r1);
	return _mm_cvtss_f32(r1); }

inline
auto hmin(const mvec4f& a) -> float {
	__m128 r0 = _mm_shuffle_ps(a.v, a.v, _MM_SHUFFLE(0, 3, 2, 1));
	__m128 r1 = _mm_min_ps(a.v, r0);
	r0 = _mm_shuffle_ps(r1, r1, _MM_SHUFFLE(2, 2, 2, 2));
	r1 = _mm_min_ss(r0, r1);
	return _mm_cvtss_f32(r1); }
*/

inline
auto mix(const mvec4f &a, const mvec4f &b, const float t) -> mvec4f {
	//return (1.0f - t)*a + t*b; }
	return a + mvec4f{t} * (b - a); }  // xxx faster, but incorrect?

inline
auto mix(const mvec4f &a, const mvec4f &b, const mvec4f& t) -> mvec4f {
	//return (mvec4f{ 1.0f }-t)*a + t*b; }
	return a + t * (b - a); }   // xxx faster, but incorrect?

inline auto vmin(const mvec4f& a, const mvec4f& b) -> mvec4f { return _mm_min_ps(a.v, b.v); }
inline auto vmax(const mvec4f& a, const mvec4f& b) -> mvec4f { return _mm_max_ps(a.v, b.v); }

inline auto andnot(const mvec4i& a, const mvec4i& b) -> mvec4i { return mvec4i(_mm_andnot_si128(a.v, b.v)); }

inline auto itof      (const mvec4i& a) -> mvec4f { return _mm_cvtepi32_ps(a.v); }
inline auto ftoi_round(const mvec4f& a) -> mvec4i { return _mm_cvtps_epi32(a.v); }
inline auto ftoi      (const mvec4f& a) -> mvec4i { return _mm_cvttps_epi32(a.v); }

inline auto cmplt(const mvec4i&a, const mvec4i& b) -> mvec4i { return mvec4i(_mm_cmplt_epi32(a.v, b.v)); }
//inline mvec4i cmple(const mvec4i&a, const mvec4i& b) { return mvec4i(_mm_cmple_epi32(a.v,b.v)); }
//inline mvec4i cmpge(const mvec4i&a, const mvec4i& b) { return mvec4i(_mm_cmpge_epi32(a.v,b.v)); }
inline auto cmpeq(const mvec4i&a, const mvec4i& b) -> mvec4i { return mvec4i(_mm_cmpeq_epi32(a.v, b.v)); }
inline auto cmpgt(const mvec4i&a, const mvec4i& b) -> mvec4i { return mvec4i(_mm_cmpgt_epi32(a.v, b.v)); }

inline auto cmpeq(const mvec4f&a, const mvec4f& b) -> mvec4f { return _mm_cmpeq_ps(a.v, b.v); }
inline auto cmplt(const mvec4f&a, const mvec4f& b) -> mvec4f { return _mm_cmplt_ps(a.v, b.v); }
inline auto cmple(const mvec4f&a, const mvec4f& b) -> mvec4f { return _mm_cmple_ps(a.v, b.v); }
inline auto cmpge(const mvec4f&a, const mvec4f& b) -> mvec4f { return _mm_cmpge_ps(a.v, b.v); }
inline auto cmpgt(const mvec4f&a, const mvec4f& b) -> mvec4f { return _mm_cmpgt_ps(a.v, b.v); }

inline auto float2bits(const mvec4f &a) -> mvec4i { return _mm_castps_si128(a.v); }
inline auto bits2float(const mvec4i &a) -> mvec4f { return _mm_castsi128_ps(a.v); }

inline auto movemask(const mvec4f& a) -> int { return _mm_movemask_ps(a.v); }

inline
auto saturate(mvec4f a) -> mvec4f {
	return vmax(vmin(a, mvec4f{1.0F}), mvec4f::zero()); }

inline
auto fract(mvec4f a) -> mvec4f {
#if 1
	__m128 ipart = _mm_cvtepi32_ps(_mm_cvttps_epi32(a.v));
	return _mm_sub_ps(a.v, ipart); 
#else
	return a - itof(ftoi_round(a - mvec4f{0.5F}));
#endif
	}

inline
auto selectbits(mvec4f a, mvec4f b, mvec4i mask) -> mvec4f {
	const mvec4i a2 = andnot(mask, float2bits(a));  // keep bits in a where mask is 0000's
	const mvec4i b2 = float2bits(b) & mask;         // keep bits in b where mask is 1111's
	return bits2float(a2 | b2); }

inline
auto selectbits(mvec4f a, mvec4f b, mvec4f mask) -> mvec4f {
	return selectbits(a, b, float2bits(mask)); }

/**
 * this is _mm_blendv_ps() for SSE2
 */
inline
auto select_by_sign(mvec4f a, mvec4f b, mvec4i mask) -> mvec4f {
	const mvec4i newmask(_mm_srai_epi32(mask.v, 31));
	return selectbits(a, b, newmask); }

inline
auto select_by_sign(mvec4f a, mvec4f b, mvec4f mask) -> mvec4f {
	const mvec4i newmask(_mm_srai_epi32(float2bits(mask).v, 31));
	return selectbits(a, b, newmask); }

inline
auto oneover(mvec4f a) -> mvec4f {
	__m128 res;

	// IEEE 
#if 0
	res = _mm_div_ps(_mm_set1_ps(1.0F), a.v);
#endif

	// SSE approx
#if 0
	res = _mm_rcp_ps(a.v);
#endif

	// approx + one NR iter
#if 1
	res = _mm_rcp_ps(a.v);
	__m128 muls = _mm_mul_ps(a.v, _mm_mul_ps(res, res));
	res = _mm_sub_ps(_mm_add_ps(res, res), muls);
#endif
	return res; }

inline
auto pow(mvec4f x, mvec4f y) -> mvec4f {
	return exp2f4(_mm_mul_ps(log2f4(x.v), y.v)); }

template<int N> inline auto shl(mvec4i x) -> mvec4i { return mvec4i(_mm_slli_epi32(x.v, N)); }
template<int N> inline auto sar(mvec4i x) -> mvec4i { return mvec4i(_mm_srai_epi32(x.v, N)); }
template<int N> inline auto shr(mvec4i x) -> mvec4i { return mvec4i(_mm_srli_epi32(x.v, N)); }

/*
 * some functions for sin/cos
 *
 * based mostly on Nick's thread here:
 * http://devmaster.net/forums/topic/4648-fast-and-accurate-sinecosine/page__st__80
 *
 * also referenced on pouet:
 * http://www.pouet.scene.org/topic.php?which=9132&page=1
 *
 * also interesting:
 * ISPC stdlib sincos, https://github.com/ispc/ispc/blob/master/stdlib.ispc
 *
 * and another here..
 * http://gruntthepeon.free.fr/ssemath/
 *
 * I'll use nick's.
 * It approximates sin() from a parabola, so it requires
 * the input be in the range -PI ... +PI
 *
 * These functions can be combined or decomposed to
 * optimize a few instructions if the inputs are known to
 * fit, and also if they are in the range -1 ... +1
 */

// clang's optimizer is too smart for this
// inline mvec4f wrap1(mvec4f a) {
// 	/*wrap a float to the range -1 ... +1 */
// 	const __m128 magic = _mm_set1_ps(25165824.0F); // 0x4bc00000
// 	const mvec4f z = a + magic;
// 	return a - (z - magic); }

/**
 * wrap a float to the range -1 ... +1
 */
inline
auto wrap1(mvec4f a) -> mvec4f {
	auto whole = ftoi(a);
	a = a - itof(whole);
	auto oddBit = shl<31>(whole);
	return _mm_xor_ps(a.v, bits2float(oddBit).v); }

/**
 * implementation of nick's original -pi...pi version
 */
template<bool high_precision>
inline
auto nick_sin(mvec4f x) -> mvec4f {
	const mvec4f B(4 / M_PI);
	const mvec4f C(-4 / (M_PI * M_PI));

	mvec4f y = B*x + C*x * abs(x);

	if (high_precision) {
		const mvec4f P(0.225);
		y = P * (y*abs(y) - y) + y; }

	return y; }

/**
 * higher-precision sin(), input must be in the range -1 ... 1
 */
inline
auto sin1hp(mvec4f x) -> mvec4f {
	const mvec4f Q(3.1F);
	const mvec4f P(3.6F);
	mvec4f y = x - (x * abs(x));
	return y * (Q + P * abs(y)); }

/**
 * low-precision sin(), input must be in the range -1 ... 1
 */
inline
auto sin1lp(mvec4f x) -> mvec4f {
	return mvec4f{4.0F} * (x - x * abs(x)); }

/**
 * drop-in-sin() replacement
 *
 * Args:
 * x: input in radians
 */
inline
auto sin(mvec4f x) -> mvec4f {
	mvec4f M = x * mvec4f(float(1.0 / M_PI));  // scale x to -1...1
	M = wrap1(M);  // wrap to -1...1
	return sin1hp(M); }

/**
 * drop-in-cos() replacement
 *
 * cos(x) = sin( pi/2 + x)
 * if... -1= -pi,  so...
 * bring into range -1 ... 1, then add 0.5, then wrap.
 */
inline
auto cos(mvec4f x) -> mvec4f {
	mvec4f M = x * mvec4f(float(1.0 / M_PI));  // scale x to -1...1
	M = wrap1(M + mvec4f{0.5F});  // add 0.5 (PI/2) and wrap
	return sin1hp(M); }

inline
auto sincos(mvec4f x, mvec4f &os, mvec4f& oc) -> void {
	mvec4f M = x * mvec4f{float(1.0F / M_PI)};  // scale x to -1...1
	os = sin1hp(wrap1(M));
	oc = sin1hp(wrap1(M + mvec4f{0.5F})); }

inline
auto smoothstep(mvec4f a, mvec4f b, mvec4f t) -> mvec4f {
	const mvec4f x = saturate((t - a) / (b - a));
	return x*x * (mvec4f{3.0F} - mvec4f{2.0F} * x); }


}  // namespace rmlv
}  // namespace rqdq
