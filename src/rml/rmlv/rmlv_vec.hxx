
#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <xmmintrin.h>

namespace rqdq {
namespace rmlv {

constexpr auto M_PI = 3.14159265358979323846;


template<typename T>
T lerp_fast(const T& a, const T& b, const T& f) {
	return a + (f*(b - a)); }


/*template<typename T>
T lerp(const T& a, const T& b, const T& f) {
	return (1 - f)*a + f*b; }*/


inline float lerp(const float& a, const float& b, const float& t) {
	return (1 - t)*a + t*b; }


inline float clamp(const float& x, const float &a, const float &b) {
	if (x < a) return a;
	if (x > b) return b;
	return x; }


template<typename T>
T lerp_premul(const T& a, const T& b, const T& f) {
	return (T(1) - f)*a + b; }


inline double fract(const double x) {
	double b;
	return modf(x, &b);
//	auto ux = static_cast<int>(x);
//	return x - ux;
	}


inline float fastsqrt(const float x) {
	union {
		int i;
		float x;
		} u;
	u.x = x;
	u.i = (1 << 29) + (u.i >> 1) - (1 << 22);
	return u.x; }


inline double fastpow(double a, double b) {
	union {
		double d;
		int x[2];
	} u = { a };
	u.x[1] = (int)(b * (u.x[1] - 1072632447) + 1072632447);
	u.x[0] = 0;
	return u.d; }


struct alignas(8) vec2 {
	vec2() = default;
	constexpr vec2(float a) noexcept :x(a), y(a) {}
	constexpr vec2(float a, float b) noexcept :x(a), y(b) {}

	vec2 operator+(const vec2& b) const { return vec2(x + b.x, y + b.y); }
	vec2 operator-(const vec2& b) const { return vec2(x - b.x, y - b.y); }
	vec2 operator*(const vec2& b) const { return vec2(x * b.x, y * b.y); }
	vec2 operator/(const vec2& b) const { return vec2(x / b.x, y / b.y); }

	vec2 operator+(const float b) const { return vec2(x + b, y + b); }
	vec2 operator-(const float b) const { return vec2(x - b, y - b); }
	vec2 operator*(const float b) const { return vec2(x * b, y * b); }
	vec2 operator/(const float b) const { return vec2(x / b, y / b); }

	vec2& operator+=(const vec2& b) { x += b.x; y += b.y; return *this; }
	vec2& operator-=(const vec2& b) { x -= b.x; y -= b.y; return *this; }
	vec2& operator*=(const vec2& b) { x *= b.x; y *= b.y; return *this; }
	vec2& operator/=(const vec2& b) { x /= b.x; y /= b.y; return *this; }

	vec2& operator+=(const float b) { x += b; y += b; return *this; }
	vec2& operator-=(const float b) { x -= b; y -= b; return *this; }
	vec2& operator*=(const float b) { x *= b; y *= b; return *this; }
	vec2& operator/=(const float b) { x /= b; y /= b; return *this; }

	float x, y; };


inline vec2 operator-(const vec2& a) { return vec2(-a.x, -a.y); }

inline vec2 operator*(const float b, const vec2& a) { return vec2(a.x*b, a.y*b); }

inline float dot(const vec2& a, const vec2& b) {
	return a.x*b.x + a.y*b.y; }

inline vec2 normalize(const vec2& a) {
	return (1.0f / sqrt(dot(a, a)))*a; }

inline vec2 lerp(const vec2& a, const vec2& b, const float t) {
	return vec2( lerp(a.x,b.x,t), lerp(a.y,b.y,t) ); }

inline vec2 abs(const vec2& a) {
	return vec2(fabs(a.x), fabs(a.y)); }


struct alignas(4) vec3 {
	vec3() = default;
	constexpr vec3(const vec2 a, const float z) noexcept : x(a.x), y(a.y), z(z) {}
	constexpr vec3(const float a, const float b, const float c) noexcept : x(a), y(b), z(c) {}
	constexpr vec3(const float a) noexcept : x(a), y(a), z(a) {}

	vec3 operator+(const vec3& b) const { return { x + b.x, y + b.y, z + b.z }; }
	vec3 operator-(const vec3& b) const { return { x - b.x, y - b.y, z - b.z }; }
	vec3 operator*(const vec3& b) const { return { x * b.x, y * b.y, z * b.z }; }
	vec3 operator/(const vec3& b) const { return { x / b.x, y / b.y, z / b.z }; }

