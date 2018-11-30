#pragma once
#include "src/rml/rmlv/rmlv_soa.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"

#include <cmath>
#include <cstring>
#include <cassert>
#include <intrin.h>


namespace rqdq {
namespace rmlm {


struct alignas(64) mat4 {

	inline mat4() = default;
	inline mat4(
		const float a00, const float a01, const float a02, const float a03,
		const float a10, const float a11, const float a12, const float a13,
		const float a20, const float a21, const float a22, const float a23,
		const float a30, const float a31, const float a32, const float a33) noexcept :ff({ {
				a00, a10, a20, a30, a01, a11, a21, a31, a02, a12, a22, a32, a03, a13, a23, a33} }) {}

	inline mat4(const std::array<float, 16>& src) {
		std::copy(src.begin(), src.end(), ff.begin()); }

	void print();

	static inline mat4 ident() {
		return mat4{
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1 }; }

	static inline mat4 scale(const rmlv::vec3& a) {
		return mat4{
			a.x, 0, 0, 0,
			0, a.y, 0, 0,
			0, 0, a.z, 0,
			0, 0, 0, 1 }; }

	static inline mat4 scale(const rmlv::vec4& a) {
		return mat4{
			a.x, 0, 0, 0,
			0, a.y, 0, 0,
			0, 0, a.z, 0,
			0, 0, 0, 1 }; }

	static inline mat4 scale(const float x, const float y, const float z) {
		return mat4{
			x, 0, 0, 0,
			0, y, 0, 0,
			0, 0, z, 0,
			0, 0, 0, 1 }; }

	static inline mat4 translate(const rmlv::vec4& a) {
		assert(rmlv::almost_equal(a.w, 1.0f));
		return mat4{
			1, 0, 0, a.x,
			0, 1, 0, a.y,
			0, 0, 1, a.z,
			0, 0, 0, 1 }; }

	static inline mat4 translate(const rmlv::vec3& a) {
		return mat4{
			1, 0, 0, a.x,
			0, 1, 0, a.y,
			0, 0, 1, a.z,
			0, 0, 0, 1 }; }

	static inline mat4 translate(const float x, const float y, const float z) {
		return mat4{
			1, 0, 0, x,
			0, 1, 0, y,
			0, 0, 1, z,
			0, 0, 0, 1 }; }

	static inline mat4 rotate(const float theta, const float x, const float y, const float z) {
		/*glRotate() matrix*/
		const float s = sin(theta);
		const float c = cos(theta);
		const float t = 1.0f - c;

		const float tx = t * x;
		const float ty = t * y;
		const float tz = t * z;

		const float sz = s * z;
		const float sy = s * y;
		const float sx = s * x;

		return mat4{
			tx*x + c,  tx*y - sz, tx*z + sy, 0,
			tx*y + sz, ty*y + c,  ty*z - sx, 0,
			tx*z - sy, ty*z + sx, tz*z + c,  0,
			0,         0,         0,         1 }; }

