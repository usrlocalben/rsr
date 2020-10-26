#pragma once
#include <cassert>

#include "src/rgl/rglv/rglv_gpu_protocol.hxx"
#include "src/rgl/rglv/rglv_interpolate.hxx"
#include "src/rgl/rglv/rglv_vao.hxx"
#include "src/rml/rmlm/rmlm_soa.hxx"
#include "src/rml/rmlv/rmlv_soa.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"

namespace rqdq {
namespace rglv {

struct Matrices {
	rmlm::qmat4 vm;
	rmlm::qmat4 pm;
	rmlm::qmat4 nm;
	rmlm::qmat4 vpm; };


struct BaseProgram {
	static constexpr int id = 0;

	struct UniformsSD {};
	struct UniformsMD {
		UniformsMD(const UniformsSD&) {} };

	struct VertexInput {
		rmlv::qfloat3 a0; };

	struct Loader {
		const std::array<const float*, 16> ptrs_;
		Loader(const std::array<const float*, 16>& buffers) :
			ptrs_(buffers) {}
		void LoadInstance(int, VertexInput&) {}
		void LoadMD(int idx, VertexInput& vi) {
			if (ptrs_[0]) {
				vi.a0.x = _mm_load_ps(&ptrs_[0][idx]);
				vi.a0.y = _mm_load_ps(&ptrs_[1][idx]);
				vi.a0.z = _mm_load_ps(&ptrs_[2][idx]); }
			else {
				vi.a0.x = vi.a0.y = vi.a0.z = _mm_setzero_ps(); }}

		void LoadLane(int idx, int li, VertexInput& vi) {
			if (ptrs_[0]) {
				vi.a0.setLane(li, rmlv::vec3{ ptrs_[0][idx], ptrs_[1][idx], ptrs_[2][idx] }); }
			else {
				vi.a0.setLane(li, rmlv::vec3{ 0, 0, 0 }); }}};

	struct VertexOutputSD {
		static VertexOutputSD Mix(VertexOutputSD, VertexOutputSD, float) {
			return {}; }};

	struct VertexOutputMD {
		VertexOutputSD Lane(int) const {
			return {}; } };

	struct Interpolants {
		Interpolants() = default;
		Interpolants(VertexOutputSD, VertexOutputSD, VertexOutputSD) {}
		VertexOutputMD Interpolate(rglv::BaryCoord BS [[maybe_unused]], rglv::BaryCoord BP [[maybe_unused]]) const {
			return {}; } };

	inline static void ShadeVertex(
		const Matrices& m,
		const UniformsMD& u [[maybe_unused]],
		const VertexInput& v,
		rmlv::qfloat4& gl_Position,
		VertexOutputMD& outs [[maybe_unused]]) {
		gl_Position = m.vpm * rmlv::qfloat4{ v.a0, 1.0F }; }

	template <typename TU0, typename TU1, typename TU3>
	inline static void ShadeFragment(
		const Matrices& m [[maybe_unused]],
		const UniformsMD& u [[maybe_unused]], 
		const TU0 tu0 [[maybe_unused]],
		const TU1 tu1 [[maybe_unused]],
		const TU3& tu3 [[maybe_unused]],
		const rglv::BaryCoord& BS [[maybe_unused]],
		const rglv::BaryCoord& BP [[maybe_unused]],
		const VertexOutputMD& v [[maybe_unused]],
		const rmlv::qfloat2& gl_FragCoord [[maybe_unused]],
		/* gl_FrontFacing, */
		const rmlv::qfloat& gl_FragDepth [[maybe_unused]],
		rmlv::qfloat4& gl_FragColor
		) {
		gl_FragColor = rmlv::mvec4f{1.0F}; }

	inline static rmlv::qfloat3 ShadeCanvas(
		rmlv::qfloat2 gl_FragCoord [[maybe_unused]],
		rmlv::qfloat3 source
		) {
		return source; } };


}  // namespace rglv
}  // namespace rqdq
