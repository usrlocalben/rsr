#include "src/rml/rmlm/rmlm_mat4.hxx"

#include <cassert>
#include <iostream>

#include "src/rml/rmlv/rmlv_vec.hxx"

#include "3rdparty/fmt/include/fmt/printf.h"

namespace rqdq {
namespace rmlm {

using rmlv::vec3;
using rmlv::vec4;

void mat4::print() {
	fmt::printf("[% 11.4f,% 11.4f,% 11.4f,% 11.4f]\n", cr[0][0], cr[1][0], cr[2][0], cr[3][0]);
	fmt::printf("[% 11.4f,% 11.4f,% 11.4f,% 11.4f]\n", cr[0][1], cr[1][1], cr[2][1], cr[3][1]);
	fmt::printf("[% 11.4f,% 11.4f,% 11.4f,% 11.4f]\n", cr[0][2], cr[1][2], cr[2][2], cr[3][2]);
	fmt::printf("[% 11.4f,% 11.4f,% 11.4f,% 11.4f]\n", cr[0][3], cr[1][3], cr[2][3], cr[3][3]); }


mat4 inverse(const mat4 src) {
	/*compute the inverse of the mat4 src

	the last remaining zed3d bits ;)
	*/
	mat4 dst;
	auto& inv = dst.ff;
	auto& m = src.ff;

	inv[0] = (
		m[ 5] * m[10] * m[15] -
		m[ 5] * m[11] * m[14] -
		m[ 9] * m[ 6] * m[15] +
		m[ 9] * m[ 7] * m[14] +
		m[13] * m[ 6] * m[11] -
		m[13] * m[ 7] * m[10]);

	inv[4] = (
		-m[4] * m[10] * m[15] +
		m[ 4] * m[11] * m[14] +
		m[ 8] * m[ 6] * m[15] -
		m[ 8] * m[ 7] * m[14] -
		m[12] * m[ 6] * m[11] +
		m[12] * m[ 7] * m[10]);

	inv[8] = (
		m[ 4] * m[ 9] * m[15] -
		m[ 4] * m[11] * m[13] -
		m[ 8] * m[ 5] * m[15] +
		m[ 8] * m[ 7] * m[13] +
		m[12] * m[ 5] * m[11] -
		m[12] * m[ 7] * m[ 9]);

	inv[12] = (
		-m[4] * m[ 9] * m[14] +
		m[ 4] * m[10] * m[13] +
		m[ 8] * m[ 5] * m[14] -
		m[ 8] * m[ 6] * m[13] -
		m[12] * m[ 5] * m[10] +
		m[12] * m[ 6] * m[ 9]);

	inv[1] = (
		-m[1] * m[10] * m[15] +
		m[ 1] * m[11] * m[14] +
		m[ 9] * m[ 2] * m[15] -
		m[ 9] * m[ 3] * m[14] -
		m[13] * m[ 2] * m[11] +
		m[13] * m[ 3] * m[10]);

	inv[5] = (
		m[ 0] * m[10] * m[15] -
		m[ 0] * m[11] * m[14] -
		m[ 8] * m[ 2] * m[15] +
		m[ 8] * m[ 3] * m[14] +
		m[12] * m[ 2] * m[11] -
		m[12] * m[ 3] * m[10]);

	inv[9] = (
		-m[0] * m[ 9] * m[15] +
		m[ 0] * m[11] * m[13] +
		m[ 8] * m[ 1] * m[15] -
		m[ 8] * m[ 3] * m[13] -
		m[12] * m[ 1] * m[11] +
		m[12] * m[ 3] * m[ 9]);

	inv[13] = (
		m[ 0] * m[ 9] * m[14] -
		m[ 0] * m[10] * m[13] -
		m[ 8] * m[ 1] * m[14] +
		m[ 8] * m[ 2] * m[13] +
		m[12] * m[ 1] * m[10] -
		m[12] * m[ 2] * m[ 9]);

	inv[2] = (
		m[ 1] * m[ 6] * m[15] -
		m[ 1] * m[ 7] * m[14] -
		m[ 5] * m[ 2] * m[15] +
		m[ 5] * m[ 3] * m[14] +
		m[13] * m[ 2] * m[ 7] -
		m[13] * m[ 3] * m[ 6]);

	inv[6] = (
		-m[0] * m[6] * m[15] +
		m[ 0] * m[7] * m[14] +
		m[ 4] * m[2] * m[15] -
		m[ 4] * m[3] * m[14] -
		m[12] * m[2] * m[ 7] +
		m[12] * m[3] * m[ 6]);

	inv[10] = (
		m[ 0] * m[ 5] * m[15] -
		m[ 0] * m[ 7] * m[13] -
		m[ 4] * m[ 1] * m[15] +
		m[ 4] * m[ 3] * m[13] +
		m[12] * m[ 1] * m[ 7] -
		m[12] * m[ 3] * m[ 5]);

	inv[14] = (
		-m[0] * m[ 5] * m[14] +
		m[ 0] * m[ 6] * m[13] +
		m[ 4] * m[ 1] * m[14] -
		m[ 4] * m[ 2] * m[13] -
		m[12] * m[ 1] * m[ 6] +
		m[12] * m[ 2] * m[ 5]);

	inv[3] = (
		-m[1] * m[ 6] * m[11] +
		m[ 1] * m[ 7] * m[10] +
		m[ 5] * m[ 2] * m[11] -
		m[ 5] * m[ 3] * m[10] -
		m[ 9] * m[ 2] * m[ 7] +
		m[ 9] * m[ 3] * m[ 6]);

	inv[7] = (
		m[ 0] * m[ 6] * m[11] -
		m[ 0] * m[ 7] * m[10] -
		m[ 4] * m[ 2] * m[11] +
		m[ 4] * m[ 3] * m[10] +
		m[ 8] * m[ 2] * m[ 7] -
		m[ 8] * m[ 3] * m[ 6]);

	inv[11] = (
		-m[0] * m[ 5] * m[11] +
		m[ 0] * m[ 7] * m[ 9] +
		m[ 4] * m[ 1] * m[11] -
		m[ 4] * m[ 3] * m[ 9] -
		m[ 8] * m[ 1] * m[ 7] +
		m[ 8] * m[ 3] * m[ 5]);

	inv[15] = (
		m[ 0] * m[ 5] * m[10] -
		m[ 0] * m[ 6] * m[ 9] -
		m[ 4] * m[ 1] * m[10] +
		m[ 4] * m[ 2] * m[ 9] +
		m[ 8] * m[ 1] * m[ 6] -
		m[ 8] * m[ 2] * m[ 5]);

	const auto det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];
	assert(det != 0);
	const auto invdet = 1.0F / det;

	for (auto& val : dst.ff) {
		val *= invdet; }

	return dst; }


}  // namespace rmlm
}  // namespace rqdq
