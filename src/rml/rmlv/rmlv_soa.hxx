/**
 * SoA 2x2 vector oprations for shaders
 */
#pragma once
#include <rmlv_mvec4.hxx>
#include <rmlv_vec.hxx>

#include <pmmintrin.h>


namespace rqdq {
namespace rmlv {


typedef mvec4f qfloat;


struct qfloat2 {
	/*SoA float2*/

	inline qfloat2() = default;
	inline qfloat2(const vec2& a) noexcept :x(a.x), y(a.y) {}
	inline qfloat2(const mvec4f& a) noexcept :x(a.xxxx()), y(a.yyyy()) {}
	inline qfloat2(const qfloat2& a) noexcept :x(a.x), y(a.y) {}
	inline qfloat2(const float x, const float y) noexcept :x(x), y(y) {}
	inline qfloat2(const mvec4f& x, const mvec4f& y) noexcept :x(x), y(y) {}

	inline qfloat2 operator+(const qfloat2& b) const { return{ v[0] + b.v[0], v[1] + b.v[1] }; }
	inline qfloat2 operator-(const qfloat2& b) const { return{ v[0] - b.v[0], v[1] - b.v[1] }; }
	inline qfloat2 operator*(const qfloat2& b) const { return{ v[0] * b.v[0], v[1] * b.v[1] }; }
	inline qfloat2 operator/(const qfloat2& b) const { return{ v[0] / b.v[0], v[1] / b.v[1] }; }

	inline qfloat2 operator+(const qfloat& b) const { return{ v[0] + b, v[1] + b }; }
	inline qfloat2 operator-(const qfloat& b) const { return{ v[0] - b, v[1] - b }; }
	inline qfloat2 operator*(const qfloat& b) const { return{ v[0] * b, v[1] * b }; }
	inline qfloat2 operator/(const qfloat& b) const { return{ v[0] / b, v[1] / b }; }

	inline qfloat2& operator+=(const qfloat2& rhs) { x += rhs.x;  y += rhs.y;  return *this; }
	inline qfloat2& operator-=(const qfloat2& rhs) { x -= rhs.x;  y -= rhs.y;  return *this; }
	inline qfloat2& operator*=(const qfloat2& rhs) { x *= rhs.x;  y *= rhs.y;  return *this; }
	inline qfloat2& operator/=(const qfloat2& rhs) { x /= rhs.x;  y /= rhs.y;  return *this; }

	inline vec2 lane(const int li) {
		return vec2{ x.lane[li], y.lane[li] }; }

	inline void copyTo(vec2* dst) {
		__m128 v0 = _mm_unpacklo_ps(x.v, y.v);  // x0 y0 x1 y1
		__m128 v1 = _mm_unpackhi_ps(x.v, y.v);  // x2 y2 x3 y3
		_mm_store_ps(reinterpret_cast<float*>(&dst[0]), v0);
		_mm_store_ps(reinterpret_cast<float*>(&dst[2]), v1); }

	union {
		struct { mvec4f x, y; };
		struct { mvec4f s, t; };
		//struct { mvec4f u, v; };
		mvec4f v[2];
		};
	};


inline qfloat length(const qfloat2 &a) {
	return sqrt(a.x*a.x + a.y*a.y); }


struct qfloat3 {
	/*SoA float3*/

	inline qfloat3() = default;
	inline qfloat3(const vec3& a) noexcept : x(a.x), y(a.y), z(a.z) {}
	inline qfloat3(const mvec4f& a) noexcept :x(a.xxxx()), y(a.yyyy()), z(a.zzzz()) {}
	inline qfloat3(const float x, const float y, const float z) noexcept :x(x), y(y), z(z) {}
	inline qfloat3(const mvec4f& x, const mvec4f& y, const mvec4f& z) noexcept :x(x), y(y), z(z) {}

	inline qfloat3 operator+(const qfloat3& b) const { return{v[0]+b.v[0], v[1]+b.v[1], v[2]+b.v[2] }; }
	inline qfloat3 operator-(const qfloat3& b) const { return{v[0]-b.v[0], v[1]-b.v[1], v[2]-b.v[2] }; }
	inline qfloat3 operator*(const qfloat3& b) const { return{v[0]*b.v[0], v[1]*b.v[1], v[2]*b.v[2] }; }
	inline qfloat3 operator/(const qfloat3& b) const { return{v[0]/b.v[0], v[1]/b.v[1], v[2]/b.v[2] }; }

	inline qfloat3 operator+(const qfloat& b) const { return{v[0]+b, v[1]+b, v[2]+b }; }
	inline qfloat3 operator-(const qfloat& b) const { return{v[0]-b, v[1]-b, v[2]-b }; }
	inline qfloat3 operator*(const qfloat& b) const { return{v[0]*b, v[1]*b, v[2]*b }; }
	inline qfloat3 operator/(const qfloat& b) const { return{v[0]/b, v[1]/b, v[2]/b }; }

	inline qfloat3& operator+=(const qfloat& rhs) { x+=rhs; y+=rhs; z+=rhs; return *this; }
	inline qfloat3& operator-=(const qfloat& rhs) { x-=rhs; y-=rhs; z-=rhs; return *this; }
	inline qfloat3& operator*=(const qfloat& rhs) { x*=rhs; y*=rhs; z*=rhs; return *this; }
	inline qfloat3& operator/=(const qfloat& rhs) { x/=rhs; y/=rhs; z/=rhs; return *this; }

	inline qfloat3& operator+=(const qfloat3& rhs) { x+=rhs.x; y+=rhs.y; z+=rhs.z; return *this; }
	inline qfloat3& operator-=(const qfloat3& rhs) { x-=rhs.x; y-=rhs.y; z-=rhs.z; return *this; }
	inline qfloat3& operator*=(const qfloat3& rhs) { x*=rhs.x; y*=rhs.y; z*=rhs.z; return *this; }
	inline qfloat3& operator/=(const qfloat3& rhs) { x/=rhs.x; y/=rhs.y; z/=rhs.z; return *this; }

