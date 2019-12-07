#pragma once
#include "src/rml/rmlv/rmlv_vec.hxx"

#include <array>
#include <cmath>

namespace rqdq {
namespace rmlm {

struct alignas(64) mat4 {
	union {
		float cr[4][4]; // col,row
		std::array<float, 16> ff; };

	mat4() = default;
	explicit mat4(float scale) noexcept;
	mat4(float a00, float a01, float a02, float a03,
	     float a10, float a11, float a12, float a13,
	     float a20, float a21, float a22, float a23,
	     float a30, float a31, float a32, float a33) noexcept;
	explicit mat4(std::array<float, 16> src) noexcept;

	static auto identity() -> mat4;

	static auto scale(float) -> mat4;
	static auto scale(float x, float y, float z=1) -> mat4;
	static auto scale(rmlv::vec2) -> mat4;
	static auto scale(rmlv::vec3) -> mat4;
	static auto scale(rmlv::vec4) -> mat4;

	static auto translate(float x, float y, float z=0) -> mat4;
	static auto translate(rmlv::vec2) -> mat4;
	static auto translate(rmlv::vec3) -> mat4;
	static auto translate(rmlv::vec4) -> mat4;

	static auto rotate(float theta, float x, float y, float z) -> mat4;
	static auto rotate(float theta, rmlv::vec3) -> mat4;
	static auto rotate(float theta) -> mat4; };
	

auto print(std::ostream&, const mat4&) -> void;
auto inverse(mat4) -> mat4;
auto transpose(mat4) -> mat4;

auto operator*(const mat4&, const rmlv::vec4&) -> rmlv::vec4;
auto operator*(const mat4&, const mat4&) -> mat4;


inline
mat4::mat4(float s) noexcept :
	ff({ { s, 0, 0, 0,
	       0, s, 0, 0,
	       0, 0, s, 0,
	       0, 0, 0, 1 } }) {}


inline
mat4::mat4(float a00, float a01, float a02, float a03,
           float a10, float a11, float a12, float a13,
           float a20, float a21, float a22, float a23,
           float a30, float a31, float a32, float a33) noexcept :
	ff({ { a00, a10, a20, a30,
	       a01, a11, a21, a31,
	       a02, a12, a22, a32,
	       a03, a13, a23, a33 } }) {}


inline
mat4::mat4(std::array<float, 16> src) noexcept :
	ff(std::move(src)) {}


inline
auto mat4::scale(float x, float y, float z) -> mat4 {
	return {
		x, 0, 0, 0,
		0, y, 0, 0,
		0, 0, z, 0,
		0, 0, 0, 1 }; }


inline
auto mat4::scale(float s) -> mat4 {
	return mat4::scale(s, s, s); }


inline
auto mat4::scale(rmlv::vec2 v) -> mat4 {
	return mat4::scale(v.x, v.y, 1); }


inline
auto mat4::scale(rmlv::vec3 v) -> mat4 {
	return mat4::scale(v.x, v.y, v.z); }


inline
auto mat4::scale(rmlv::vec4 v) -> mat4 {
	return mat4::scale(v.x, v.y, v.z); }


inline
auto mat4::identity() -> mat4 {
	return mat4::scale(1); }


inline
auto mat4::translate(float x, float y, float z) -> mat4 {
	return mat4{
		1, 0, 0, x,
		0, 1, 0, y,
		0, 0, 1, z,
		0, 0, 0, 1 }; }


inline
auto mat4::translate(rmlv::vec2 v) -> mat4 {
	return mat4::translate(v.x, v.y, 1); }


inline
auto mat4::translate(rmlv::vec3 v) -> mat4 {
	return mat4::translate(v.x, v.y, v.z); }


inline
auto mat4::translate(rmlv::vec4 v) -> mat4 {
	return mat4::translate(v.x, v.y, v.z); }


inline
auto mat4::rotate(float theta, float x, float y, float z) -> mat4 {
	/*glRotate() matrix*/
	const float s = sin(theta);
	const float c = cos(theta);
	const float t = 1.0F - c;

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


inline
auto mat4::rotate(float theta) -> mat4 {
	return mat4::rotate(theta, 0, 0, 1); }


inline
auto mat4::rotate(float theta, rmlv::vec3 a) -> mat4 {
	return mat4::rotate(theta, a.x, a.y, a.z); }


inline
auto transpose(mat4 m) -> mat4 {
	return mat4{
		m.ff[ 0], m.ff[ 1], m.ff[ 2], m.ff[ 3],
		m.ff[ 4], m.ff[ 5], m.ff[ 6], m.ff[ 7],
		m.ff[ 8], m.ff[ 9], m.ff[10], m.ff[11],
		m.ff[12], m.ff[13], m.ff[14], m.ff[15] }; }


inline
auto operator*(const mat4& a, const rmlv::vec4& b) -> rmlv::vec4 {
	return rmlv::vec4{
		a.ff[0]*b.x + a.ff[4]*b.y + a.ff[ 8]*b.z + a.ff[12]*b.w,
		a.ff[1]*b.x + a.ff[5]*b.y + a.ff[ 9]*b.z + a.ff[13]*b.w,
		a.ff[2]*b.x + a.ff[6]*b.y + a.ff[10]*b.z + a.ff[14]*b.w,
		a.ff[3]*b.x + a.ff[7]*b.y + a.ff[11]*b.z + a.ff[15]*b.w
		}; }


inline
auto operator*(const mat4& lhs, const mat4& rhs) -> mat4 {
	mat4 out;
	for (int leftRow{0}; leftRow<4; ++leftRow) {
		for (int rightCol{0}; rightCol<4; ++rightCol) {
			float ax;
			ax  = lhs.cr[0][leftRow] * rhs.cr[rightCol][0];
			ax += lhs.cr[1][leftRow] * rhs.cr[rightCol][1];
			ax += lhs.cr[2][leftRow] * rhs.cr[rightCol][2];
			ax += lhs.cr[3][leftRow] * rhs.cr[rightCol][3];
			out.cr[rightCol][leftRow] = ax; }}
	return out; }


}  // namespace rmlm
}  // namespace rqdq
