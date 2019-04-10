#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>

#include <xmmintrin.h>

namespace rqdq {
namespace rmlv {

constexpr auto M_PI = 3.14159265358979323846;


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


struct alignas(8) vec2 {
	vec2() = default;
	constexpr vec2(float a) noexcept :x(a), y(a) {}
	constexpr vec2(float a, float b) noexcept :x(a), y(b) {}

	vec2 operator+(vec2 rhs) const { return { x+rhs.x, y+rhs.y }; }
	vec2 operator-(vec2 rhs) const { return { x-rhs.x, y-rhs.y }; }
	vec2 operator*(vec2 rhs) const { return { x*rhs.x, y*rhs.y }; }
	vec2 operator/(vec2 rhs) const { return { x/rhs.x, y/rhs.y }; }

	vec2 operator+(float rhs) const { return { x+rhs, y+rhs }; }
	vec2 operator-(float rhs) const { return { x-rhs, y-rhs }; }
	vec2 operator*(float rhs) const { return { x*rhs, y*rhs }; }
	vec2 operator/(float rhs) const { return { x/rhs, y/rhs }; }

	vec2& operator+=(vec2 rhs) { x += rhs.x; y += rhs.y; return *this; }
	vec2& operator-=(vec2 rhs) { x -= rhs.x; y -= rhs.y; return *this; }
	vec2& operator*=(vec2 rhs) { x *= rhs.x; y *= rhs.y; return *this; }
	vec2& operator/=(vec2 rhs) { x /= rhs.x; y /= rhs.y; return *this; }

	vec2& operator+=(float rhs) { x += rhs; y += rhs; return *this; }
	vec2& operator-=(float rhs) { x -= rhs; y -= rhs; return *this; }
	vec2& operator*=(float rhs) { x *= rhs; y *= rhs; return *this; }
	vec2& operator/=(float rhs) { x /= rhs; y /= rhs; return *this; }

	bool operator==(vec2 rhs) const { return x==rhs.x && y==rhs.y; }

	union {
		struct { float x, y; };
		std::array<float, 2> arr; }; };


struct alignas(4) vec3 {
	vec3() = default;
	constexpr vec3(float a) noexcept : x(a), y(a), z(a) {}
	constexpr vec3(float a, float b, float c) noexcept : x(a), y(b), z(c) {}
	constexpr vec3(vec2 a, float z) noexcept : x(a.x), y(a.y), z(z) {}

	vec3 operator+(vec3 rhs) const { return { x+rhs.x, y+rhs.y, z+rhs.z }; }
	vec3 operator-(vec3 rhs) const { return { x-rhs.x, y-rhs.y, z-rhs.z }; }
	vec3 operator*(vec3 rhs) const { return { x*rhs.x, y*rhs.y, z*rhs.z }; }
	vec3 operator/(vec3 rhs) const { return { x/rhs.x, y/rhs.y, z/rhs.z }; }

	vec3 operator+(float rhs) const { return { x+rhs, y+rhs, z+rhs }; }
	vec3 operator-(float rhs) const { return { x-rhs, y-rhs, z-rhs }; }
	vec3 operator*(float rhs) const { return { x*rhs, y*rhs, z*rhs }; }
	vec3 operator/(float rhs) const { return { x/rhs, y/rhs, z/rhs }; }

	vec3& operator+=(vec3 rhs) { x += rhs.x; y += rhs.y; z += rhs.z; return *this; }
	vec3& operator-=(vec3 rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; return *this; }
	vec3& operator*=(vec3 rhs) { x *= rhs.x; y *= rhs.y; z *= rhs.z; return *this; }
	vec3& operator/=(vec3 rhs) { x /= rhs.x; y /= rhs.y; z /= rhs.z; return *this; }

	vec3& operator+=(float rhs) { x += rhs; y += rhs; z += rhs; return *this; }
	vec3& operator-=(float rhs) { x -= rhs; y -= rhs; z -= rhs; return *this; }
	vec3& operator*=(float rhs) { x *= rhs; y *= rhs; z *= rhs; return *this; }
	vec3& operator/=(float rhs) { x /= rhs; y /= rhs; z /= rhs; return *this; }

	bool operator==(const vec3& rhs) const {
		return x==rhs.x && y==rhs.y && z==rhs.z; }

	vec2 xy() const { return { x, y }; }

	union {
		struct { float x, y, z; };
		std::array<float, 3> arr; }; };


struct alignas(16) vec4 {
	vec4() = default;
	constexpr vec4(float a, float b, float c, float d) :x(a), y(b), z(c), w(d) {}
	constexpr vec4(float a) : x(a), y(a), z(a), w(a) {}
	constexpr vec4(vec3 a, float b) : x(a.x), y(a.y), z(a.z), w(b) {}

