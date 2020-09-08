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

struct EnvmapProgram final : public rglv::BaseProgram {
	static constexpr int id = 620953;

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

		VertexOutputSD Lane(const int li) const {
			return VertexOutputSD{
				envmapUV.lane(li) }; }};

	struct Interpolants {
		Interpolants() = default;
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


void InstallEnvmap(class rqdq::rglv::GPU& gpu);


}  // namespace rqv
}  // namespace rqdq

#undef gl_ModelViewMatrix
#undef gl_ModelViewProjectionMatrix
#undef gl_NormalMatrix
#undef gl_ProjectionMatrix
