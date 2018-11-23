#pragma once
#include <rglv_fragment.hxx>
#include <rglv_gpu.hxx>
#include <rglv_math.hxx>
#include <rglv_triangle.hxx>
#include <rmlm_mat4.hxx>
#include <rmlv_soa.hxx>
#include <rmlv_vec.hxx>


namespace rqdq {
namespace rqv {

enum class ShaderProgramId {
	Default,
	Wireframe,
	IQ,
	Envmap,
	Amy,
	EnvmapX,
	};

ShaderProgramId deserialize_program_name(const std::string&);


/*
 * wireframe shader, adapted from
 * http://codeflow.org/entries/2012/aug/02/easy-wireframe-display-with-barycentric-coordinates/
 */
struct WireframeProgram final : public rglv::BaseProgram {
	static int id;

	template <typename TU1>
	inline static void shadeFragment(
		// built-in
		const rmlv::qfloat2& gl_FragCoord, /* gl_FrontFacing, */ const rmlv::qfloat& gl_FragDepth,
		// unforms
		const rglv::ShaderUniforms& u,
		// vertex shader output
		const rglv::VertexOutput& v,
		// special
		const rglv::tri_qfloat& _BS, const rglv::tri_qfloat& _BP,
		// texture units
		const TU1& tu1,
		// outputs
		rmlv::qfloat4& gl_FragColor
		) {
		using namespace rmlv;
		static const qfloat grey(0.1f);
		static const qfloat3 face_color{ 0.5f, 0.6f, 0.7f };
		const qfloat e = edgefactor(_BP);

		//auto fd = (-gl_FragDepth + qfloat(1));
		gl_FragColor = qfloat4{
			mix(grey, face_color.v[0], e) *  qfloat{4.0f},
			mix(grey, face_color.v[1], e) *  qfloat(4.0f),
			mix(grey, face_color.v[2], e) *  qfloat(4.0f),
			0.0f }; }

	inline static rmlv::qfloat edgefactor(const rglv::tri_qfloat& _BP) {
		static const rmlv::qfloat thickfactor(1.5f);
		rmlv::qfloat3 d = rglv::fwidth(_BP);
		rmlv::qfloat3 a3 = rglv::smoothstep(0, d*thickfactor, _BP);
		return vmin(a3.v[0], vmin(a3.v[1], a3.v[2])); } };


struct IQPostProgram final : public rglv::BaseProgram {
	static int id;
	inline static rmlv::qfloat3 shadeCanvas(const rmlv::qfloat2 q, const rmlv::qfloat3 source) {
		//const auto q = gl_FragCoord / targetSize;
		auto col = source;
		col = pow(col, rmlv::qfloat3(0.45f, 0.5f, 0.55f));
		col *= 0.2f + 0.8f * pow(16.0f * q.x * q.y * (1.0f - q.x) * (1.0f - q.y), 0.2f);
		col += (1.0 / 255.0) * rglv::hash3(q.x + 13.0*q.y);
		return col; }};


struct EnvmapProgram final : public rglv::BaseProgram {
	static int id;
#define IN_POSITION v.a0
#define IN_SMOOTH_NORMAL v.a1
#define IN_FACE_NORMAL v.a2
#define OUT_ENVMAP_UV outs.r0
#define gl_ModelViewMatrix u.mvm
#define gl_ModelViewProjectionMatrix u.mvpm
#define gl_NormalMatrix u.nm
	inline static void shadeVertex(
		const rglv::VertexInput& v,
		const rglv::ShaderUniforms& u,
		rmlv::qfloat4& gl_Position,
		rglv::VertexOutput& outs
		) {

		rmlv::qfloat4 position = mul(gl_ModelViewMatrix, IN_POSITION);
		rmlv::qfloat3 e = normalize(position.xyz());
		rmlv::qfloat4 vn = mix(IN_SMOOTH_NORMAL, IN_FACE_NORMAL, 0.0f); // in_uniform.roughness);
		rmlv::qfloat3 n = normalize(mul(gl_NormalMatrix, vn).xyz());
		rmlv::qfloat3 r = rglv::reflect(e, n);

		rmlv::qfloat m = rmlv::qfloat{2.0f} * sqrt((r.x*r.x) + (r.y*r.y) + ((r.z + 1.0f)*(r.z + 1.0f)));
		rmlv::qfloat uu = r.x / m + 0.5f;
		rmlv::qfloat vv = r.y / m + 0.5f;
		OUT_ENVMAP_UV = { uu, vv, 0 };
		gl_Position = mul(gl_ModelViewProjectionMatrix, IN_POSITION); }

