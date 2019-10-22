#pragma once
#include <xmmintrin.h>

#include "src/rml/rmlm/rmlm_mat4.hxx"
#include "src/rml/rmlv/rmlv_soa.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"

namespace rqdq {
namespace rglv {

/**
 * Perspective Divide (SIMD version)
 *
 * float operations _must_ match scalar version !
 *
 * Returns:
 *   x/w, y/w, z/w, 1/w
 */
inline rmlv::qfloat4 pdiv(rmlv::qfloat4 p) {
	__m128 invw = _mm_rcp_ps(p.w.v);
	//__m128 r1 = _mm_div_ps(_mm_set1_ps(1.0f), p.w.v);
	return{ p.x * invw, p.y * invw, p.z * invw, invw }; }

/*
 * Perspective Divide (scalar version)
 *
 * float operations _must_ match SoA version !
 */
inline rmlv::vec4 pdiv(rmlv::vec4 p) {
	float invw;
	_mm_store_ss(&invw, _mm_rcp_ss(_mm_set_ss(p.w)));
	// auto bias_z = 0.5F * (p.z * invw) + 0.5F;
	// return rmlv::vec4{ p.x*invw, p.y*invw, bias_z, invw };
	return rmlv::vec4{ p.x*invw, p.y*invw, p.z*invw, invw }; }


inline rmlv::vec3 reflect(rmlv::vec3 i, rmlv::vec3 n) {
	return i - 2.0F * dot(n, i) * n; }

inline rmlv::qfloat3 reflect(rmlv::qfloat3 i, rmlv::qfloat3 n) {
	return i - 2.0F * dot(n, i) * n; }


// view matrices
rmlm::mat4 LookFromTo(rmlv::vec4 from, rmlv::vec4 to);
rmlm::mat4 LookAt(rmlv::vec3 eye, rmlv::vec3 center, rmlv::vec3 up);


// projection matrices
rmlm::mat4 Perspective(float l, float r, float b, float t, float n, float f);
rmlm::mat4 Perspective2(float fovy, float aspect, float znear, float zfar);
rmlm::mat4 InfinitePerspective(float left, float right, float bottom, float top, float near, float far);
rmlm::mat4 Orthographic(float l, float r, float b, float t, float n, float f);


} // namespace rglv
} // namespace rqdq
