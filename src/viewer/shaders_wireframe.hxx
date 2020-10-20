#pragma once

#include "src/rgl/rglv/rglv_fragment.hxx"
#include "src/rgl/rglv/rglv_gpu.hxx"
#include "src/rgl/rglv/rglv_gpu_protocol.hxx"
#include "src/rgl/rglv/rglv_gpu_shaders.hxx"
#include "src/rgl/rglv/rglv_interpolate.hxx"
#include "src/rgl/rglv/rglv_math.hxx"
#include "src/rgl/rglv/rglv_triangle.hxx"
#include "src/rgl/rglv/rglv_vao.hxx"
#include "src/rml/rmlm/rmlm_mat4.hxx"
#include "src/rml/rmlm/rmlm_soa.hxx"
#include "src/rml/rmlv/rmlv_soa.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"

#define gl_ModelViewMatrix mats.vm
#define gl_ModelViewProjectionMatrix mats.vpm
#define gl_NormalMatrix mats.nm
#define gl_ProjectionMatrix mats.pm

namespace rqdq {
namespace rqv {


/*
 * wireframe shader, adapted from
 * http://codeflow.org/entries/2012/aug/02/easy-wireframe-display-with-barycentric-coordinates/
 */
struct WireframeProgram final : public rglv::BaseProgram {
	static constexpr int id = 11;

	template <typename TU0, typename TU1, typename TU3>
	inline static void ShadeFragment(
		const rglv::Matrices& mats,
		const rglv::BaseProgram::UniformsMD& u,
		const TU0 tu0,
		const TU1 tu1,
		const TU3& tu3,
		const rglv::BaryCoord& BS,
		const rglv::BaryCoord& BP,
		const WireframeProgram::VertexOutputMD& v,
		const rmlv::qfloat2& gl_FragCoord,
		/* gl_FrontFacing, */
		const rmlv::qfloat& gl_FragDepth,
		rmlv::qfloat4& gl_FragColor
		) {
		using namespace rmlv;
		static const qfloat grey(0.1f);
		static const qfloat3 face_color{ 0.5f, 0.6f, 0.7f };
		const qfloat e = edgefactor(BP);

		//auto fd = (-gl_FragDepth + qfloat(1));
		gl_FragColor = qfloat4{
			mix(grey, face_color.v[0], e) *  qfloat{4.0f},
			mix(grey, face_color.v[1], e) *  qfloat(4.0f),
			mix(grey, face_color.v[2], e) *  qfloat(4.0f),
			0.0f }; }

	inline static rmlv::qfloat edgefactor(const rmlv::qfloat3& BP) {
		static const rmlv::qfloat thickfactor(1.5F);
		rmlv::qfloat3 d = rglv::fwidth(BP);
		rmlv::qfloat3 a3 = rglv::smoothstep(0, d*thickfactor, BP);
		return vmin(a3.v[0], vmin(a3.v[1], a3.v[2])); } };


void InstallWireframe(class rqdq::rglv::GPU& gpu);


}  // namespace rqv
}  // namespace rqdq

#undef gl_ModelViewMatrix
#undef gl_ModelViewProjectionMatrix
#undef gl_NormalMatrix
#undef gl_ProjectionMatrix
