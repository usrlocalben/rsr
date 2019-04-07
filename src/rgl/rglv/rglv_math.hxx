#pragma once
#include <xmmintrin.h>

#include "src/rml/rmlm/rmlm_mat4.hxx"
#include "src/rml/rmlv/rmlv_soa.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"

namespace rqdq {
namespace rglv {

/*
 * perspective divide, SoA version
 *
 * float operations _must_ match scalar version !
 *
 * Args:
 *   p: point in homogeneous clip-space
 *
 * Returns:
 *   x/w, y/w, z/w, 1/w
 */
inline rmlv::qfloat4 pdiv(rmlv::qfloat4 p) {
	__m128 invw = _mm_rcp_ps(p.w.v);
	//__m128 r1 = _mm_div_ps(_mm_set1_ps(1.0f), p.w.v);
	return{ p.x * invw, p.y * invw, p.z * invw, invw }; }


/*
 * perspective divide, scalar version
 *
 * float operations _must_ match SoA version !
 */
inline rmlv::vec4 pdiv(rmlv::vec4 p) {
	float invw;
	_mm_store_ss(&invw, _mm_rcp_ss(_mm_set_ss(p.w)));
	auto bias_z = 0.5F * (p.z * invw) + 0.5F;
	return rmlv::vec4{ p.x*invw, p.y*invw, bias_z, invw }; }


inline rmlv::vec3 reflect(rmlv::vec3 i, rmlv::vec3 n) {
	return i - 2.0F * dot(n, i) * n; }

inline rmlv::qfloat3 reflect(const rmlv::qfloat3& i, const rmlv::qfloat3& n) {
	return i - rmlv::qfloat{2.0F} * dot(n, i) * n; }

/**
 * matrix/vector multiply, specialized for the device-matrix
 */
__forceinline rmlv::vec4 devmatmul(const rmlm::mat4 a, const rmlv::vec4 b) {
	return rmlv::vec4{
		a.ff[0] * b.x +             0 +             0 + a.ff[12] * b.w,
		            0 + a.ff[5] * b.y +             0 + a.ff[13] * b.w,
		            0 +             0 +           b.z +              0,
		            0 +             0 +             0 +            b.w
		}; }


__forceinline auto devmatmul(const rmlm::qmat4 a, const rmlv::qfloat4 b) {
	return rmlv::qfloat4{
		a.f[0]*b.x + a.f[12]*b.w,
		a.f[5]*b.y + a.f[13]*b.w,
		b.z,
		b.w
		}; }

/**
 * matrix/vector multiply, specialized for the device-matrix
 */
/*
__forceinline rmlv::vec4 devmatmul(const rmlm::qmat4 a, const rmlv::qfloat4 b) {
	return rmlv::qfloat4{
		a.ff[0] * b.x +             0 +             0 + a.ff[12] * b.w,
		            0 + a.ff[5] * b.y +             0 + a.ff[13] * b.w,
		            0 +             0 +           b.z +              0,
		            0 +             0 +             0 +            b.w
		}; }
*/

rmlm::mat4 look_at(rmlv::vec3 pos, rmlv::vec3 center, rmlv::vec3 up);


inline rmlm::mat4 make_glFrustum(const float l, const float r, const float b, const float t, const float n, const float f) {
	/*standard opengl perspective transform*/
	return rmlm::mat4{
	    (2*n)/(r-l),      0,         (r+l)/(r-l),        0,
	         0,      (2*n)/(t-b),    (t+b)/(t-b),        0,
	         0,           0,      -((f+n)/(f-n)),  -((2*f*n)/(f-n)),
	         0,           0,            -1,              0
		};}


inline rmlm::mat4 make_glOrtho(const float l, const float r, const float b, const float t, const float n, const float f) {
	/*standard opengl orthographic transform*/
	return rmlm::mat4{
	 	   2/(r-l),   0,        0,      -((r+l)/(r-l)),
	         0,    2/(t-b),     0,      -((t+b)/(t-b)),
	         0,       0,    -2/(f-n),   -((f+n)/(f-n)),
	         0,       0,        0,            1
		};}


inline rmlm::mat4 create_opengl_pinf(const float left, const float right, const float bottom, const float top, const float near, const float far) {
	/*perspective transform with infinite far clip distance
	from the paper
	"Practical and Robust Stenciled Shadow Volumes for Hardware-Accelerated Rendering" (2002, nvidia)
	*/
	return rmlm::mat4(
		(2*near)/(right-left)           ,0           , (right+left)/(right-left)            ,0
		         ,0           ,(2*near)/(top-bottom) , (top+bottom)/(top-bottom)            ,0
		         ,0                     ,0                       ,-1                     ,-2*near
		         ,0                     ,0                       ,-1                        ,0
		); }


inline rmlm::mat4 make_gluPerspective(const float fovy, const float aspect, const float znear, const float zfar) {
	auto h = float(tan((fovy / 2) / 180 * rmlv::M_PI) * znear);
	auto w = float(h * aspect);
	return make_glFrustum(-w, w, -h, h, znear, zfar); }


inline rmlm::mat4 make_device_matrix(const int width, const int height) {
	const auto x0 = 0;
	const auto y0 = 0;

	auto md_yfix = rmlm::mat4(
		   1,        0,       0,       0,
		   0,       -1,       0,       0,
		   0,        0,       1,       0,
		   0,        0,       0,       1);

	auto md_scale = rmlm::mat4(
		float(width/2), 0,          0,       0,
		     0,    float(height/2), 0,       0,
		     0,         0,          1.0F,    0, ///zfar-znear, // 0,
		     0,         0,          0,       1.0F);

	auto md_origin = rmlm::mat4(
		   1,        0,       0,  float(x0+width /2),
		   0,        1,       0,  float(y0+height/2),
		   0,        0,       1,       0,
		   0,        0,       0,       1);

	//md = mat4_mul(md_origin, mat4_mul(md_scale, md_yfix));
	return md_origin * (md_scale * md_yfix); }


} // namespace rglv
} // namespace rqdq
