#pragma once
#include <array>
#include <cassert>
#include <string_view>

#include "src/rgl/rglv/rglv_fragment.hxx"
#include "src/rgl/rglv/rglv_gpu_protocol.hxx"
#include "src/rgl/rglv/rglv_gpu_shaders.hxx"
#include "src/rgl/rglv/rglv_interpolate.hxx"
#include "src/rgl/rglv/rglv_math.hxx"
#include "src/rgl/rglv/rglv_triangle.hxx"
#include "src/rgl/rglv/rglv_vao.hxx"
#include "src/rml/rmlm/rmlm_mat4.hxx"
#include "src/rml/rmlv/rmlv_soa.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"

#define gl_ModelViewMatrix u.mvm
#define gl_ModelViewProjectionMatrix u.mvpm
#define gl_NormalMatrix u.nm

namespace rqdq {
namespace rqv {

enum class ShaderProgramId {
	Default,
	Wireframe,
	IQ,
	Envmap,
	Amy,
	EnvmapX,
	Many, };


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
		const rglv::BaseProgram::UniformsMD& u,
		// vertex shader output
		const WireframeProgram::VertexOutputMD& v,
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

	struct UniformsSD {
		rmlm::mat4 mvm;
		rmlm::mat4 pm;
		rmlm::mat4 nm;
		rmlm::mat4 mvpm; };

	struct UniformsMD {
		const rmlm::qmat4 mvm;
		const rmlm::qmat4 pm;
		const rmlm::qmat4 nm;
		const rmlm::qmat4 mvpm;

		UniformsMD(const UniformsSD& data) :
			mvm(data.mvm),
			pm(data.pm),
			nm(data.nm),
			mvpm(data.mvpm) {} };

	struct VertexInput {
		rmlv::qfloat4 position;
		rmlv::qfloat4 smoothNormal;
		rmlv::qfloat4 faceNormal; };

	struct Loader {
		Loader(const std::array<const void*, 4>& buffers, const std::array<int, 4>& formats) :
			data_(*static_cast<const rglv::VertexArray_F3F3F3*>(buffers[0])) {
			assert(formats[0] == rglv::AF_VAO_F3F3F3);
			assert(buffers[0] != nullptr); }
		int Size() const { return data_.size(); }
		void LoadInstance(int id, VertexInput& vi) {}
		void LoadMD(int idx, VertexInput& vi) {
			vi.position     = data_.a0.loadxyz1(idx);
			vi.smoothNormal = data_.a1.loadxyz0(idx);
			vi.faceNormal   = data_.a2.loadxyz0(idx); }
		void LoadLane(int idx, int li, VertexInput& vi) {
			vi.position    .setLane(li, rmlv::vec4{ data_.a0.at(idx), 1 });
			vi.smoothNormal.setLane(li, rmlv::vec4{ data_.a1.at(idx), 0 });
			vi.faceNormal  .setLane(li, rmlv::vec4{ data_.a2.at(idx), 0 }); }
		const rglv::VertexArray_F3F3F3& data_; };

	struct VertexOutputSD {
		rmlv::vec2 envmapUV;
		static VertexOutputSD Mix(VertexOutputSD a, VertexOutputSD b, float t) {
			return { mix(a.envmapUV, b.envmapUV, t) }; }};

	struct VertexOutputMD {
		rmlv::qfloat2 envmapUV;

		VertexOutputSD Lane(const int li) {
			return VertexOutputSD{
				envmapUV.lane(li) }; }};

	struct Interpolants {
		Interpolants(VertexOutputSD d0, VertexOutputSD d1, VertexOutputSD d2) :
			envmapUV({ d0.envmapUV, d1.envmapUV, d2.envmapUV }) {}
		VertexOutputMD Interpolate(rglv::BaryCoord bary) const {
			return { rglv::Interpolate(bary, envmapUV) }; }
		rglv::VertexFloat2 envmapUV; };

