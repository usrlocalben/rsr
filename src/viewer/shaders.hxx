#pragma once
#include <string_view>

#include "src/rgl/rglv/rglv_fragment.hxx"
#include "src/rgl/rglv/rglv_gpu_shaders.hxx"
#include "src/rgl/rglv/rglv_interpolate.hxx"
#include "src/rgl/rglv/rglv_math.hxx"
#include "src/rgl/rglv/rglv_triangle.hxx"
#include "src/rml/rmlm/rmlm_mat4.hxx"
#include "src/rml/rmlv/rmlv_soa.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"

namespace rqdq {
namespace rqv {

enum class ShaderProgramId {
	Default,
	Wireframe,
	IQ,
	Envmap,
	Amy,
	EnvmapX, };


struct ShaderProgramNameSerializer {
	static ShaderProgramId Deserialize(std::string_view text); };


/*
 * wireframe shader, adapted from
 * http://codeflow.org/entries/2012/aug/02/easy-wireframe-display-with-barycentric-coordinates/
 */
struct WireframeProgram final : public rglv::BaseProgram {
	static int id;

	template <typename TEXTURE_UNIT>
	inline static void ShadeFragment(
		// built-in
		const rmlv::qfloat2& gl_FragCoord, /* gl_FrontFacing, */ const rmlv::qfloat& gl_FragDepth,
		// unforms
		const rglv::ShaderUniforms& u,
		// vertex shader output
		const rglv::VertexOutput& v,
		// special
		const rglv::BaryCoord& _bary,
		// texture units
		const TEXTURE_UNIT& tu0,
		const TEXTURE_UNIT& tu1,
		// outputs
		rmlv::qfloat4& gl_FragColor
		) {
		using namespace rmlv;
		static const qfloat grey(0.1f);
		static const qfloat3 face_color{ 0.5f, 0.6f, 0.7f };
		const qfloat e = edgefactor(_bary);

		//auto fd = (-gl_FragDepth + qfloat(1));
		gl_FragColor = qfloat4{
			mix(grey, face_color.v[0], e) *  qfloat{4.0f},
			mix(grey, face_color.v[1], e) *  qfloat(4.0f),
			mix(grey, face_color.v[2], e) *  qfloat(4.0f),
			0.0f }; }

