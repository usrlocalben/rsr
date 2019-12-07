#include "src/rgl/rglv/rglv_math.hxx"

#include "src/rml/rmlv/rmlv_vec.hxx"
#include "src/rml/rmlm/rmlm_mat4.hxx"

#include <cassert>

namespace rqdq {
namespace rglv {

using mat4 = rqdq::rmlm::mat4;
using vec3 = rqdq::rmlv::vec3;
using vec4 = rqdq::rmlv::vec4;


mat4 LookFromTo(vec4 from, vec4 to) {
	vec4 up;
	vec4 right;

	// build ->vector from 'from' to 'to'
	vec4 dir = normalize(from - to);

	// build a temporary 'up' vector
	if (fabs(dir.x)<FLT_EPSILON && fabs(dir.z)<FLT_EPSILON) {
		/*
		* special case where dir & up would be degenerate
		* this fix came from <midnight>
		* http://www.flipcode.com/archives/3DS_Camera_Orientation.shtml
		* an alternate up vector is used
		*/
		assert(0); // i'd like to know if i ever hit this
		up = vec4(-dir.y, 0, 0, 0); }
	else {
		up = vec4(0, 1, 0, 0); }

	// dir & right are already normalized
	// so up should be already.

	// UP cross DIR gives us "right"
	right = normalize(cross(up, dir));
	//	vec3_cross( right, up, dir );  vec3_normalize(right);

	// DIR cross RIGHT gives us a proper UP
	up = cross(dir, right);
	//	vec3_cross( up, dir, right );//vec3_normalize(up);

	mat4 mrot = mat4(
		right.x, right.y, right.z, 0.0F,
		up.x, up.y, up.z, 0.0F,
		dir.x, dir.y, dir.z, 0.0F,
		0.0F, 0.0F, 0.0F, 1.0F
	);

	mat4 mpos = mat4(
		1, 0, 0, -from.x,
		0, 1, 0, -from.y,
		0, 0, 1, -from.z,
		0, 0, 0, 1
	);

	return mrot * mpos; }


/**
 * gluLookAt
 */
mat4 LookAt(vec3 eye, vec3 center, vec3 up) {
	auto forward = normalize(center - eye);
	auto side = normalize(cross(forward, up));
	auto up2 = cross(side, forward);

	mat4 mrot = mat4{
		side.x, side.y, side.z, 0,
		up2.x, up2.y, up2.z, 0,
		-forward.x, -forward.y, -forward.z, 0,
		0, 0, 0, 1};

	mat4 mEye = mat4{
		1, 0, 0, -eye.x,
		0, 1, 0, -eye.y,
		0, 0, 1, -eye.z,
		0, 0, 0, 1};

	return mrot * mEye; }


/**
 * glFrustum matrix
 */
rmlm::mat4 Perspective(float l, float r, float b, float t, float n, float f) {
	/*standard opengl perspective transform*/
	return rmlm::mat4{
	    (2*n)/(r-l),      0,         (r+l)/(r-l),        0,
	         0,      (2*n)/(t-b),    (t+b)/(t-b),        0,
	         0,           0,      -((f+n)/(f-n)),  -((2*f*n)/(f-n)),
	         0,           0,            -1,              0            };}


/**
 * gluPerspective matrix
 */
rmlm::mat4 Perspective2(float fovy, float aspect, float znear, float zfar) {
	auto h = float(tan((fovy / 2) / 180 * rmlv::M_PI) * znear);
	auto w = float(h * aspect);
	return Perspective(-w, w, -h, h, znear, zfar); }


/**
 * glOrtho matrix
 */
rmlm::mat4 Orthographic(float l, float r, float b, float t, float n, float f) {
	/*standard opengl orthographic transform*/
	return rmlm::mat4{
	 	   2/(r-l),   0,        0,      -((r+l)/(r-l)),
	         0,    2/(t-b),     0,      -((t+b)/(t-b)),
	         0,       0,    -2/(f-n),   -((f+n)/(f-n)),
	         0,       0,        0,            1         };}


/**
 * perspective transform with infinite far clip distance
 * from the paper
 * "Practical and Robust Stenciled Shadow Volumes for Hardware-Accelerated Rendering" (2002, nvidia)
 */
rmlm::mat4 InfinitePerspective(float left, float right, float bottom, float top, float near, float far) {
	return rmlm::mat4(
		(2*near)/(right-left)           ,0           , (right+left)/(right-left)            ,0
		         ,0           ,(2*near)/(top-bottom) , (top+bottom)/(top-bottom)            ,0
		         ,0                     ,0                       ,-1                     ,-2*near
		         ,0                     ,0                       ,-1                        ,0     ); }


/*
rmlm::mat4 make_device_matrix(const int width, const int height) {
	rmlm::mat4 yFix{
		1,  0,  0,  0,
		0, -1,  0,  0,
		0,  0,  1,  0,
		0,  0,  0,  1};

	rmlm::mat4 scale{
		width/2.0F,      0,              0,          0,
		     0,    height/2.0F,          0,          0,
		     0,          0,            1.0F,         0, ///zfar-znear, // 0,
		     0,          0,              0,        1.0F};

	rmlm::mat4 origin{
		1.0F,         0,         0,  width /2.0F,
		  0,        1.0F,        0,  height/2.0F,
		  0,          0,       1.0F,       0,
		  0,          0,         0,      1.0F };

	//md = mat4_mul(origin, mat4_mul(scale, yFix));
	return origin * (scale * yFix); }
*/


}  // namespace rglv
}  // namespace rqdq
