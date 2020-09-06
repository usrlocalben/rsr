#pragma once
#include <array>
#include <cassert>
#include <string_view>

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

enum class ShaderProgramId {
	Default,
	Wireframe,
	IQ,
	Envmap,
	Amy,
	Depth,
	Many,
	OBJ1,
	OBJ2,
	OBJ2S };


struct ShaderProgramNameSerializer {
	static ShaderProgramId Deserialize(std::string_view text); };


/*
 * wireframe shader, adapted from
 * http://codeflow.org/entries/2012/aug/02/easy-wireframe-display-with-barycentric-coordinates/
 */
struct WireframeProgram final : public rglv::BaseProgram {
	static constexpr int id = int(ShaderProgramId::Wireframe);

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


struct DefaultPostProgram final : public rglv::BaseProgram {
	static constexpr int id = int(ShaderProgramId::IQ);

	static inline
	auto ShadeCanvas(const rmlv::qfloat2 q [[maybe_unused]], const rmlv::qfloat3 source) -> rmlv::qfloat3 {
		return source; }};


struct IQPostProgram final : public rglv::BaseProgram {
	static constexpr int id = int(ShaderProgramId::IQ);

	inline static rmlv::qfloat3 ShadeCanvas(const rmlv::qfloat2 q, const rmlv::qfloat3 source) {
		//const auto q = gl_FragCoord / targetSize;
		auto col = source;
		col = pow(col, rmlv::qfloat3(0.45F, 0.5F, 0.55F));
		col *= 0.2F + 0.8F * pow(16.0F * q.x * q.y * (1.0F - q.x) * (1.0F - q.y), 0.2F);
		col += (1.0 / 255.0) * rglv::hash3(q.x + 13.0*q.y);
		return col; }};


struct EnvmapProgram final : public rglv::BaseProgram {
	static constexpr int id = int(ShaderProgramId::Envmap);

	struct VertexInput {
		rmlv::qfloat4 position;
		rmlv::qfloat4 smoothNormal;
		rmlv::qfloat4 faceNormal; };

	struct Loader {
		Loader(const std::array<const void*, 4>& buffers,
		       const std::array<int, 4>& formats [[maybe_unused]]) :
			data_(*static_cast<const rglv::VertexArray_F3F3F3*>(buffers[0])) {
			assert(formats[0] == rglv::AF_VAO_F3F3F3);
			assert(buffers[0] != nullptr); }
		int Size() const { return data_.size(); }
		void LoadInstance(int, VertexInput&) {}
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
		VertexOutputMD Interpolate(rglv::BaryCoord BS [[maybe_unused]], rglv::BaryCoord BP) const {
			return { rglv::Interpolate(BP, envmapUV) }; }
		rglv::VertexFloat2 envmapUV; };

	inline static void ShadeVertex(
		const rglv::Matrices& mats [[maybe_unused]],
		const UniformsMD& u [[maybe_unused]],
		const VertexInput& v [[maybe_unused]],
		rmlv::qfloat4& gl_Position,
		VertexOutputMD& out [[maybe_unused]]) {

		rmlv::qfloat4 position = gl_ModelViewMatrix * v.position;
		rmlv::qfloat3 e = normalize(position.xyz());
		rmlv::qfloat4 vn = mix(v.smoothNormal, v.faceNormal, 0.0F); // in_uniform.roughness);
		rmlv::qfloat3 n = normalize(mul_w0(gl_NormalMatrix, vn));
		rmlv::qfloat3 r = rglv::reflect(e, n);

		rmlv::qfloat m = rmlv::qfloat{2.0F} * sqrt((r.x*r.x) + (r.y*r.y) + ((r.z + 1.0F)*(r.z + 1.0F)));
		rmlv::qfloat uu = r.x / m + 0.5F;
		rmlv::qfloat vv = r.y / m + 0.5F;
		out.envmapUV = { uu, vv };
		gl_Position = gl_ModelViewProjectionMatrix * v.position; }