	inline static void ShadeVertex(
		const VertexInput& v,
		const UniformsMD& u,
		rmlv::qfloat4& gl_Position,
		VertexOutputMD& out
		) {

		rmlv::qfloat4 position = mul(gl_ModelViewMatrix, v.position);
		rmlv::qfloat3 e = normalize(position.xyz());
		rmlv::qfloat4 vn = mix(v.smoothNormal, v.faceNormal, 0.0F); // in_uniform.roughness);
		rmlv::qfloat3 n = normalize(mul(gl_NormalMatrix, vn).xyz());
		rmlv::qfloat3 r = rglv::reflect(e, n);

		rmlv::qfloat m = rmlv::qfloat{2.0F} * sqrt((r.x*r.x) + (r.y*r.y) + ((r.z + 1.0F)*(r.z + 1.0F)));
		rmlv::qfloat uu = r.x / m + 0.5F;
		rmlv::qfloat vv = r.y / m + 0.5F;
		out.envmapUV = { uu, vv };
		gl_Position = mul(gl_ModelViewProjectionMatrix, v.position); }

	template <typename TEXTURE_UNIT>
	inline static void ShadeFragment(
		// built-in
		const rmlv::qfloat2& gl_FragCoord, /* gl_FrontFacing, */ const rmlv::qfloat& gl_FragDepth,
		// unforms
		const UniformsMD& u,
		// vertex shader output
		const VertexOutputMD& outs,
		// special
		const rglv::BaryCoord& _bary,
		// texture units
		const TEXTURE_UNIT& tu0,
		const TEXTURE_UNIT& tu1,
		// outputs
		rmlv::qfloat4& gl_FragColor
		) {
		gl_FragColor = tu0.sample({ outs.envmapUV.x, outs.envmapUV.y }); } };


struct AmyProgram final : public rglv::BaseProgram {
	static int id;

	struct VertexInput {
		rmlv::qfloat4 position;
		rmlv::qfloat4 normal;
		rmlv::qfloat4 uv; };

	struct Loader {
		Loader(const std::array<const void*, 4>& buffers, const std::array<int, 4>& formats) :
			data_(*static_cast<const rglv::VertexArray_F3F3F3*>(buffers[0])) {
			assert(formats[0] == rglv::AF_VAO_F3F3F3);
			assert(buffers[0] != nullptr); }
		int Size() const { return data_.size(); }
		void LoadInstance(int id, VertexInput& vi) {}
		void LoadMD(int idx, VertexInput& vi) {
			vi.position = data_.a0.loadxyz1(idx);
			vi.normal   = data_.a1.loadxyz0(idx);
			vi.uv       = data_.a2.loadxyz0(idx); }
		void LoadLane(int idx, int li, VertexInput& vi) {
			vi.position.setLane(li, rmlv::vec4{ data_.a0.at(idx), 1 });
			vi.normal  .setLane(li, rmlv::vec4{ data_.a1.at(idx), 0 });
			vi.uv      .setLane(li, rmlv::vec4{ data_.a2.at(idx), 0 }); }
		const rglv::VertexArray_F3F3F3& data_; };

	struct VertexOutputSD {
		rmlv::vec3 uv;
		static VertexOutputSD Mix(VertexOutputSD a, VertexOutputSD b, float t) {
			return { mix(a.uv, b.uv, t) }; }};

	struct VertexOutputMD {
		rmlv::qfloat3 uv;

		VertexOutputSD Lane(const int li) {
			return VertexOutputSD{
				uv.lane(li) }; }};

	struct Interpolants {
		Interpolants(VertexOutputSD d0, VertexOutputSD d1, VertexOutputSD d2) :
			uv({ d0.uv, d1.uv, d2.uv }) {}
		VertexOutputMD Interpolate(rglv::BaryCoord bary) const {
			return { rglv::Interpolate(bary, uv) }; }
		rglv::VertexFloat3 uv; };

	static void ShadeVertex(
		const VertexInput& v,
		const rglv::BaseProgram::UniformsMD& u,
		rmlv::qfloat4& gl_Position,
		VertexOutputMD& outs
		) {
		outs.uv = v.uv.xyz();
		gl_Position = mul(gl_ModelViewProjectionMatrix, v.position); }

	template <typename TEXTURE_UNIT>
	inline static void ShadeFragment(
		// built-in
		const rmlv::qfloat2& gl_FragCoord, /* gl_FrontFacing, */ const rmlv::qfloat& gl_FragDepth,
		// unforms
		const rglv::BaseProgram::UniformsMD& u,
		// vertex shader output
		const VertexOutputMD& outs,
		// special
		const rglv::BaryCoord& _bary,
		// texture units
		const TEXTURE_UNIT& tu0,
		const TEXTURE_UNIT& tu1,
		// outputs
		rmlv::qfloat4& gl_FragColor
		) {
		gl_FragColor = tu0.sample({ outs.uv.x, outs.uv.y }); } };


struct EnvmapXProgram final : public rglv::BaseProgram {
	static int id;

