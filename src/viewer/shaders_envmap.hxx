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

#include <array>

#define gl_ModelViewMatrix mats.vm
#define gl_ModelViewProjectionMatrix mats.vpm
#define gl_NormalMatrix mats.nm
#define gl_ProjectionMatrix mats.pm

namespace rqdq {
namespace rqv {

struct EnvmapProgram final : public rglv::BaseProgram {
	static constexpr int id = 10;

	struct VertexInput {
		rmlv::qfloat3 position;
		rmlv::qfloat3 smoothNormal;
		rmlv::qfloat3 faceNormal; };

	struct Loader {
		const std::array<const float*, 16> ptrs_;
		Loader(const std::array<const float*, 16>& buffers) :
			ptrs_(buffers) {}
		// int Size() const { return data_.size(); }
		void LoadInstance(int, VertexInput&) {}
		void LoadMD(int idx, VertexInput& vi) {
			if (ptrs_[0]) {
				vi.position.x = _mm_load_ps(&ptrs_[0][idx]);
				vi.position.y = _mm_load_ps(&ptrs_[1][idx]);
				vi.position.z = _mm_load_ps(&ptrs_[2][idx]); }
			else {
				vi.position.x = vi.position.y = vi.position.z = _mm_setzero_ps(); }

			if (ptrs_[3]) {
				vi.smoothNormal.x = _mm_load_ps(&ptrs_[3][idx]);
				vi.smoothNormal.y = _mm_load_ps(&ptrs_[4][idx]);
				vi.smoothNormal.z = _mm_load_ps(&ptrs_[5][idx]); }
			else {
				vi.smoothNormal.x = vi.smoothNormal.y = _mm_setzero_ps(); vi.smoothNormal.z = _mm_set1_ps(1.0F); }

			if (ptrs_[6]) {
				vi.faceNormal.x = _mm_load_ps(&ptrs_[6][idx]);
				vi.faceNormal.y = _mm_load_ps(&ptrs_[7][idx]);
				vi.faceNormal.z = _mm_load_ps(&ptrs_[8][idx]); }
			else {
				vi.faceNormal.x = vi.faceNormal.y = vi.faceNormal.z = _mm_set1_ps(1.0F); }}

		void LoadLane(int idx, int li, VertexInput& vi) {
			if (ptrs_[0]) {
				vi.position.setLane(li, rmlv::vec3{ ptrs_[0][idx], ptrs_[1][idx], ptrs_[2][idx] }); }
			else {
				vi.position.setLane(li, rmlv::vec3{ 0, 0, 0 }); }

			if (ptrs_[3]) {
				vi.smoothNormal.setLane(li, rmlv::vec3{ ptrs_[3][idx], ptrs_[4][idx], ptrs_[5][idx] }); }
			else {
				vi.smoothNormal.setLane(li, rmlv::vec3{ 0, 0, 1 }); }

			if (ptrs_[6]) {
				vi.faceNormal.setLane(li, rmlv::vec3{ ptrs_[6][idx], ptrs_[7][idx], ptrs_[8][idx] }); }
			else {
				vi.faceNormal.setLane(li, rmlv::vec3{ 1, 1, 1 }); }}};

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

		rmlv::qfloat4 position = gl_ModelViewMatrix * rmlv::qfloat4{ v.position, 1 };
		rmlv::qfloat3 e = normalize(position.xyz());
		rmlv::qfloat3 vn = mix(v.smoothNormal, v.faceNormal, 0.0F); // in_uniform.roughness);
		rmlv::qfloat3 n = normalize(mul_w0(gl_NormalMatrix, vn));
		rmlv::qfloat3 r = rglv::reflect(e, n);

		rmlv::qfloat m = rmlv::qfloat{2.0F} * sqrt((r.x*r.x) + (r.y*r.y) + ((r.z + 1.0F)*(r.z + 1.0F)));
		rmlv::qfloat uu = r.x / m + 0.5F;
		rmlv::qfloat vv = r.y / m + 0.5F;
		out.envmapUV = { uu, vv };
		gl_Position = gl_ModelViewProjectionMatrix * rmlv::qfloat4{ v.position, 1 }; }

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
		rmlv::qfloat4& gl_FragColor,
		rmlv::mvec4i& gl_triMask [[maybe_unused]]) {
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
