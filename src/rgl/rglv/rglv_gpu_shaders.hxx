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
		UniformsMD(const UniformsSD& data) {} };

	struct VertexInput {
		rmlv::qfloat4 a0; };

	struct Loader {
		Loader(const std::array<const void*, 4>& buffers, const std::array<int, 4>& formats) :
			data_(*static_cast<const rglv::VertexArray_F3F3F3*>(buffers[0])) {
				assert(formats[0] == AF_VAO_F3F3F3);
				assert(buffers[0] != nullptr); }
		int Size() const { return data_.size(); }
		void LoadInstance(int id, VertexInput& vi) {}
		void LoadMD(int idx, VertexInput& vi) {
			vi.a0 = data_.a0.loadxyz1(idx); }
		/*void LoadOne(int idx, VertexInput& vi) {
			vi.a0 = rmlv::vec4{ data_.a0.at(idx), 1 }; }*/
		void LoadLane(int idx, int li, VertexInput& vi) {
			vi.a0.setLane(li, rmlv::vec4{ data_.a0.at(idx), 1 }); }
		const rglv::VertexArray_F3F3F3& data_; };

	struct VertexOutputSD {
		static VertexOutputSD Mix(VertexOutputSD a, VertexOutputSD b, float t) {
			return {}; }};

	struct VertexOutputMD {
		VertexOutputSD Lane(const int li) {
			return {}; } };

	struct Interpolants {
		Interpolants(VertexOutputSD d0, VertexOutputSD d1, VertexOutputSD d2) {}
		VertexOutputMD Interpolate(rglv::BaryCoord bary) const {
			return {}; } };

	inline static void ShadeVertex(
		const Matrices& m,
		const UniformsMD& u,
		const VertexInput& v,
		rmlv::qfloat4& gl_Position,
		VertexOutputMD& outs
		) {
		gl_Position = m.vpm * v.a0; }

	template <typename TEXTURE_UNIT>
	inline static void ShadeFragment(
		const Matrices& m,
		const UniformsMD& u,
		const TEXTURE_UNIT& tu0,
		const TEXTURE_UNIT& tu1,
		const rglv::BaryCoord& bary,
		const VertexOutputMD& v,
		const rmlv::qfloat2& gl_FragCoord,
		/* gl_FrontFacing, */
		const rmlv::qfloat& gl_FragDepth,
		rmlv::qfloat4& gl_FragColor
		) {
		gl_FragColor = rmlv::mvec4f{1.0F}; }

	inline static rmlv::qfloat3 ShadeCanvas(
		rmlv::qfloat2 gl_FragCoord,
		rmlv::qfloat3 source
		) {
		return source; } };


}  // namespace rglv
}  // namespace rqdq
