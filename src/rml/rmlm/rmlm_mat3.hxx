#pragma once
#include "src/rml/rmlv/rmlv_vec.hxx"

#include <array>
#include <cmath>

namespace rqdq {
namespace rmlm {

struct mat3 {
	union {
		float cr[3][3]; // col,row
		std::array<float, 16> ff; };

	mat3();
	mat3(float);
	mat3(float a00, float a01, float a02,
		 float a10, float a11, float a12,
		 float a20, float a21, float a22);

	static mat3 scale(float x, float y);
	static mat3 translate(float x, float y);
	static mat3 rotate(float t); };


inline
mat3::mat3() = default;


inline
mat3::mat3(float s) :
	ff({ {
	     s,   0,   0,
	     0,   s,   0,
	     0,   0,1.0F } }) {}


inline
mat3::mat3(float a00, float a01, float a02,
           float a10, float a11, float a12,
           float a20, float a21, float a22) :
	ff({ {
		a00, a10, a20,
		a01, a11, a21,
		a02, a12, a22 } }) {}


inline
mat3 mat3::translate(float x, float y) {
	return mat3(1, 0, x,
	            0, 1, y,
	            0, 0, 1); }


inline
mat3 mat3::rotate(float t) {
	auto st = std::sin(t);
	auto ct = std::cos(t);
	return mat3(ct,-st, 0,
	            st, ct, 0,
	             0,  0, 1); }


inline
mat3 mat3::scale(float x, float y) {
	return mat3( x, 0, 0,
	             0, y, 0,
	             0, 0, 1); }

inline
mat3 operator*(const mat3& a, const mat3& b) {
	mat3 out;
	for (int ly{0}; ly<3; ++ly) {
		for (int rx{0}; rx<3; ++rx) {
			float ax{0};
			for (int i{0}; i<3; ++i) {
				ax += a.cr[i][ly] * b.cr[rx][i]; }
			out.cr[rx][ly] = ax; }}
	return out; }


inline
rmlv::vec3 operator*(const mat3& a, const rmlv::vec3& v) {
	float x = a.cr[0][0]*v.x + a.cr[1][0]*v.y + a.cr[2][0]*v.z;
	float y = a.cr[0][1]*v.x + a.cr[1][1]*v.y + a.cr[2][1]*v.z;
	float z = a.cr[0][2]*v.x + a.cr[1][2]*v.y + a.cr[2][2]*v.z;
	return rmlv::vec3{ x, y, z }; }


}  // namespace rmlm
}  // namespace rqdq