	template <typename TU0, typename TU1, typename TU3>
	inline static void ShadeFragment(
		const rglv::Matrices& mats [[maybe_unused]],
		const UniformsMD& u [[maybe_unused]],
		const TU0 tu0 [[maybe_unused]],
		const TU1 tu1 [[maybe_unused]],
		const TU3& tu3 [[maybe_unused]],
		const rglv::BaryCoord& BS [[maybe_unused]],
		const rglv::BaryCoord& BP [[maybe_unused]],
		const VertexOutputMD& outs [[maybe_unused]],
		const rmlv::qfloat2& gl_FragCoord [[maybe_unused]],
		/* gl_FrontFacing, */
		const rmlv::qfloat& gl_FragDepth [[maybe_unused]],
		rmlv::qfloat4& gl_FragColor) {
		tu0->sample({ outs.envmapUV.x, outs.envmapUV.y }, gl_FragColor);
		gl_FragColor.w = 0.5F;
	} };


struct AmyProgram final : public rglv::BaseProgram {
	static constexpr int id = int(ShaderProgramId::Amy);

	struct VertexInput {
		rmlv::qfloat4 position;
		rmlv::qfloat4 normal;
		rmlv::qfloat4 uv; };

	struct Loader {
		Loader(const std::array<const void*, 4>& buffers,
		       const std::array<int, 4>& formats [[maybe_unused]]) :
			data_(*static_cast<const rglv::VertexArray_F3F3F3*>(buffers[0])) {
			assert(formats[0] == rglv::AF_VAO_F3F3F3);
			assert(buffers[0] != nullptr); }
		int Size() const { return data_.size(); }
		void LoadInstance(int, VertexInput&) {}
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
		VertexOutputMD Interpolate(rglv::BaryCoord BS [[maybe_unused]], rglv::BaryCoord BP [[maybe_unused]]) const {
			return { rglv::Interpolate(BP, uv) }; }
		rglv::VertexFloat3 uv; };

	static void ShadeVertex(
		const rglv::Matrices& mats,
		const rglv::BaseProgram::UniformsMD& u [[maybe_unused]],
		const VertexInput& v,
		rmlv::qfloat4& gl_Position,
		VertexOutputMD& outs) {
		outs.uv = v.uv.xyz();
		gl_Position = gl_ModelViewProjectionMatrix * v.position; }

	template <typename TU0, typename TU1, typename TU3>
	inline static void ShadeFragment(
		const rglv::Matrices& mats,
		const rglv::BaseProgram::UniformsMD& u,
		const TU0 tu0,
		const TU1 tu1,
		const TU3& tu3 [[maybe_unused]],
		const rglv::BaryCoord& BS,
		const rglv::BaryCoord& BP,
		const VertexOutputMD& outs,
		const rmlv::qfloat2& gl_FragCoord,
		/* gl_FrontFacing, */
		const rmlv::qfloat& gl_FragDepth,
		rmlv::qfloat4& gl_FragColor) {
		// gl_FragColor = { 0.5F, 0.5F, 0.5F, 1.0F };
		// fmt::print(std::cerr, "tu0 is {}\n", reinterpret_cast<const void*>(tu0));
		rmlv::qfloat2 tmp{ outs.uv.x, outs.uv.y };
		tu0->sample(tmp, gl_FragColor);
		} };


struct DepthProgram final : public rglv::BaseProgram {
	static constexpr int id = int(ShaderProgramId::Depth);

	struct VertexInput {
		rmlv::qfloat3 position;
		rmlv::qfloat3 normal;
		rmlv::qfloat2 uv; };