	vec3 operator+(const float b) const { return { x + b, y + b, z + b }; }
	vec3 operator-(const float b) const { return { x - b, y - b, z - b }; }
	vec3 operator*(const float b) const { return { x * b, y * b, z * b }; }
	vec3 operator/(const float b) const { return { x / b, y / b, z / b }; }

	vec3& operator+=(const vec3& b) { x += b.x; y += b.y; z += b.z; return *this; }
	vec3& operator-=(const vec3& b) { x -= b.x; y -= b.y; z -= b.z; return *this; }
	vec3& operator*=(const vec3& b) { x *= b.x; y *= b.y; z *= b.z; return *this; }
	vec3& operator/=(const vec3& b) { x /= b.x; y /= b.y; z /= b.z; return *this; }

	vec3& operator+=(const float b) { x += b; y += b; z += b; return *this; }
	vec3& operator-=(const float b) { x -= b; y -= b; z -= b; return *this; }
	vec3& operator*=(const float b) { x *= b; y *= b; z *= b; return *this; }
	vec3& operator/=(const float b) { x /= b; y /= b; z /= b; return *this; }

	static inline vec3 from_rgb(const int rgb) {
		return vec3{
			(float)((rgb >> 16) & 0xff) / 256.0f,
			(float)((rgb >> 8) & 0xff) / 256.0f,
			(float)((rgb >> 0) & 0xff) / 256.0f }; }

	vec2 xy() const { return vec2{x, y}; }

	union {
		struct { float x, y, z; };
		std::array<float, 3> arr; }; };


/*	vec3 operator+(float a, const vec3& b) { return b + a; }
	vec3 operator-(float a, const vec3& b) { return vec3(_mm_set1_ps(a)) - b; }*/
inline vec3 operator*(float a, const vec3& b) { return{ a*b.x, a*b.y, a*b.z }; }
/*	inline vec3 operator/(float a, const vec3& b) { return vec3(_mm_set1_ps(a)) / b; }*/

inline vec3 operator-(const vec3& a) {
	return{ -a.x, -a.y, -a.z }; }

inline vec3 abs(const vec3& a) {
	return{ fabs(a.x), fabs(a.y), fabs(a.z) }; }

inline float dot(const vec3& a, const vec3& b) {
	return a.x*b.x + a.y*b.y + a.z*b.z; }

inline vec3 normalize(const vec3& a) {
	/*normalize a to length 1.0*/
	return a / sqrt(dot(a, a)); }
//return a * (1.0f / sqrt(dot(a, a))); }

inline float length(const vec3& a) {
	return sqrt(dot(a, a)); }

inline float hadd(const vec3& a) {
	return a.x + a.y + a.z; }

inline float hmax(const vec3& a) {
	using std::max;
	return max(max(a.x, a.y), a.z); }

inline float hmin(const vec3& a) {
	using std::min;
	return min(min(a.x, a.y), a.z); }

inline vec3 vmin(const vec3& a, const vec3& b) {
	using std::min;
	return{ min(a.x, b.x), min(a.y, b.y), min(a.z, b.z) }; }

inline vec3 vmax(const vec3& a, const vec3& b) {
	using std::max;
	return{ max(a.x, b.x), max(a.y, b.y), max(a.z, b.z) }; }

inline vec3 cross(const vec3& a, const vec3& b) {
	return{
		a.y*b.z - a.z*b.y,
		a.z*b.x - a.x*b.z,
		a.x*b.y - a.y*b.x }; }

inline vec3 reflect(const vec3& i, const vec3& n) {
	return i - vec3{ 2.0f } *dot(n, i) * n; }

inline vec3 lerp(const vec3 &a, const vec3 &b, const float t) {
	return (1.0f - t)*a + t*b; }


struct alignas(16) vec4 {
	vec4() = default;
	constexpr vec4(float a, float b, float c, float d) :x(a), y(b), z(c), w(d) {}
	constexpr vec4(float a) : x(a), y(a), z(a), w(a) {}
	constexpr vec4(const vec3 a, float b) : x(a.x), y(a.y), z(a.z), w(b) {}  // XXX shuffle/set1

	vec4 operator+(const vec4& b) const { return { x + b.x, y + b.y, z + b.z, w + b.w }; }
	vec4 operator-(const vec4& b) const { return { x - b.x, y - b.y, z - b.z, w - b.w }; }
	vec4 operator*(const vec4& b) const { return { x * b.x, y * b.y, z * b.z, w * b.w }; }
	vec4 operator/(const vec4& b) const { return { x / b.x, y / b.y, z / b.z, w / b.w }; }

