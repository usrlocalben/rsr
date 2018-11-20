#include <rglv_math.hxx>
#include <rmlv_vec.hxx>
#include <rmlm_mat4.hxx>


namespace rqdq {
namespace rglv {

using mat4 = rqdq::rmlm::mat4;
using vec3 = rqdq::rmlv::vec3;
using vec4 = rqdq::rmlv::vec4;


mat4 mat4_look_from_to(const vec4& from, const vec4& to) {
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
		right.x, right.y, right.z, 0.0f,
		up.x, up.y, up.z, 0.0f,
		dir.x, dir.y, dir.z, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	mat4 mpos = mat4(
		1, 0, 0, -from.x,
		0, 1, 0, -from.y,
		0, 0, 1, -from.z,
		0, 0, 0, 1
	);

	return mrot * mpos; }


mat4 look_at(vec3 pos, vec3 center, vec3 up) {
	auto forward = normalize(center - pos);
	auto side = normalize(cross(forward, up));
	auto up2 = cross(side, forward);

	mat4 mrot = mat4{
		side.x, side.y, side.z, 0,
		up2.x, up2.y, up2.z, 0,
		-forward.x, -forward.y, -forward.z, 0,
		0, 0, 0, 1};

	mat4 mpos = mat4{
		1, 0, 0, -pos.x,
		0, 1, 0, -pos.y,
		0, 0, 1, -pos.z,
		0, 0, 0, 1};

	return mrot * mpos; }



}  // close package namespace
}  // close enterprise namespace