	struct Loader {
		Loader(const std::array<const void*, 4>& buffers,
		       const std::array<int, 4>& formats [[maybe_unused]]) :
			data_(*static_cast<const rglv::VertexArray_F3F3F3*>(buffers[0])) {
			assert(formats[0] == rglv::AF_VAO_F3F3F3);
			assert(buffers[0] != nullptr); }
		int Size() const { return data_.size(); }
		void LoadInstance(int, VertexInput&) {}
		void LoadMD(int idx, VertexInput& vi) {
			vi.position = data_.a0.load(idx);
			vi.normal   = data_.a1.load(idx);
			vi.uv       = data_.a2.loadxy(idx); }
		void LoadLane(int idx, int li, VertexInput& vi) {
			vi.position.setLane(li, data_.a0.at(idx));
			vi.normal  .setLane(li, data_.a1.at(idx));
			vi.uv      .setLane(li, data_.a2.at(idx).xy()); }
		const rglv::VertexArray_F3F3F3& data_; };

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
		rglv::VertexFloat2 uv;
		Interpolants(VertexOutputSD d0, VertexOutputSD d1, VertexOutputSD d2) :
			uv({ d0.uv, d1.uv, d2.uv }) {}
		VertexOutputMD Interpolate(rglv::BaryCoord BS [[maybe_unused]], rglv::BaryCoord BP) const {
			return { rglv::Interpolate(BP, uv) }; }};

	inline static void ShadeVertex(
		const rglv::Matrices& mats,
		const UniformsMD& u [[maybe_unused]],
		const VertexInput& v,
		rmlv::qfloat4& gl_Position,
		VertexOutputMD& outs) {
		outs.uv = v.uv;
		gl_Position = gl_ModelViewProjectionMatrix * rmlv::qfloat4{ v.position, 1.0F }; }

	template <typename TU0, typename TU1, typename TU3>
	inline static void ShadeFragment(
		const rglv::Matrices& mats,
		const UniformsMD& u,
		const TU0 tu0,
		const TU1 tu1,
		const TU3& tu3 [[maybe_unused]],
		const rglv::BaryCoord& BS,
		const rglv::BaryCoord& BP,
		const VertexOutputMD& outs,
		const rmlv::qfloat2& gl_FragCoord,
		/* gl_FrontFacing, */
		const rmlv::qfloat& gl_FragDepth,
		rmlv::qfloat4& gl_FragColor) {

		rmlv::qfloat zNear{ 10.0F };
		rmlv::qfloat zFar{ 1000.0F };

		rmlv::qfloat tmp;
		tu3.sample(outs.uv, tmp);

		tmp = (2.0F * zNear) / (zFar + zNear - tmp * (zFar - zNear));
		gl_FragColor = { tmp, tmp, tmp, 1.0F };
		} };


struct ManyProgram final : public rglv::BaseProgram {
	static constexpr int id = int(ShaderProgramId::Many);

	struct UniformsSD {
		float magic; };

	struct UniformsMD {
		rmlv::qfloat magic;

		UniformsMD(const UniformsSD& data) :
			magic(data.magic) {} };

	struct VertexInput {
		rmlv::qfloat4 position;
		rmlv::qfloat4 normal;
		rmlv::qfloat2 uv;
		rmlm::qmat4 imat; };

	struct Loader {
		Loader(const std::array<const void*, 4>& buffers,
		       const std::array<int, 4>& formats [[maybe_unused]]) :
			vbo_(*static_cast<const rglv::VertexArray_F3F3F3*>(buffers[0])),
			mats_(static_cast<const rmlm::mat4*>(buffers[1])) {
			assert(formats[0] == rglv::AF_VAO_F3F3F3);
			assert(buffers[0] != nullptr);
			assert(formats[1] == rglv::AF_FLOAT);
			assert(buffers[1] != nullptr); }
		int Size() const { return vbo_.size(); }
		void LoadInstance(int idx, VertexInput& vi) {
			vi.imat = rmlm::qmat4{ mats_[idx] }; }
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
		VertexOutputMD Interpolate(rglv::BaryCoord BS [[maybe_unused]], rglv::BaryCoord BP) const {
			return { rglv::Interpolate(BP, uv) }; }
		rglv::VertexFloat2 uv; };