	vec4 operator+(const float b) const { return { x + b, y + b, z + b, w + b }; }
	vec4 operator-(const float b) const { return { x - b, y - b, z - b, w - b }; }
	vec4 operator*(const float b) const { return { x * b, y * b, z * b, w * b }; }
	vec4 operator/(const float b) const { return { x / b, y / b, z / b, w / b }; }

	vec4& operator+=(const vec4& b) { x += b.x; y += b.y; z += b.z; w += b.w; return *this; }
	vec4& operator-=(const vec4& b) { x -= b.x; y -= b.y; z -= b.z; w -= b.w; return *this; }
	vec4& operator*=(const vec4& b) { x *= b.x; y *= b.y; z *= b.z; w *= b.w; return *this; }
	vec4& operator/=(const vec4& b) { x /= b.x; y /= b.y; z /= b.z; w /= b.w; return *this; }

	vec4& operator+=(const float b) { x += b; y += b; z += b; w += b; return *this; }
	vec4& operator-=(const float b) { x -= b; y -= b; z -= b; w -= b; return *this; }
	vec4& operator*=(const float b) { x *= b; y *= b; z *= b; w *= b; return *this; }
	vec4& operator/=(const float b) { x /= b; y /= b; z /= b; w /= b; return *this; }

	vec2 xy() { return vec2{ x, y }; }
	vec3 xyz() { return vec3{ x, y, z }; }

/*	inline vec4 from_rgb(const int rgb) {
		return vec4{
			(float)((rgb >> 16) & 0xff) / 256.0f,
			(float)((rgb >> 8) & 0xff) / 256.0f,
			(float)((rgb >> 0) & 0xff) / 256.0f}; }*/

	float x, y, z, w; };

inline vec4 operator*(float a, const vec4& b) { return{ a*b.x, a*b.y, a*b.z, a*b.w }; }

inline vec4 operator-(const vec4& a) {
	return{ -a.x, -a.y, -a.z, -a.w }; }

inline vec4 abs(const vec4& a) {
	return{ fabs(a.x), fabs(a.y), fabs(a.z), fabs(a.w) }; }

inline float dot(const vec4& a, const vec4& b) {
	return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w; }

inline vec4 normalize(const vec4 &a) {
	/*normalize a to length 1.0*/
	return a / sqrt(dot(a, a)); }

inline float length(const vec4& a) {
	return sqrt(dot(a, a)); }

inline vec4 sqrt(const vec4& a) {
	auto sx = sqrtf(a.x);
	auto sy = sqrtf(a.y);
	auto sz = sqrtf(a.z);
	auto sw = sqrtf(a.w);
	return{sx, sy, sz, sw}; }

inline float hadd(const vec4& a) {
	return a.x + a.y + a.z + a.w; }

inline float hmax(const vec4& a) {
	using std::max;
	return max(max(max(a.x, a.y), a.z), a.w); }

inline float hmin(const vec4& a) {
	using std::min;
	return min(min(min(a.x, a.y), a.z), a.w); }

inline vec4 vmin(const vec4& a, const vec4& b) {
	using std::min;
	return{ min(a.x, b.x), min(a.y, b.y), min(a.z, b.z), min(a.w, b.w) }; }

inline vec4 vmax(const vec4& a, const vec4& b) {
	using std::max;
	return{ max(a.x, b.x), max(a.y, b.y), max(a.z, b.z), max(a.w, b.w) }; }

inline vec4 cross(const vec4& a, const vec4& b) {
	return{
		a.y*b.z - a.z*b.y,
		a.z*b.x - a.x*b.z,
		a.x*b.y - a.y*b.x,
		a.w*b.w - a.w*b.w }; }

inline vec4 lerp(const vec4 &a, const vec4 &b, const float t) {
	return (1.0f - t)*a + t*b; }

struct alignas(8) ivec2 {
	ivec2() = default;
	constexpr ivec2(const vec2 a) noexcept : x(int(a.x)), y(int(a.y)) {}
	constexpr ivec2(const int a, const int b) noexcept : x(a), y(b){}
	constexpr ivec2(const int a) noexcept : x(a), y(a) {}

	ivec2 operator+(const ivec2& b) const { return ivec2(x + b.x, y + b.y); }
	ivec2 operator-(const ivec2& b) const { return ivec2(x - b.x, y - b.y); }
	ivec2 operator*(const ivec2& b) const { return ivec2(x * b.x, y * b.y); }
	ivec2 operator/(const ivec2& b) const { return ivec2(x / b.x, y / b.y); }

