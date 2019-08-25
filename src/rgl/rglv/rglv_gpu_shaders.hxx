#pragma once
#include "src/rgl/rglv/rglv_interpolate.hxx"
#include "src/rml/rmlm/rmlm_mat4.hxx"
#include "src/rml/rmlv/rmlv_soa.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"

namespace rqdq {
namespace rglv {

struct VertexInput {
	rmlv::qfloat4 a0;
	rmlv::qfloat4 a1;
	rmlv::qfloat4 a2;
	rmlv::qfloat4 a3; };


struct ShaderUniforms {
	rmlv::qfloat4 u0;
	rmlv::qfloat4 u1;
	rmlm::qmat4 mvm;
	rmlm::qmat4 pm;
	rmlm::qmat4 nm;
	rmlm::qmat4 mvpm; };


struct VertexOutputx1 {
	rmlv::vec3 r0;
	rmlv::vec3 r1;
	rmlv::vec3 r2;
	rmlv::vec3 r3; };


inline VertexOutputx1 mix(VertexOutputx1 a, VertexOutputx1 b, float t) {
	return { mix(a.r0, b.r0, t),
	         mix(a.r1, b.r1, t),
	         mix(a.r2, b.r2, t),
	         mix(a.r3, b.r3, t) }; }


struct VertexOutput {
	rmlv::qfloat3 r0;
	rmlv::qfloat3 r1;
	rmlv::qfloat3 r2;
	rmlv::qfloat3 r3;

	VertexOutputx1 lane(const int li) {
		return VertexOutputx1{
			r0.lane(li),
			r1.lane(li),
			r2.lane(li),
			r3.lane(li), }; } };


struct BaseProgram {
	static int id;

	inline static void ShadeVertex(
		const VertexInput& v,
		const ShaderUniforms& u,
		rmlv::qfloat4& gl_Position,
		VertexOutput& outs
		) {
		gl_Position = rmlm::mul(u.mvpm, v.a0); }

	template <typename TEXTURE_UNIT>
	inline static void ShadeFragment(
		// built-in
		const rmlv::qfloat2& gl_FragCoord, /* gl_FrontFacing, */ const rmlv::qfloat& gl_FragDepth,
		// uniforms
		const ShaderUniforms& u,
		// vertex shader output
		const VertexOutput& v,
		// special
		const rglv::BaryCoord& bary,
		// texture units
		const TEXTURE_UNIT& tu0,
		const TEXTURE_UNIT& tu1,
		// outputs
		rmlv::qfloat4& gl_FragColor
		) {
		gl_FragColor = rmlv::mvec4f::one(); }

	inline static rmlv::qfloat3 ShadeCanvas(
		rmlv::qfloat2 gl_FragCoord,
		rmlv::qfloat3 source
		) {
		return source; } };


}  // namespace rglv
}  // namespace rqdq