	inline static void ShadeVertex(
		const rglv::Matrices& mats,
		const UniformsMD& u [[maybe_unused]],
		const VertexInput& v,
		rmlv::qfloat4& gl_Position,
		VertexOutputMD& outs) {
		rmlv::qfloat4 p1 = v.imat * v.position;
		outs.uv = v.uv;
		gl_Position = gl_ModelViewProjectionMatrix * p1; }

	template <typename TU0, typename TU1, typename TU3>
	inline static void ShadeFragment(
		const rglv::Matrices& mats,
		const UniformsMD& u,
		const TU0 tu0,
		const TU1 tu1,
		const TU3& tu3 [[maybe_unused]],
		const rglv::BaryCoord& BS,
		const rglv::BaryCoord& BP,
		const VertexOutputMD& outs,
		const rmlv::qfloat2& gl_FragCoord,
		/* gl_FrontFacing, */
		const rmlv::qfloat& gl_FragDepth,
		rmlv::qfloat4& gl_FragColor) {
		gl_FragColor = { outs.uv.x, outs.uv.y, u.magic, 1.0F }; } };


struct OBJ1Program final : public rglv::BaseProgram {
	static constexpr int id = int(ShaderProgramId::OBJ1);

	struct VertexInput {
		rmlv::qfloat4 position;
		rmlv::qfloat4 normal;
		rmlv::qfloat3 kd; };

	struct Loader {
		Loader(const std::array<const void*, 4>& buffers,
		       const std::array<int, 4>& formats [[maybe_unused]]) :
			vbo_(*static_cast<const rglv::VertexArray_F3F3F3*>(buffers[0])) {
			assert(formats[0] == rglv::AF_VAO_F3F3F3);
			assert(buffers[0] != nullptr); }
		int Size() const { return vbo_.size(); }
		void LoadInstance(int, VertexInput&) {}
		void LoadMD(int idx, VertexInput& vi) {
			vi.position = vbo_.a0.loadxyz1(idx);
			vi.normal   = vbo_.a1.loadxyz0(idx);
			vi.kd       = vbo_.a2.load(idx); }
		void LoadLane(int idx, int li, VertexInput& vi) {
			vi.position.setLane(li, rmlv::vec4{ vbo_.a0.at(idx), 1 });
			vi.normal  .setLane(li, rmlv::vec4{ vbo_.a1.at(idx), 0 });
			vi.kd      .setLane(li, vbo_.a2.at(idx)); }
		const rglv::VertexArray_F3F3F3& vbo_; };

	struct VertexOutputSD {
		rmlv::vec3 kd;
		static VertexOutputSD Mix(VertexOutputSD a, VertexOutputSD b, float t) {
			return { mix(a.kd, b.kd, t) }; }};

	struct VertexOutputMD {
		rmlv::qfloat3 kd;

		VertexOutputSD Lane(const int li) {
			return VertexOutputSD{
				kd.lane(li) }; }};

	struct Interpolants {
		Interpolants(VertexOutputSD d0, VertexOutputSD d1, VertexOutputSD d2) :
			kd({ d0.kd, d1.kd, d2.kd }) {}
		VertexOutputMD Interpolate(rglv::BaryCoord BS [[maybe_unused]], rglv::BaryCoord BP) const {
			return { rglv::Interpolate(BP, kd) }; }
		rglv::VertexFloat3 kd; };

	inline static void ShadeVertex(
		const rglv::Matrices& mats,
		const UniformsMD& u [[maybe_unused]],
		const VertexInput& v,
		rmlv::qfloat4& gl_Position,
		VertexOutputMD& outs) {

		// rmlv::qfloat4 lightPos{ 0.0F, 100.0F, 100.0F, 1.0F };
		// lightPos = mats.vm * lightPos;

		rmlv::qfloat4 lightPos{ 0.0F, 0.0F, 0.0F, 1.0F };

		auto mvv = mats.vm * v.position;
		auto mvn = mats.vm * v.normal;
		auto distance = rmlv::length(lightPos - mvv);
		auto lightVector = rmlv::normalize(lightPos - mvv);
		auto diffuse = vmax(dot(mvn, lightVector), 0.1F);
		// diffuse = diffuse * (1.0F / (1.0F + (0.02F * distance * distance)));
		diffuse = diffuse * (100.0F / distance);
		outs.kd = v.kd * diffuse;

		// outs.kd = v.kd;
		gl_Position = gl_ModelViewProjectionMatrix * v.position; }