	struct UniformsSD {
		rmlm::mat4 mvm;
		rmlm::mat4 pm;
		rmlm::mat4 nm;
		rmlm::mat4 mvpm;
		rmlv::vec4 backColor;
		float opacity; };

	struct UniformsMD {
		rmlm::qmat4 mvm;
		rmlm::qmat4 pm;
		rmlm::qmat4 nm;
		rmlm::qmat4 mvpm;
		rmlv::qfloat4 backColor;
		rmlv::qfloat opacity;

		UniformsMD(const UniformsSD& data) :
			mvm(data.mvm),
			pm(data.pm),
			nm(data.nm),
			mvpm(data.mvpm),
			backColor(data.backColor),
			opacity(data.opacity) {} };

	struct VertexInput {
		rmlv::qfloat4 position;
		rmlv::qfloat4 smoothNormal; };

	struct Loader {
		Loader(const std::array<const void*, 4>& buffers, const std::array<int, 4>& formats) :
			data_(*static_cast<const rglv::VertexArray_F3F3F3*>(buffers[0])) {
			assert(formats[0] == rglv::AF_VAO_F3F3F3);
			assert(buffers[0] != nullptr); }
		int Size() const { return data_.size(); }
		void LoadInstance(int id, VertexInput& vi) {}
		void LoadMD(int idx, VertexInput& vi) {
			vi.position     = data_.a0.loadxyz1(idx);
			vi.smoothNormal = data_.a1.loadxyz0(idx); }
		void LoadLane(int idx, int li, VertexInput& vi) {
			vi.position    .setLane(li, rmlv::vec4{ data_.a0.at(idx), 1 });
			vi.smoothNormal.setLane(li, rmlv::vec4{ data_.a1.at(idx), 0 }); }
		const rglv::VertexArray_F3F3F3& data_; };

	struct VertexOutputSD {
		rmlv::vec3 envmapUV;
		static VertexOutputSD Mix(VertexOutputSD a, VertexOutputSD b, float t) {
			return { mix(a.envmapUV, b.envmapUV, t) }; }};

	struct VertexOutputMD {
		rmlv::qfloat3 envmapUV;

		VertexOutputSD Lane(const int li) {
			return VertexOutputSD{
				envmapUV.lane(li) }; }};

	struct Interpolants {
		Interpolants(VertexOutputSD d0, VertexOutputSD d1, VertexOutputSD d2) :
			envmapUV({ d0.envmapUV, d1.envmapUV, d2.envmapUV }) {}
		VertexOutputMD Interpolate(rglv::BaryCoord bary) const {
			return { rglv::Interpolate(bary, envmapUV) }; }
		rglv::VertexFloat3 envmapUV; };

	inline static void ShadeVertex(
		const VertexInput& v,
		const UniformsMD& u,
		rmlv::qfloat4& gl_Position,
		VertexOutputMD& outs
		) {

		rmlv::qfloat4 position = mul(gl_ModelViewMatrix, v.position);
		rmlv::qfloat3 e = normalize(position.xyz());
		rmlv::qfloat3 n = normalize(mul(gl_NormalMatrix, v.smoothNormal).xyz());
		rmlv::qfloat3 r = rglv::reflect(e, n);

		rmlv::qfloat m = rmlv::qfloat{2.0F} * sqrt((r.x*r.x) + (r.y*r.y) + ((r.z + 1.0F)*(r.z + 1.0F)));
		rmlv::qfloat uu = r.x / m + 0.5F;
		rmlv::qfloat vv = r.y / m + 0.5F;
		outs.envmapUV = { uu, vv, 0 };
		gl_Position = mul(gl_ModelViewProjectionMatrix, v.position); }

	template <typename TEXTURE_UNIT>
	inline static void ShadeFragment(
		// built-in
		const rmlv::qfloat2& gl_FragCoord, /* gl_FrontFacing, */ const rmlv::qfloat& gl_FragDepth,
		// unforms
		const UniformsMD& u,
		// vertex shader output
		const VertexOutputMD& outs,
		// special
		const rglv::BaryCoord& _bary,
		// texture units
		const TEXTURE_UNIT& tu0,
		const TEXTURE_UNIT& tu1,
		// outputs
		rmlv::qfloat4& gl_FragColor
		) {
		gl_FragColor = tu0.sample({ outs.envmapUV.x, outs.envmapUV.y });
		// gl_FragColor = mix(gl_FragColor, u.backColor, u.opacity); }
		}
		};


struct ManyProgram final : public rglv::BaseProgram {
	static int id;