	union {
		float cr[4][4]; // col,row
		std::array<float, 16> ff; };
	};


inline mat4 transpose(const mat4& m) {
	return mat4{
		m.ff[ 0], m.ff[ 1], m.ff[ 2], m.ff[ 3],
		m.ff[ 4], m.ff[ 5], m.ff[ 6], m.ff[ 7],
		m.ff[ 8], m.ff[ 9], m.ff[10], m.ff[11],
		m.ff[12], m.ff[13], m.ff[14], m.ff[15] }; }


mat4 inverse(const mat4& m);


inline rmlv::vec4 operator*(const mat4 a, const rmlv::vec4 b) {
	return rmlv::vec4{
		a.ff[0]*b.x + a.ff[4]*b.y + a.ff[ 8]*b.z + a.ff[12]*b.w,
		a.ff[1]*b.x + a.ff[5]*b.y + a.ff[ 9]*b.z + a.ff[13]*b.w,
		a.ff[2]*b.x + a.ff[6]*b.y + a.ff[10]*b.z + a.ff[14]*b.w,
		a.ff[3]*b.x + a.ff[7]*b.y + a.ff[11]*b.z + a.ff[15]*b.w
		}; }


inline mat4 operator*(const mat4& lhs, const mat4& rhs) {
	mat4 r;
	for (int row = 0; row < 4; row++) {
		for (int col = 0; col < 4; col++) {
			float ax = 0;
			for (int n = 0; n < 4; n++) {
				ax += lhs.cr[n][row] * rhs.cr[col][n]; }
			r.cr[col][row] = ax; }}
	return r; }


inline rmlv::vec4 mul_w1(const mat4& a, const rmlv::vec4& b) {
	assert(rmlv::almost_equal(b.w, 1.0f));
	return rmlv::vec4{
		a.ff[0]*b.x + a.ff[4]*b.y + a.ff[ 8]*b.z + a.ff[12],
		a.ff[1]*b.x + a.ff[5]*b.y + a.ff[ 9]*b.z + a.ff[13],
		a.ff[2]*b.x + a.ff[6]*b.y + a.ff[10]*b.z + a.ff[14],
		a.ff[3]*b.x + a.ff[7]*b.y + a.ff[11]*b.z + a.ff[15]
		}; }


inline rmlv::vec4 mul_w1(const mat4& a, const rmlv::vec3& b) {
	return rmlv::vec4{
		a.ff[0]*b.x + a.ff[4]*b.y + a.ff[ 8]*b.z + a.ff[12],
		a.ff[1]*b.x + a.ff[5]*b.y + a.ff[ 9]*b.z + a.ff[13],
		a.ff[2]*b.x + a.ff[6]*b.y + a.ff[10]*b.z + a.ff[14],
		a.ff[3]*b.x + a.ff[7]*b.y + a.ff[11]*b.z + a.ff[15]
		}; }


inline rmlv::vec4 mul_w0(const mat4& a, const rmlv::vec4& b) {
	assert(rmlv::almost_equal(b.w, 0.0f));
	return rmlv::vec4{
		a.ff[0]*b.x + a.ff[4]*b.y + a.ff[ 8]*b.z,
		a.ff[1]*b.x + a.ff[5]*b.y + a.ff[ 9]*b.z,
		a.ff[2]*b.x + a.ff[6]*b.y + a.ff[10]*b.z,
		a.ff[3]*b.x + a.ff[7]*b.y + a.ff[11]*b.z
		}; }


inline rmlv::vec4 mul_w0(const mat4& a, const rmlv::vec3& b) {
	return rmlv::vec4{
		a.ff[0]*b.x + a.ff[4]*b.y + a.ff[ 8]*b.z,
		a.ff[1]*b.x + a.ff[5]*b.y + a.ff[ 9]*b.z,
		a.ff[2]*b.x + a.ff[6]*b.y + a.ff[10]*b.z,
		a.ff[3]*b.x + a.ff[7]*b.y + a.ff[11]*b.z
		}; }


const int maxStackDepth = 16;


class Mat4Stack {
public:
	Mat4Stack() :d_sp(0) {}
	void push() {
		assert(d_sp + 1 < maxStackDepth);
		d_stack[d_sp + 1] = d_stack[d_sp];
		d_sp += 1; }
	void pop() {
		assert(d_sp > 0);
		d_sp--; }
	const mat4& top() const {
		return d_stack[d_sp]; }
	void clear() {
		d_sp = 0; }
	void mul(const mat4& m) {
		d_stack[d_sp] = d_stack[d_sp] * m; }
	void load(const mat4& m) {
		d_stack[d_sp] = m; }
	void reset() {
		d_sp = 0;
		load(mat4::ident()); }
private:
	std::array<mat4, maxStackDepth> d_stack;
	int d_sp; };


struct qmat4 {
	qmat4() = default;
	qmat4(const mat4& m) :f({ {
		/*these are transposed, the indices match openGL*/
		rmlv::mvec4f{m.ff[0x00]}, rmlv::mvec4f{m.ff[0x01]}, rmlv::mvec4f{m.ff[0x02]}, rmlv::mvec4f{m.ff[0x03]},
		rmlv::mvec4f{m.ff[0x04]}, rmlv::mvec4f{m.ff[0x05]}, rmlv::mvec4f{m.ff[0x06]}, rmlv::mvec4f{m.ff[0x07]},
		rmlv::mvec4f{m.ff[0x08]}, rmlv::mvec4f{m.ff[0x09]}, rmlv::mvec4f{m.ff[0x0a]}, rmlv::mvec4f{m.ff[0x0b]},
		rmlv::mvec4f{m.ff[0x0c]}, rmlv::mvec4f{m.ff[0x0d]}, rmlv::mvec4f{m.ff[0x0e]}, rmlv::mvec4f{m.ff[0x0f]} } }) {}

	std::array<rmlv::mvec4f, 16> f;
	};


inline rmlv::qfloat4 mul_w1(const qmat4& a, const rmlv::qfloat3& b) {
	return rmlv::qfloat4{
		a.f[0]*b.x + a.f[4]*b.y + a.f[ 8]*b.z + a.f[12],
		a.f[1]*b.x + a.f[5]*b.y + a.f[ 9]*b.z + a.f[13],
		a.f[2]*b.x + a.f[6]*b.y + a.f[10]*b.z + a.f[14],
		a.f[3]*b.x + a.f[7]*b.y + a.f[11]*b.z + a.f[15]
		}; }


inline rmlv::qfloat4 mul(const qmat4& a, const rmlv::qfloat4& b) {
	return rmlv::qfloat4{
		a.f[0]*b.x + a.f[4]*b.y + a.f[ 8]*b.z + a.f[12]*b.w,
		a.f[1]*b.x + a.f[5]*b.y + a.f[ 9]*b.z + a.f[13]*b.w,
		a.f[2]*b.x + a.f[6]*b.y + a.f[10]*b.z + a.f[14]*b.w,
		a.f[3]*b.x + a.f[7]*b.y + a.f[11]*b.z + a.f[15]*b.w
		}; }


inline rmlv::qfloat3 mul_w0(const qmat4& a, const rmlv::qfloat3& b) {
	return rmlv::qfloat3{
		a.f[0]*b.x + a.f[4]*b.y + a.f[ 8]*b.z,
		a.f[1]*b.x + a.f[5]*b.y + a.f[ 9]*b.z,
		a.f[2]*b.x + a.f[6]*b.y + a.f[10]*b.z
		}; }


}  // close package namespace
}  // close enterprise namespace