	template <typename TU0, typename TU1, typename TU3>
	inline static void ShadeFragment(
		const rglv::Matrices& mats,
		const UniformsMD& u,
		const TU0 tu0,
		const TU1 tu1,
		const TU3& tu3 [[maybe_unused]],
		const rglv::BaryCoord& BS,
		const rglv::BaryCoord& BP,
		const VertexOutputMD& outs,
		const rmlv::qfloat2& gl_FragCoord,
		/* gl_FrontFacing, */
		const rmlv::qfloat& gl_FragDepth,
		rmlv::qfloat4& gl_FragColor) {
		gl_FragColor = { outs.kd, 1.0F }; } };


struct OBJ2Program final : public rglv::BaseProgram {
	static constexpr int id = int(ShaderProgramId::OBJ2);

	struct VertexInput {
		rmlv::qfloat4 position;
		rmlv::qfloat4 normal;
		rmlv::qfloat3 kd; };

	struct Loader {
		Loader(const std::array<const void*, 4>& buffers,
		       const std::array<int, 4>& formats [[maybe_unused]]) :
			vbo_(*static_cast<const rglv::VertexArray_F3F3F3*>(buffers[0])) {
			assert(formats[0] == rglv::AF_VAO_F3F3F3);
			assert(buffers[0] != nullptr); }
		int Size() const { return vbo_.size(); }
		void LoadInstance(int, VertexInput&) {}
		void LoadMD(int idx, VertexInput& vi) {
			vi.position = vbo_.a0.loadxyz1(idx);
			vi.normal   = vbo_.a1.loadxyz0(idx);
			vi.kd       = vbo_.a2.load(idx); }
		void LoadLane(int idx, int li, VertexInput& vi) {
			vi.position.setLane(li, rmlv::vec4{ vbo_.a0.at(idx), 1 });
			vi.normal  .setLane(li, rmlv::vec4{ vbo_.a1.at(idx), 0 });
			vi.kd      .setLane(li, vbo_.a2.at(idx)); }
		const rglv::VertexArray_F3F3F3& vbo_; };

	struct VertexOutputSD {
		rmlv::vec4 sp;
		rmlv::vec4 sn;
		rmlv::vec3 kd;
		static VertexOutputSD Mix(VertexOutputSD a, VertexOutputSD b, float t) {
			return { mix(a.sp, b.sp, t),
			         mix(a.sn, b.sn, t),
			         mix(a.kd, b.kd, t) }; }};

	struct VertexOutputMD {
		rmlv::qfloat4 sp;
		rmlv::qfloat4 sn;
		rmlv::qfloat3 kd;

		VertexOutputSD Lane(const int li) {
			return VertexOutputSD{
				sp.lane(li), sn.lane(li), kd.lane(li) }; }};

	struct Interpolants {
		Interpolants(VertexOutputSD d0, VertexOutputSD d1, VertexOutputSD d2) :
			sp({ d0.sp, d1.sp, d2.sp }),
			sn({ d0.sn, d1.sn, d2.sn }),
			kd({ d0.kd, d1.kd, d2.kd }) {}
		VertexOutputMD Interpolate(rglv::BaryCoord BS [[maybe_unused]], rglv::BaryCoord BP) const {
			return { rglv::Interpolate(BP, sp), rglv::Interpolate(BP, sn), rglv::Interpolate(BP, kd) }; }
		rglv::VertexFloat4 sp, sn;
		rglv::VertexFloat3 kd; };

	inline static void ShadeVertex(
		const rglv::Matrices& mats,
		const UniformsMD& u [[maybe_unused]],
		const VertexInput& v,
		rmlv::qfloat4& gl_Position,
		VertexOutputMD& outs) {
		outs.sp = mats.vm * v.position;
		outs.sn = mats.vm * v.normal;
		outs.kd = v.kd;
		gl_Position = gl_ModelViewProjectionMatrix * v.position; }