	struct UniformsSD {
		rmlm::mat4 mvm;
		rmlm::mat4 pm;
		rmlm::mat4 nm;
		rmlm::mat4 mvpm;
		float magic; };

	struct UniformsMD {
		rmlm::qmat4 mvm;
		rmlm::qmat4 pm;
		rmlm::qmat4 nm;
		rmlm::qmat4 mvpm;
		rmlv::qfloat magic;

		UniformsMD(const UniformsSD& data) :
			mvm(data.mvm),
			pm(data.pm),
			nm(data.nm),
			mvpm(data.mvpm),
			magic(data.magic) {} };

	struct VertexInput {
		rmlv::qfloat4 position;
		rmlv::qfloat4 normal;
		rmlv::qfloat2 uv;
		rmlm::qmat4 imat; };

	struct Loader {
		Loader(const std::array<const void*, 4>& buffers, const std::array<int, 4>& formats) :
			vbo_(*static_cast<const rglv::VertexArray_F3F3F3*>(buffers[0])),
			mats_(static_cast<const rmlm::mat4*>(buffers[1])) {
			assert(formats[0] == rglv::AF_VAO_F3F3F3);
			assert(buffers[0] != nullptr);
			assert(formats[1] == rglv::AF_FLOAT);
			assert(buffers[1] != nullptr); }
		int Size() const { return vbo_.size(); }
		void LoadInstance(int id, VertexInput& vi) {
			vi.imat = mats_[id]; } //rmlm::mat4::ident(); } //mats_[id]; }
		void LoadMD(int idx, VertexInput& vi) {
			vi.position = vbo_.a0.loadxyz1(idx);
			vi.normal   = vbo_.a1.loadxyz0(idx);
			vi.uv       = vbo_.a2.loadxy(idx); }
		void LoadLane(int idx, int li, VertexInput& vi) {
			vi.position.setLane(li, rmlv::vec4{ vbo_.a0.at(idx), 1 });
			vi.normal  .setLane(li, rmlv::vec4{ vbo_.a1.at(idx), 0 });
			vi.uv      .setLane(li, vbo_.a2.at(idx).xy()); }
		const rglv::VertexArray_F3F3F3& vbo_;
		const rmlm::mat4* const mats_; };

	struct VertexOutputSD {
		rmlv::vec2 uv;
		static VertexOutputSD Mix(VertexOutputSD a, VertexOutputSD b, float t) {
			return { mix(a.uv, b.uv, t) }; }};

	struct VertexOutputMD {
		rmlv::qfloat2 uv;

		VertexOutputSD Lane(const int li) {
			return VertexOutputSD{
				uv.lane(li) }; }};

	struct Interpolants {
		Interpolants(VertexOutputSD d0, VertexOutputSD d1, VertexOutputSD d2) :
			uv({ d0.uv, d1.uv, d2.uv }) {}
		VertexOutputMD Interpolate(rglv::BaryCoord bary) const {
			return { rglv::Interpolate(bary, uv) }; }
		rglv::VertexFloat2 uv; };

	inline static void ShadeVertex(
		const VertexInput& v,
		const UniformsMD& u,
		rmlv::qfloat4& gl_Position,
		VertexOutputMD& outs
		) {
		rmlv::qfloat4 p1 = mul(v.imat, v.position);
		outs.uv = v.uv;
		gl_Position = mul(gl_ModelViewProjectionMatrix, p1); }

	template <typename TEXTURE_UNIT>
	inline static void ShadeFragment(
		// built-in
		const rmlv::qfloat2& gl_FragCoord, /* gl_FrontFacing, */ const rmlv::qfloat& gl_FragDepth,
		// unforms
		const UniformsMD& u,
		// vertex shader output
		const VertexOutputMD& outs,
		// special
		const rglv::BaryCoord& _bary,
		// texture units
		const TEXTURE_UNIT& tu0,
		const TEXTURE_UNIT& tu1,
		// outputs
		rmlv::qfloat4& gl_FragColor
		) {
		gl_FragColor = { outs.uv.x, outs.uv.y, u.magic, 1.0F }; }
		};


}  // namespace rqv
}  // namespace rqdq

#undef gl_ModelViewMatrix
#undef gl_ModelViewProjectionMatrix
#undef gl_NormalMatrix