	ivec2& operator+=(const ivec2& b) { x += b.x; y += b.y; return *this; }
	ivec2& operator-=(const ivec2& b) { x -= b.x; y -= b.y; return *this; }

	ivec2 operator+(int b) const { return ivec2(x + b, y + b); }
	ivec2 operator-(int b) const { return ivec2(x - b, y - b); }

	static ivec2 min(const ivec2& a, const ivec2& b) {
		return {std::min(a.x, b.x),
		        std::min(a.y, b.y)}; }

	static ivec2 max(const ivec2& a, const ivec2& b) {
		return {std::max(a.x, b.x),
		        std::max(a.y, b.y)}; }

	int x, y; };


inline bool operator==(const ivec2& a, const ivec2& b) {
	return (a.x == b.x && a.y == b.y); }

inline bool operator!=(const ivec2& a, const ivec2& b) {
	return (a.x != b.x || a.y != b.y); }


struct ivec4 {
	ivec4() = default;
	constexpr ivec4(const int a, const int b, const int c, const int d) noexcept :x(a), y(b), z(c), w(d) {}
	constexpr ivec4(const int a) noexcept : x(a), y(a), z(a), w(a) {}

	ivec4& operator+=(const ivec4& b) { x += b.x; y += b.y; z += b.z; return *this; }
	ivec4& operator-=(const ivec4& b) { x -= b.x; y -= b.y; z -= b.z; return *this; }
	ivec4& operator*=(const ivec4& b) { x *= b.x; y *= b.y; z *= b.z; return *this; }

	union {
		struct { int x, y, z, w; };
		int32_t si[4]; }; };


/*
 * perspective divide, scalar version
 *
 * float operations _must_ match SoA version !
 *
 * Args:
 * p: point in homogeneous clip-space
 * 
 * Returns:
 * x/w, y/w, z/w, 1/w
 */
inline vec4 pdiv(vec4 p) {
	float one_over_w;
	_mm_store_ss(&one_over_w, _mm_rcp_ss(_mm_set_ss(p.w)));
	auto bias_z = 0.5f * (p.z * one_over_w) + 0.5f;
	//return vec4{ p.x*one_over_w, p.y*one_over_w, p.z*one_over_w, one_over_w }; }
	return vec4{ p.x*one_over_w, p.y*one_over_w, bias_z, one_over_w }; }
	//float a = 1.0f / p.w;
	//return p * a; }


inline bool almost_equal(const float a, const float b) {
	return fabs(a - b) < 0.0001; }

inline bool almost_equal(const vec2& a, const vec2& b) {
	return (almost_equal(a.x, b.x) &&
	        almost_equal(a.y, b.y)); }

inline bool almost_equal(const vec4& a, const vec4& b) {
	return (
		almost_equal(a.x, b.x) &&
		almost_equal(a.y, b.y) &&
		almost_equal(a.z, b.z) &&
		almost_equal(a.w, b.w)
	); }

inline bool almost_equal(const vec3& a, const vec3& b) {
	return (
		almost_equal(a.x, b.x) &&
		almost_equal(a.y, b.y) &&
		almost_equal(a.z, b.z)
	); }


inline uint32_t pack_udec3(float x, float y, float z) {
	auto ix = uint32_t( (x + 1.0f) * 511.5f );
	auto iy = uint32_t( (y + 1.0f) * 511.5f );
	auto iz = uint32_t( (z + 1.0f) * 511.5f );
	return (ix & 0x3FF) | ((iy & 0x3FF) << 10) | ((iz & 0x3FF) << 20); }


inline auto unpack_udec3(uint32_t N) {
	auto x = float( (N >> 22) / ((1 << 10) - 1) );
	auto y = float( ((N >> 12) & ((1 << 10) - 1)) / ((1 << 10) - 1) );
	auto z = float( ((N >> 2)  & ((1 << 10) - 1)) / ((1 << 10) - 1) );
	return std::tuple{x, y, z}; }

}  // close package namespace
}  // close enterprise namespace


std::ostream& operator<<(std::ostream& os, const rqdq::rmlv::vec4& v);
std::ostream& operator<<(std::ostream& os, const rqdq::rmlv::vec3& v);
std::ostream& operator<<(std::ostream& os, const rqdq::rmlv::ivec2& v);