	inline vec3 lane(const int li) {
		return vec3{ x.lane[li], y.lane[li], z.lane[li] }; }

	union {
		struct { mvec4f x, y, z; };
		struct { mvec4f r, g, b; };
		mvec4f v[3];
		};
	};


inline qfloat3 sin(const qfloat3& a) {
	return qfloat3{ sin(a.x), sin(a.y), sin(a.z) }; }


inline qfloat3 fract(const qfloat3& a) {
	return qfloat3{ fract(a.x), fract(a.y), fract(a.z) }; }


inline qfloat3 pow(const qfloat3& a, const qfloat3& b) {
	return qfloat3{ pow(a.x, b.x), pow(a.y, b.y), pow(a.z, b.z) }; }


inline qfloat3 operator*(const float& lhs, const qfloat3& rhs) {
	return qfloat3{ lhs * rhs.x, lhs * rhs.y, lhs * rhs.z }; }


inline qfloat3 operator*(const qfloat& lhs, const qfloat3& rhs) {
	return qfloat3{ lhs * rhs.x, lhs * rhs.y, lhs * rhs.z }; }


inline qfloat length(const qfloat3 &a) {
	return sqrt(a.x*a.x + a.y*a.y + a.z*a.z); }


inline qfloat3 sqrt(const qfloat3 &a) {
	return qfloat3{ sqrt(a.x), sqrt(a.y), sqrt(a.z) }; }


inline qfloat dot(const qfloat3& a, const qfloat3& b) {
	return a.x*b.x + a.y*b.y + a.z*b.z; }


inline qfloat3 normalize(const qfloat3& a) {
	mvec4f scale = rsqrt(dot(a, a));
	return qfloat3{
		a.x * scale,
		a.y * scale,
		a.z * scale }; }


inline qfloat3 lerp(const qfloat3& a, const qfloat3& b, const qfloat& t) {
	return qfloat3{ lerp(a.x, b.x, t), lerp(a.y, b.y, t), lerp(a.z, b.z, t) }; }

inline qfloat3 reflect(const qfloat3& i, const qfloat3& n) {
	return i - qfloat{2.0f} * dot(n, i) * n; }




struct qfloat4 {

	inline qfloat4() = default;
	inline qfloat4(const vec3& a, const float w) noexcept :x(a.x), y(a.y), z(a.z), w(w) {}
	inline qfloat4(const vec4& a) noexcept :x(a.x), y(a.y), z(a.z), w(a.w) {}
	inline qfloat4(const mvec4f& a) noexcept :x(a.xxxx()), y(a.yyyy()), z(a.zzzz()), w(a.wwww()) {}
	inline qfloat4(const qfloat3& a, const mvec4f& w) noexcept :x(a.x), y(a.y), z(a.z), w(w) {}
	inline qfloat4(const float x, const float y, const float z, const float w) noexcept :x(x), y(y), z(z), w(w) {}
	inline qfloat4(mvec4f x, mvec4f y, mvec4f z, mvec4f w) noexcept :x(x), y(y), z(z), w(w) {}

	inline qfloat4& operator+=(const qfloat4& rhs) { x += rhs.x; y += rhs.y; z += rhs.z; w += rhs.w; return *this; }

	inline qfloat3 xyz() const {
		return qfloat3{ x, y, z }; }

	inline vec4 lane(const int li) {
		return vec4{ x.lane[li], y.lane[li], z.lane[li], w.lane[li] }; }

	inline void setLane(const int li, const vec4 a) {
		x.lane[li] = a.x;
		y.lane[li] = a.y;
		z.lane[li] = a.z;
		w.lane[li] = a.w; }

	/*
	 * destructive store
	 */
	inline void moveTo(vec4* dst) {
		_MM_TRANSPOSE4_PS(x.v, y.v, z.v, w.v);
		_mm_store_ps(reinterpret_cast<float*>(&dst[0]), x.v);
		_mm_store_ps(reinterpret_cast<float*>(&dst[1]), y.v);
		_mm_store_ps(reinterpret_cast<float*>(&dst[2]), z.v);
		_mm_store_ps(reinterpret_cast<float*>(&dst[3]), w.v); }

	union {
		struct { mvec4f x, y, z, w; };
		struct { mvec4f r, g, b, a; };
		mvec4f v[4];
		};
	};


inline void load_interleaved_lut(const float *bp, mvec4i ofs, qfloat4& out) {
	load_interleaved_lut(bp, ofs, out.x, out.y, out.z, out.w); }

inline mvec4f mix(const mvec4f& a, const mvec4f& b, const mvec4f& t) {
	return a*(mvec4f(1.0f) - t) + b*t; }

inline qfloat dot(const qfloat4& a, const qfloat4& b) {
	return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w; }

inline qfloat4 operator*(const qfloat4& lhs, const mvec4f& rhs) {
	return qfloat4{ lhs.x * rhs, lhs.y * rhs, lhs.z * rhs, lhs.w * rhs }; }

inline qfloat4 normalize(const qfloat4& a) {
	mvec4f scale = rsqrt(dot(a, a));
	return qfloat4{
		a.x * scale,
		a.y * scale,
		a.z * scale,
		a.w * scale}; }

inline qfloat4 lerp(const qfloat4& a, const qfloat4& b, const qfloat& t) {
	return qfloat4{ lerp(a.x, b.x, t), lerp(a.y, b.y, t), lerp(a.z, b.z, t), lerp(a.w, b.w, t) }; }


}  // close package namespace
}  // close enterprise namespace
