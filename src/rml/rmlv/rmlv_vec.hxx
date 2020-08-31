#pragma once
#include <algorithm>
#include <iostream>

namespace rqdq {
namespace rmlv {

struct vec2 {

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4201)
#endif
	union {
		struct { float x, y; };
		float arr[2]; };
#ifdef _MSC_VER
#pragma warning(pop)
#endif

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

	bool operator==(vec2 rhs) const { return x==rhs.x && y==rhs.y; } };


struct alignas(4) vec3 {

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4201)
#endif
	union {
		struct { float x, y, z; };
		float arr[3]; };
#ifdef _MSC_VER
#pragma warning(pop)
#endif

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

	vec2 xy() const { return { x, y }; } };


struct alignas(16) vec4 {

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4201)
#endif
	union {
		struct { float x, y, z, w; };
		float arr[4]; };
#ifdef _MSC_VER
#pragma warning(pop)
#endif

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

	vec2 xy() const { return { x, y }; }
	vec3 xyz() const { return { x, y, z }; } };


struct alignas(8) ivec2 {

	int x, y;

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
	ivec2 operator-(int b) const { return ivec2(x - b, y - b); }};


struct alignas(16) ivec4 {

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4201)
#endif
	union {
		struct { int x, y, z, w; };
		int32_t si[4]; };
#ifdef _MSC_VER
#pragma warning(pop)
#endif

	ivec4() = default;
	constexpr ivec4(const int a, const int b, const int c, const int d) noexcept :x(a), y(b), z(c), w(d) {}
	constexpr ivec4(const int a) noexcept : x(a), y(a), z(a), w(a) {}

	ivec4& operator+=(const ivec4& b) { x += b.x; y += b.y; z += b.z; return *this; }
	ivec4& operator-=(const ivec4& b) { x -= b.x; y -= b.y; z -= b.z; return *this; }
	ivec4& operator*=(const ivec4& b) { x *= b.x; y *= b.y; z *= b.z; return *this; } };


inline
auto operator==(const ivec2& a, const ivec2& b) -> bool {
	return (a.x == b.x && a.y == b.y); }


inline
auto operator!=(const ivec2& a, const ivec2& b) -> bool {
	return (a.x != b.x || a.y != b.y); }

// float * vec
inline vec2 operator*(float a, vec2 b) { return{ a*b.x, a*b.y }; }
inline vec3 operator*(float a, vec3 b) { return{ a*b.x, a*b.y, a*b.z }; }
inline vec4 operator*(float a, vec4 b) { return{ a*b.x, a*b.y, a*b.z, a*b.w }; } 


// unary minus
inline vec2 operator-(vec2 a) { return{ -a.x, -a.y }; }
inline vec3 operator-(vec3 a) { return{ -a.x, -a.y, -a.z }; }
inline vec4 operator-(vec4 a) { return{ -a.x, -a.y, -a.z, -a.w }; }


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