	inline static rmlv::qfloat edgefactor(const rmlv::qfloat3& _bary) {
		static const rmlv::qfloat thickfactor(1.5F);
		rmlv::qfloat3 d = rglv::fwidth(_bary);
		rmlv::qfloat3 a3 = rglv::smoothstep(0, d*thickfactor, _bary);
		return vmin(a3.v[0], vmin(a3.v[1], a3.v[2])); } };


struct IQPostProgram final : public rglv::BaseProgram {
	static int id;
	inline static rmlv::qfloat3 ShadeCanvas(const rmlv::qfloat2 q, const rmlv::qfloat3 source) {
		//const auto q = gl_FragCoord / targetSize;
		auto col = source;
		col = pow(col, rmlv::qfloat3(0.45F, 0.5F, 0.55F));
		col *= 0.2F + 0.8F * pow(16.0F * q.x * q.y * (1.0F - q.x) * (1.0F - q.y), 0.2F);
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
	inline static void ShadeVertex(
		const rglv::VertexInput& v,
		const rglv::ShaderUniforms& u,
		rmlv::qfloat4& gl_Position,
		rglv::VertexOutput& outs
		) {

		rmlv::qfloat4 position = mul(gl_ModelViewMatrix, IN_POSITION);
		rmlv::qfloat3 e = normalize(position.xyz());
		rmlv::qfloat4 vn = mix(IN_SMOOTH_NORMAL, IN_FACE_NORMAL, 0.0F); // in_uniform.roughness);
		rmlv::qfloat3 n = normalize(mul(gl_NormalMatrix, vn).xyz());
		rmlv::qfloat3 r = rglv::reflect(e, n);

		rmlv::qfloat m = rmlv::qfloat{2.0F} * sqrt((r.x*r.x) + (r.y*r.y) + ((r.z + 1.0F)*(r.z + 1.0F)));
		rmlv::qfloat uu = r.x / m + 0.5F;
		rmlv::qfloat vv = r.y / m + 0.5F;
		OUT_ENVMAP_UV = { uu, vv, 0 };
		gl_Position = mul(gl_ModelViewProjectionMatrix, IN_POSITION); }

	template <typename TEXTURE_UNIT>
	inline static void ShadeFragment(
		// built-in
		const rmlv::qfloat2& gl_FragCoord, /* gl_FrontFacing, */ const rmlv::qfloat& gl_FragDepth,
		// unforms
		const rglv::ShaderUniforms& u,
		// vertex shader output
		const rglv::VertexOutput& outs,
		// special
		const rglv::BaryCoord& _bary,
		// texture units
		const TEXTURE_UNIT& tu0,
		const TEXTURE_UNIT& tu1,
		// outputs
		rmlv::qfloat4& gl_FragColor
		) {
		gl_FragColor = tu0.sample({ OUT_ENVMAP_UV.x, OUT_ENVMAP_UV.y }); }
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
	static void ShadeVertex(
		const rglv::VertexInput& v,
		const rglv::ShaderUniforms& u,
		rmlv::qfloat4& gl_Position,
		rglv::VertexOutput& outs
		) {
		OUT_TEXCOORD = IN_TEXCOORD.xyz();
		gl_Position = mul(gl_ModelViewProjectionMatrix, IN_POSITION); }

	template <typename TEXTURE_UNIT>
	inline static void ShadeFragment(
		// built-in
		const rmlv::qfloat2& gl_FragCoord, /* gl_FrontFacing, */ const rmlv::qfloat& gl_FragDepth,
		// unforms
		const rglv::ShaderUniforms& u,
		// vertex shader output
		const rglv::VertexOutput& outs,
		// special
		const rglv::BaryCoord& _bary,
		// texture units
		const TEXTURE_UNIT& tu0,
		const TEXTURE_UNIT& tu1,
		// outputs
		rmlv::qfloat4& gl_FragColor
		) {
		gl_FragColor = tu0.sample({ OUT_TEXCOORD.x, OUT_TEXCOORD.y }); }
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
	inline static void ShadeVertex(
		const rglv::VertexInput& v,
		const rglv::ShaderUniforms& u,
		rmlv::qfloat4& gl_Position,
		rglv::VertexOutput& outs
		) {

		rmlv::qfloat4 position = mul(gl_ModelViewMatrix, IN_POSITION);
		rmlv::qfloat3 e = normalize(position.xyz());
		rmlv::qfloat4 vn = mix(IN_SMOOTH_NORMAL, IN_FACE_NORMAL, 0.0F); // in_uniform.roughness);
		rmlv::qfloat3 n = normalize(mul(gl_NormalMatrix, vn).xyz());
		rmlv::qfloat3 r = rglv::reflect(e, n);

		rmlv::qfloat m = rmlv::qfloat{2.0F} * sqrt((r.x*r.x) + (r.y*r.y) + ((r.z + 1.0F)*(r.z + 1.0F)));
		rmlv::qfloat uu = r.x / m + 0.5F;
		rmlv::qfloat vv = r.y / m + 0.5F;
		OUT_ENVMAP_UV = { uu, vv, 0 };
		gl_Position = mul(gl_ModelViewProjectionMatrix, IN_POSITION); }

	template <typename TEXTURE_UNIT>
	inline static void ShadeFragment(
		// built-in
		const rmlv::qfloat2& gl_FragCoord, /* gl_FrontFacing, */ const rmlv::qfloat& gl_FragDepth,
		// unforms
		const rglv::ShaderUniforms& u,
		// vertex shader output
		const rglv::VertexOutput& outs,
		// special
		const rglv::BaryCoord& _bary,
		// texture units
		const TEXTURE_UNIT& tu0,
		const TEXTURE_UNIT& tu1,
		// outputs
		rmlv::qfloat4& gl_FragColor
		) {
		gl_FragColor = tu0.sample({ OUT_ENVMAP_UV.x, OUT_ENVMAP_UV.y });
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


}  // namespace rqv
}  // namespace rqdq