	vec4 operator+(vec4 rhs) const { return { x+rhs.x, y+rhs.y, z+rhs.z, w+rhs.w }; }
	vec4 operator-(vec4 rhs) const { return { x-rhs.x, y-rhs.y, z-rhs.z, w-rhs.w }; }
	vec4 operator*(vec4 rhs) const { return { x*rhs.x, y*rhs.y, z*rhs.z, w*rhs.w }; }
	vec4 operator/(vec4 rhs) const { return { x/rhs.x, y/rhs.y, z/rhs.z, w/rhs.w }; }

	vec4 operator+(float rhs) const { return { x+rhs, y+rhs, z+rhs, w+rhs }; }
	vec4 operator-(float rhs) const { return { x-rhs, y-rhs, z-rhs, w-rhs }; }
	vec4 operator*(float rhs) const { return { x*rhs, y*rhs, z*rhs, w*rhs }; }
	vec4 operator/(float rhs) const { return { x/rhs, y/rhs, z/rhs, w/rhs }; }

	vec4& operator+=(vec4 rhs) { x += rhs.x; y += rhs.y; z += rhs.z; w += rhs.w; return *this; }
	vec4& operator-=(vec4 rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; w -= rhs.w; return *this; }
	vec4& operator*=(vec4 rhs) { x *= rhs.x; y *= rhs.y; z *= rhs.z; w *= rhs.w; return *this; }
	vec4& operator/=(vec4 rhs) { x /= rhs.x; y /= rhs.y; z /= rhs.z; w /= rhs.w; return *this; }

	vec4& operator+=(float rhs) { x += rhs; y += rhs; z += rhs; w += rhs; return *this; }
	vec4& operator-=(float rhs) { x -= rhs; y -= rhs; z -= rhs; w -= rhs; return *this; }
	vec4& operator*=(float rhs) { x *= rhs; y *= rhs; z *= rhs; w *= rhs; return *this; }
	vec4& operator/=(float rhs) { x /= rhs; y /= rhs; z /= rhs; w /= rhs; return *this; }

	vec2 xy() { return { x, y }; }
	vec3 xyz() { return { x, y, z }; }

	union {
		struct { float x, y, z, w; };
		std::array<float, 3> arr; }; };


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


// float * vec
inline vec2 operator*(float a, vec2 b) { return{ a*b.x, a*b.y }; }
inline vec3 operator*(float a, vec3 b) { return{ a*b.x, a*b.y, a*b.z }; }
inline vec4 operator*(float a, vec4 b) { return{ a*b.x, a*b.y, a*b.z, a*b.w }; } 


// unary minus
inline vec2 operator-(vec2 a) { return{ -a.x, -a.y }; }
inline vec3 operator-(vec3 a) { return{ -a.x, -a.y, -a.z }; }
inline vec4 operator-(vec4 a) { return{ -a.x, -a.y, -a.z, -a.w }; }


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
inline float hmax(vec3 a) { return std::max(std::max(a.x, a.y), a.z); }
inline float hmax(vec4 a) { return std::max(std::max(std::max(a.x, a.y), a.z), a.w); }

inline float hmin(vec2 a) { return std::min(a.x, a.y); }
inline float hmin(vec3 a) { return std::min(std::min(a.x, a.y), a.z); }
inline float hmin(vec4 a) { return std::min(std::min(std::min(a.x, a.y), a.z), a.w); }

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

}  // namespace rmlv
}  // namespace rqdq

namespace std {

template <>
struct hash<rqdq::rmlv::vec2> {
	std::size_t operator()(const rqdq::rmlv::vec2& k) const {
		using std::size_t;
		using std::hash;
		return hash<float>()(k.x) ^ hash<float>()(k.y); }};

template <>
struct hash<rqdq::rmlv::vec3> {
	std::size_t operator()(const rqdq::rmlv::vec3& k) const {
		using std::size_t;
		using std::hash;
		return hash<float>()(k.x) ^ hash<float>()(k.y) ^ hash<float>()(k.z); }};

template <>
struct hash<rqdq::rmlv::vec4> {
	std::size_t operator()(const rqdq::rmlv::vec4& k) const {
		using std::size_t;
		using std::hash;
		return hash<float>()(k.x) ^ hash<float>()(k.y) ^ hash<float>()(k.z) ^ hash<float>()(k.w); }};

}  // namespace std

std::ostream& operator<<(std::ostream& os, rqdq::rmlv::vec2 v);
std::ostream& operator<<(std::ostream& os, rqdq::rmlv::vec4 v);
std::ostream& operator<<(std::ostream& os, rqdq::rmlv::vec3 v);
std::ostream& operator<<(std::ostream& os, rqdq::rmlv::ivec2 v);