	template <typename TU0, typename TU1, typename TU3>
	inline static void ShadeFragment(
		const rglv::Matrices& mats,
		const UniformsMD& u,
		const TU0 tu0,
		const TU1 tu1,
		const TU3& tu3 [[maybe_unused]],
		const rglv::BaryCoord& BS,
		const rglv::BaryCoord& BP,
		const VertexOutputMD& outs,
		const rmlv::qfloat2& gl_FragCoord,
		/* gl_FrontFacing, */
		const rmlv::qfloat& gl_FragDepth,
		rmlv::qfloat4& gl_FragColor) {

		// rmlv::qfloat4 lightPos{ 0.0F, 100.0F, 100.0F, 1.0F };
		// lightPos = mats.vm * lightPos;
		rmlv::qfloat4 lightPos{ 0.0F, 0.0F, 0.0F, 1.0F };
		auto distance = rmlv::length(lightPos - outs.sp);
		auto lightVector = rmlv::normalize(lightPos - outs.sp);
		auto diffuse = vmax(dot(outs.sn, lightVector), 0.1F);
		// diffuse = diffuse * (1.0F / (1.0F + (0.02F * distance * distance)));
		diffuse = diffuse * (100.0F / distance);
		// outs.kd = v.kd * diffuse;
		gl_FragColor = { outs.kd * diffuse, 1.0F }; } };


struct OBJ2SProgram final : public rglv::BaseProgram {
	static constexpr int id = int(ShaderProgramId::OBJ2S);

	struct UniformsSD {
		rmlm::mat4 modelToShadow;
		rmlv::vec3 lpos;
		rmlv::vec3 ldir;
		float lcos; };

	struct UniformsMD {
		rmlm::qmat4 modelToShadow;
		rmlv::qfloat3 lpos;
		rmlv::qfloat3 ldir;
		rmlv::qfloat lcos;

		UniformsMD(const UniformsSD& data) :
			modelToShadow(data.modelToShadow),
			lpos(data.lpos),
			ldir(data.ldir),
			lcos(data.lcos) {} };

	struct VertexInput {
		rmlv::qfloat3 position;
		rmlv::qfloat3 normal;
		rmlv::qfloat3 kd; };

	struct Loader {
		Loader(const std::array<const void*, 4>& buffers,
		       const std::array<int, 4>& formats [[maybe_unused]]) :
			vbo_(*static_cast<const rglv::VertexArray_F3F3F3*>(buffers[0])) {
			assert(formats[0] == rglv::AF_VAO_F3F3F3);
			assert(buffers[0] != nullptr); }
		int Size() const { return vbo_.size(); }
		void LoadInstance(int, VertexInput&) {}
		void LoadMD(int idx, VertexInput& vi) {
			vi.position = vbo_.a0.load(idx);
			vi.normal   = vbo_.a1.load(idx);
			vi.kd       = vbo_.a2.load(idx); }
		void LoadLane(int idx, int li, VertexInput& vi) {
			vi.position.setLane(li, vbo_.a0.at(idx));
			vi.normal  .setLane(li, vbo_.a1.at(idx));
			vi.kd      .setLane(li, vbo_.a2.at(idx)); }
		const rglv::VertexArray_F3F3F3& vbo_; };

	struct VertexOutputSD {
		rmlv::vec4 sp;
		rmlv::vec4 sn;
		rmlv::vec3 kd;
		rmlv::vec4 lp;
		static VertexOutputSD Mix(VertexOutputSD a, VertexOutputSD b, float t) {
			return { mix(a.sp, b.sp, t),
			         mix(a.sn, b.sn, t),
			         mix(a.kd, b.kd, t),
			         mix(a.lp, b.lp, t) }; }};

	struct VertexOutputMD {
		rmlv::qfloat4 sp;
		rmlv::qfloat4 sn;
		rmlv::qfloat3 kd;
		rmlv::qfloat4 lp;

