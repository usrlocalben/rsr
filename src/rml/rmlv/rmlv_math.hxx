#pragma once
#include <algorithm>
#include <cmath>
#include <tuple>

#include "src/rml/rmlv/rmlv_vec.hxx"

namespace rqdq {
namespace rmlv {

constexpr auto M_PI = 3.14159265358979323846;

template<typename T>
T Min(const T& a, const T& b, const T& c) {
	T out = a;
	if (b < out) out = b;
	if (c < out) out = c;
	return out; }

template<typename T>
T Min(const T& a, const T& b, const T& c, const T& d) {
	T out = a;
	if (b < out) out = b;
	if (c < out) out = c;
	if (d < out) out = d;
	return out; }


template<typename T>
T Max(const T& a, const T& b, const T& c) {
	T out = a;
	if (b > out) out = b;
	if (c > out) out = c;
	return out; }

template<typename T>
T Max(const T& a, const T& b, const T& c, const T& d) {
	T out = a;
	if (b > out) out = b;
	if (c > out) out = c;
	if (d > out) out = d;
	return out; }


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
	u.x[1] = static_cast<int>(b * (u.x[1] - 1072632447) + 1072632447);
	u.x[0] = 0;
	return u.d; }


// mix
/*
template<typename T>
T lerp_fast(const T& a, const T& b, const T& f) {
	return a + (f*(b - a)); }
template<typename T>
T lerp_premul(const T& a, const T& b, const T& f) {
	return (T(1) - f)*a + b; }
*/

/*template<typename T>
T lerp(const T& a, const T& b, const T& f) {
	return (1 - f)*a + f*b; }*/

inline float mix(float a, float b, float t) { return (1.0F - t)*a + t*b; }
inline vec2 mix(vec2 a, vec2 b, float t) { return (1.0F - t)*a + t*b; }
inline vec3 mix(vec3 a, vec3 b, float t) { return (1.0F - t)*a + t*b; }
inline vec4 mix(vec4 a, vec4 b, float t) { return (1.0F - t)*a + t*b; }

// area
inline int Area(ivec2 a) { return a.x * a.y; }
inline float Area(vec2 a) { return a.x * a.y; }

// dot
inline float dot(vec2 a, vec2 b) { return a.x*b.x + a.y*b.y; }
inline float dot(vec3 a, vec3 b) { return a.x*b.x + a.y*b.y + a.z*b.z; }
inline float dot(vec4 a, vec4 b) { return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w; }

// length
inline float length(vec2 a) { return sqrtf(dot(a, a)); }
inline float length(vec3 a) { return sqrtf(dot(a, a)); }
inline float length(vec4 a) { return sqrtf(dot(a, a)); }

// normalize
inline vec2 normalize(vec2 a) { return a / length(a); }
inline vec3 normalize(vec3 a) { return a / length(a); }
inline vec4 normalize(vec4 a) { return a / length(a); }

// sqrt
inline vec2 sqrt(vec2 a) { return{ sqrtf(a.x), sqrtf(a.y) }; }
inline vec3 sqrt(vec3 a) { return{ sqrtf(a.x), sqrtf(a.y), sqrtf(a.z) }; }
inline vec4 sqrt(vec4 a) { return{ sqrtf(a.x), sqrtf(a.y), sqrtf(a.z), sqrtf(a.w) }; }

// abs
inline vec2 abs(vec2 a) { return{ fabs(a.x), fabs(a.y) }; }
inline vec3 abs(vec3 a) { return{ fabs(a.x), fabs(a.y), fabs(a.z) }; }
inline vec4 abs(vec4 a) { return{ fabs(a.x), fabs(a.y), fabs(a.z), fabs(a.w) }; }

// h*
inline float hadd(vec2 a) { return a.x + a.y; }
inline float hadd(vec3 a) { return a.x + a.y + a.z; }
inline float hadd(vec4 a) { return a.x + a.y + a.z + a.w; }

inline float hmax(vec2 a) { return std::max(a.x, a.y); }
inline float hmax(vec3 a) { return Max(a.x, a.y, a.z); }
inline float hmax(vec4 a) { return Max(a.x, a.y, a.z, a.w); }

inline float hmin(vec2 a) { return std::min(a.x, a.y); }
inline float hmin(vec3 a) { return Min(a.x, a.y, a.z); }
inline float hmin(vec4 a) { return Min(a.x, a.y, a.z, a.w); }

// vmin/vmax
inline ivec2 vmin(ivec2 a, ivec2 b) { return { std::min(a.x, b.x), std::min(a.y, b.y) }; }
inline vec2 vmin(vec2 a, vec2 b) { return{ std::min(a.x, b.x), std::min(a.y, b.y) }; }
inline vec3 vmin(vec3 a, vec3 b) { return{ std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z) }; }
inline vec4 vmin(vec4 a, vec4 b) { return{ std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z), std::min(a.w, b.w) }; }

inline ivec2 vmax(ivec2 a, ivec2 b) { return { std::max(a.x, b.x), std::max(a.y, b.y) }; }
inline vec2 vmax(vec2 a, vec2 b) { return{ std::max(a.x, b.x), std::max(a.y, b.y) }; }
inline vec3 vmax(vec3 a, vec3 b) { return{ std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z) }; }
inline vec4 vmax(vec4 a, vec4 b) { return{ std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z), std::max(a.w, b.w) }; }


// cross product
inline vec3 cross(vec3 a, vec3 b) {
	return { a.y*b.z - a.z*b.y,
	         a.z*b.x - a.x*b.z,
	         a.x*b.y - a.y*b.x }; }
inline vec4 cross(vec4 a, vec4 b) {
	return{ a.y*b.z - a.z*b.y,
	        a.z*b.x - a.x*b.z,
	        a.x*b.y - a.y*b.x,
	        a.w*b.w - a.w*b.w }; }


// almost_equal
inline bool almost_equal(const float a, const float b) {
	return fabs(a - b) < 0.0001; }

inline bool almost_equal(const vec2& a, const vec2& b) {
	return (almost_equal(a.x, b.x) &&
	        almost_equal(a.y, b.y)); }

inline bool almost_equal(const vec3& a, const vec3& b) {
	return (almost_equal(a.x, b.x) &&
	        almost_equal(a.y, b.y) &&
	        almost_equal(a.z, b.z)); }

inline bool almost_equal(const vec4& a, const vec4& b) {
	return (almost_equal(a.x, b.x) &&
	        almost_equal(a.y, b.y) &&
	        almost_equal(a.z, b.z) &&
	        almost_equal(a.w, b.w)); }


inline uint32_t pack_udec3(float x, float y, float z) {
	auto ix = uint32_t( (x + 1.0F) * 511.5F );
	auto iy = uint32_t( (y + 1.0F) * 511.5F );
	auto iz = uint32_t( (z + 1.0F) * 511.5F );
	return (ix & 0x3FF) | ((iy & 0x3FF) << 10) | ((iz & 0x3FF) << 20); }

inline auto unpack_udec3(uint32_t N) {
	auto x = float( (N >> 22) / ((1 << 10) - 1) );
	auto y = float( ((N >> 12) & ((1 << 10) - 1)) / ((1 << 10) - 1) );
	auto z = float( ((N >> 2)  & ((1 << 10) - 1)) / ((1 << 10) - 1) );
	return std::tuple{x, y, z}; }


}  // close package namespace
}  // close enterprise namespace