	template <typename TU1>
	inline static void shadeFragment(
		// built-in
		const rmlv::qfloat2& gl_FragCoord, /* gl_FrontFacing, */ const rmlv::qfloat& gl_FragDepth,
		// unforms
		const rglv::ShaderUniforms& u,
		// vertex shader output
		const rglv::VertexOutput& outs,
		// special
		const rglv::tri_qfloat& _BS, const rglv::tri_qfloat& _BP,
		// texture units
		const TU1& tu1,
		// outputs
		rmlv::qfloat4& gl_FragColor
		) {
		gl_FragColor = tu1.sample({ OUT_ENVMAP_UV.x, OUT_ENVMAP_UV.y }); }
#undef gl_ModelViewMatrix
#undef gl_ModelViewProjectionMatrix
#undef gl_NormalMatrix
#undef OUT_ENVMAP_UV
#undef IN_POSITION
#undef IN_SMOOTH_NORMAL
#undef IN_FACE_NORMAL
};


struct AmyProgram final : public rglv::BaseProgram {
	static int id;
#define IN_POSITION v.a0
#define IN_TEXCOORD v.a2
#define OUT_TEXCOORD outs.r0
#define gl_ModelViewProjectionMatrix u.mvpm
	static void shadeVertex(
		const rglv::VertexInput& v,
		const rglv::ShaderUniforms& u,
		rmlv::qfloat4& gl_Position,
		rglv::VertexOutput& outs
		) {
		OUT_TEXCOORD = IN_TEXCOORD.xyz();
		gl_Position = mul(gl_ModelViewProjectionMatrix, IN_POSITION); }

	template <typename TU1>
	inline static void shadeFragment(
		// built-in
		const rmlv::qfloat2& gl_FragCoord, /* gl_FrontFacing, */ const rmlv::qfloat& gl_FragDepth,
		// unforms
		const rglv::ShaderUniforms& u,
		// vertex shader output
		const rglv::VertexOutput& outs,
		// special
		const rglv::tri_qfloat& _BS, const rglv::tri_qfloat& _BP,
		// texture units
		const TU1& tu1,
		// outputs
		rmlv::qfloat4& gl_FragColor
		) {
		gl_FragColor = tu1.sample({ OUT_TEXCOORD.x, OUT_TEXCOORD.y }); }
#undef gl_ModelViewProjectionMatrix
#undef OUT_TEXCOORD
#undef IN_TEXCOORD
#undef IN_POSITION
};


struct EnvmapXProgram final : public rglv::BaseProgram {
	static int id;
#define IN_POSITION v.a0
#define IN_SMOOTH_NORMAL v.a1
#define IN_FACE_NORMAL v.a2
#define UNIFORM_BACKCOLOR u.u0
#define UNIFORM_OPACITY u.u1
#define OUT_ENVMAP_UV outs.r0
#define gl_ModelViewMatrix u.mvm
#define gl_ModelViewProjectionMatrix u.mvpm
#define gl_NormalMatrix u.nm
	inline static void shadeVertex(
		const rglv::VertexInput& v,
		const rglv::ShaderUniforms& u,
		rmlv::qfloat4& gl_Position,
		rglv::VertexOutput& outs
		) {

		rmlv::qfloat4 position = mul(gl_ModelViewMatrix, IN_POSITION);
		rmlv::qfloat3 e = normalize(position.xyz());
		rmlv::qfloat4 vn = mix(IN_SMOOTH_NORMAL, IN_FACE_NORMAL, 0.0f); // in_uniform.roughness);
		rmlv::qfloat3 n = normalize(mul(gl_NormalMatrix, vn).xyz());
		rmlv::qfloat3 r = rglv::reflect(e, n);

		rmlv::qfloat m = rmlv::qfloat{2.0f} * sqrt((r.x*r.x) + (r.y*r.y) + ((r.z + 1.0f)*(r.z + 1.0f)));
		rmlv::qfloat uu = r.x / m + 0.5f;
		rmlv::qfloat vv = r.y / m + 0.5f;
		OUT_ENVMAP_UV = { uu, vv, 0 };
		gl_Position = mul(gl_ModelViewProjectionMatrix, IN_POSITION); }

	template <typename TU1>
	inline static void shadeFragment(
		// built-in
		const rmlv::qfloat2& gl_FragCoord, /* gl_FrontFacing, */ const rmlv::qfloat& gl_FragDepth,
		// unforms
		const rglv::ShaderUniforms& u,
		// vertex shader output
		const rglv::VertexOutput& outs,
		// special
		const rglv::tri_qfloat& _BS, const rglv::tri_qfloat& _BP,
		// texture units
		const TU1& tu1,
		// outputs
		rmlv::qfloat4& gl_FragColor
		) {
		gl_FragColor = tu1.sample({ OUT_ENVMAP_UV.x, OUT_ENVMAP_UV.y });
		gl_FragColor = mix(gl_FragColor, UNIFORM_BACKCOLOR, UNIFORM_OPACITY.x); }
#undef gl_ModelViewMatrix
#undef gl_ModelViewProjectionMatrix
#undef gl_NormalMatrix
#undef OUT_ENVMAP_UV
#undef IN_POSITION
#undef IN_SMOOTH_NORMAL
#undef IN_FACE_NORMAL
#undef UNIFORM_BACKCOLOR
#undef UNIFORM_OPACITY
};


}  // close package namespace
}  // close enterprise namespace