		VertexOutputSD Lane(const int li) {
			return VertexOutputSD{
				sp.lane(li), sn.lane(li), kd.lane(li), lp.lane(li) }; }};

	struct Interpolants {
		rglv::VertexFloat4 sp, sn;
		rglv::VertexFloat3 kd;
		rglv::VertexFloat4 lp;

		Interpolants(VertexOutputSD d0, VertexOutputSD d1, VertexOutputSD d2) :
			sp({ d0.sp, d1.sp, d2.sp }),
			sn({ d0.sn, d1.sn, d2.sn }),
			kd({ d0.kd, d1.kd, d2.kd }),
			lp({ d0.lp, d1.lp, d2.lp }) {}
		VertexOutputMD Interpolate(rglv::BaryCoord BS [[maybe_unused]], rglv::BaryCoord BP) const {
			return {
				rglv::Interpolate(BP, sp),
				rglv::Interpolate(BP, sn),
				rglv::Interpolate(BP, kd),
				rglv::Interpolate(BP, lp) }; } };

	inline static void ShadeVertex(
		const rglv::Matrices& mats,
		const UniformsMD& u [[maybe_unused]],
		const VertexInput& v,
		rmlv::qfloat4& gl_Position,
		VertexOutputMD& outs) {
		outs.sp = mats.vm * rmlv::qfloat4{ v.position, 1 };
		outs.sn = mats.nm * rmlv::qfloat4{ v.normal, 1 };
		outs.kd = v.kd;
		outs.lp = u.modelToShadow * rmlv::qfloat4{ v.position, 1 };
		gl_Position = gl_ModelViewProjectionMatrix * rmlv::qfloat4{ v.position, 1 }; }

	template <typename TU0, typename TU1, typename TU3>
	inline static void ShadeFragment(
		const rglv::Matrices& mats,
		const UniformsMD& u,
		const TU0 tu0,
		const TU1 tu1,
		const TU3& tu3 [[maybe_unused]],
		const rglv::BaryCoord& BS,
		const rglv::BaryCoord& BP,
		const VertexOutputMD& outs,
		const rmlv::qfloat2& gl_FragCoord,
		/* gl_FrontFacing, */
		const rmlv::qfloat& gl_FragDepth,
		rmlv::qfloat4& gl_FragColor) {

		auto surfaceToLight = normalize(u.lpos - outs.sp.xyz());
		auto distanceToLight = length(u.lpos - outs.sp.xyz());
		// auto attenuation = 1.0F / (1.0F + 0.1F * (distanceToLight*distanceToLight));
		auto attenuation = 100.0F / distanceToLight;

		auto lightToSurfaceAngle = dot(-surfaceToLight, normalize(u.ldir));
		// std::cout << "ltsa: " << lightToSurfaceAngle.get_x() << "\n";
		attenuation = SelectFloat(attenuation, 0.111F, cmplt(lightToSurfaceAngle, u.lcos));

		auto diffuseFactor = vmax(0.0F, dot(normalize(outs.sn.xyz()), surfaceToLight));

		auto lpx = outs.lp.x / outs.lp.w;
		auto lpy = outs.lp.y / outs.lp.w;
		auto lpz = outs.lp.z / outs.lp.w;
		rmlv::qfloat tmp;
		tu3.sample(rmlv::qfloat2{ lpx, lpy }*0.5F+0.5F, tmp);
		// tu3.sample(rmlv::qfloat2{ lpx, lpy }, tmp);

		attenuation = SelectFloat(attenuation, 0.111F, cmpgt(lpz-0.0005F, tmp));

		gl_FragColor = { attenuation * outs.kd * diffuseFactor, 1.0F }; } };


void Install(class rqdq::rglv::GPU& gpu);


}  // namespace rqv
}  // namespace rqdq

#undef gl_ModelViewMatrix
#undef gl_ModelViewProjectionMatrix
#undef gl_NormalMatrix
#undef gl_ProjectionMatrix
