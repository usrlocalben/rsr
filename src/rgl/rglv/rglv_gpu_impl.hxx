#pragma once
#include <algorithm>
#include <array>
#include <cassert>
#include <iostream>
#include <memory_resource>
#include <numeric>
#include <optional>
#include <string>
#include <vector>

#include "src/rcl/rclmt/rclmt_jobsys.hxx"
#include "src/rcl/rclr/rclr_algorithm.hxx"
#include "src/rgl/rglr/rglr_algorithm.hxx"
#include "src/rgl/rglr/rglr_canvas.hxx"
#include "src/rgl/rglr/rglr_canvas_util.hxx"
#include "src/rgl/rglr/rglr_texture_sampler.hxx"
#include "src/rgl/rglv/rglv_gl.hxx"
#include "src/rgl/rglv/rglv_gpu.hxx"
#include "src/rgl/rglv/rglv_gpu_protocol.hxx"
#include "src/rgl/rglv/rglv_gpu_shaders.hxx"
#include "src/rgl/rglv/rglv_interpolate.hxx"
#include "src/rgl/rglv/rglv_math.hxx"
#include "src/rgl/rglv/rglv_triangle.hxx"
#include "src/rgl/rglv/rglv_vao.hxx"
#include "src/rgl/rglv/rglv_view_frustum.hxx"
#include "src/rml/rmlg/rmlg_irect.hxx"
#include "src/rml/rmlg/rmlg_triangle.hxx"
#include "src/rml/rmlm/rmlm_mat4.hxx"
#include "src/rml/rmlm/rmlm_soa.hxx"
#include "src/rml/rmlv/rmlv_soa.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"

#include "3rdparty/fmt/include/fmt/printf.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4701) // used potentially uninitialized var
#endif

namespace rqdq {
namespace {

inline
auto MakeMatrices(const rglv::GLState& s) -> rglv::Matrices {
	rglv::Matrices m;
	m.vm = rmlm::qmat4{ s.viewMatrix };
	m.pm = rmlm::qmat4{ s.projectionMatrix };
	m.nm = rmlm::qmat4{ transpose(inverse(s.viewMatrix)) };
	m.vpm = rmlm::qmat4{ s.projectionMatrix * s.viewMatrix };
	return m; }

inline
int FindAndClearLSB(uint32_t& x) {
	unsigned long idx;
	_BitScanForward(&idx, x);
	x &= x - 1;
	return idx; }

}  // close unnamed namespace

namespace rglv {

struct DepthLT {
	auto operator()(const rmlv::mvec4f& fragDepth, const rmlv::mvec4f& destDepth) const -> rmlv::mvec4f {
		return cmplt(fragDepth, destDepth); } };

struct DepthLE {
	auto operator()(const rmlv::mvec4f& fragDepth, const rmlv::mvec4f& destDepth) const -> rmlv::mvec4f {
		return cmple(fragDepth, destDepth); } };

struct DepthEQ {
	auto operator()(const rmlv::mvec4f& fragDepth, const rmlv::mvec4f& destDepth) const -> rmlv::mvec4f {
		return cmpeq(fragDepth, destDepth); } };

struct BlendOff {
	auto operator()(const rmlv::qfloat4& fragColor, const rmlv::qfloat3& destColor [[maybe_unused]] ) const -> rmlv::qfloat3 {
		return fragColor.xyz(); } };

struct BlendAlpha {
	auto operator()(const rmlv::qfloat4& fragColor, const rmlv::qfloat3& destColor [[maybe_unused]] ) const -> rmlv::qfloat3 {
		auto alpha = fragColor.w;
		auto oneMinusAlpha = rmlv::mvec4f{1.0F} - alpha;
		return fragColor.xyz()*alpha + destColor*oneMinusAlpha; }};

template <typename SHADER,
          typename COLOR_IO, typename DEPTH_IO,
          typename TU0, typename TU1, typename TU3,
          bool DEPTH_TEST, typename DEPTH_FUNC, bool DEPTH_WRITEMASK, bool COLOR_WRITEMASK, typename BLEND_FUNC>
struct TriangleProgram {
	COLOR_IO cc_;
	DEPTH_IO dc_;
	const TU0 tu0_;
	const TU1 tu1_;
	const TU3& tu3_;
	const Matrices matrices_;
	const typename SHADER::UniformsMD uniforms_;

	rglv::VertexFloat1 invW_;
	rglv::VertexFloat1 ndcZ_;
	typename SHADER::Interpolants vo_;
	// XXX rmlv::mvec4i backfacing_;

	const rmlv::mvec4f vw0_, vw1_, vw2_;
	const rmlv::mvec4f vz0_, vz1_, vz2_;
	const typename SHADER::VertexOutputMD vd0_, vd1_, vd2_;
	// XXX const rmlv::mvec4i vbf_;

	TriangleProgram(
		COLOR_IO cc, DEPTH_IO dc,
		const TU0 tu0, const TU1 tu1, const TU3& tu3,
		Matrices matrices,
		typename SHADER::UniformsMD uniforms,
		VertexFloat1 invW_,
		VertexFloat1 ndcZ_,
		typename SHADER::VertexOutputSD computed0, typename SHADER::VertexOutputSD computed1, typename SHADER::VertexOutputSD computed2) :
		cc_(cc), dc_(dc),
		tu0_(tu0), tu1_(tu1), tu3_(tu3),
		matrices_(matrices),
		uniforms_(uniforms),
		// XXX backfacing,
		invW_(invW_),
		ndcZ_(ndcZ_),
		vo_(computed0, computed1, computed2)
		{}

	TriangleProgram(
		COLOR_IO cc, DEPTH_IO dc,
		const TU0 tu0, const TU1 tu1, const TU3& tu3,
		Matrices matrices,
		typename SHADER::UniformsMD uniforms,
		// XXX rmlv::mvec4i vbf,
		rmlv::mvec4f vw0, rmlv::mvec4f vw1, rmlv::mvec4f vw2,
		rmlv::mvec4f vz0, rmlv::mvec4f vz1, rmlv::mvec4f vz2,
		typename SHADER::VertexOutputMD vd0, typename SHADER::VertexOutputMD vd1, typename SHADER::VertexOutputMD vd2) :
		cc_(cc), dc_(dc),
		tu0_(tu0), tu1_(tu1), tu3_(tu3),
		matrices_(matrices),
		uniforms_(uniforms),
		// XXX vbf_(vbf),
		vw0_(vw0), vw1_(vw1), vw2_(vw2),
		vz0_(vz0), vz1_(vz1), vz2_(vz2),
		vd0_(vd0), vd1_(vd1), vd2_(vd2)
		{}

	void Lane(int i) {
		// XXX backfacing_ = vbf_.si[i];
		invW_ = VertexFloat1{ vw0_.lane[i], vw1_.lane[i], vw2_.lane[i] };
		ndcZ_ = VertexFloat1{ vz0_.lane[i], vz1_.lane[i], vz2_.lane[i] };
		vo_ = typename SHADER::Interpolants{ vd0_.Lane(i), vd1_.Lane(i), vd2_.Lane(i) }; }

	void Begin(int x, int y) {
		cc_.Begin(x, y);
		dc_.Begin(x, y); }

	void CR() {
		cc_.CR();
		dc_.CR(); }

	void Right2() {
		cc_.Right2();
		dc_.Right2(); }

	void Render(const rmlv::qfloat2 fragCoord, const rmlv::mvec4i triMask, rglv::BaryCoord BS) {
		using rmlv::qfloat, rmlv::qfloat3, rmlv::qfloat4;

		const auto fragDepth = rglv::Interpolate(BS, ndcZ_);
		qfloat destDepth;
		rmlv::mvec4i fragMask;
		if (DEPTH_TEST) {
			dc_.Load(destDepth);
			const auto depthMask = float2bits(DEPTH_FUNC{}(fragDepth, destDepth));
			fragMask = andnot(triMask, depthMask);
			if (movemask(bits2float(fragMask)) == 0) {
				// early out if whole quad fails depth test
				return; }}
		else {
			fragMask = andnot(triMask, float2bits(rmlv::mvec4f::all_ones())); }

		// restore perspective
		if (COLOR_WRITEMASK) {
			const auto fragW = rmlv::oneover(Interpolate(BS, invW_));
			rglv::BaryCoord BP;
			BP.x = invW_.v0 * BS.x * fragW;
			BP.z = invW_.v2 * BS.z * fragW;
			BP.y = 1.0f - BP.x - BP.z;

			auto attrs = vo_.Interpolate(BS, BP);

			qfloat4 fragColor;
			SHADER::ShadeFragment(
				matrices_,
				uniforms_,
				tu0_, tu1_, tu3_,
				BS, BP, attrs,
				fragCoord,
				/*frontFacing,*/
				fragDepth,
				fragColor);

		// XXX late-Z should happen here <----

			qfloat3 destColor;
			cc_.Load(destColor);
			qfloat3 blendedColor = BLEND_FUNC()(fragColor, destColor);
			cc_.Store(destColor, blendedColor, fragMask); }

		if (DEPTH_WRITEMASK) {
			dc_.Store(destDepth, fragDepth, fragMask); }}};


template <typename SHADER>
class GPUBltImpl : GPU {

	void StoreTrueColor(const GLState& state, const void* uniformsPtr, const rmlg::irect rect, const bool enableGamma, rglr::TrueColorCanvas& outcanvas) {
		// XXX add support for uniforms in stream-out shader
		if (state.color0AttachmentType == RB_COLOR_DEPTH || state.color0AttachmentType == RB_RGBAF32) {
			auto ptr = reinterpret_cast<rmlv::qfloat4*>(color0Buf_.data() + kTileColorSizeInBytes*rclmt::jobsys::threadId);
			rglr::QFloat4Canvas cc{ tileDimensionsInPixels_.x, tileDimensionsInPixels_.y, ptr, 64 };
			if (enableGamma) {
				rglr::FilterTile<SHADER, rglr::sRGB>(cc, outcanvas, rect); }
			else {
				rglr::FilterTile<SHADER, rglr::LinearColor>(cc, outcanvas, rect); }}
		else if (state.color0AttachmentType == RB_RGBF32) {
			auto ptr = reinterpret_cast<rmlv::qfloat3*>(color0Buf_.data() + kTileColorSizeInBytes*rclmt::jobsys::threadId);
			rglr::QFloat3Canvas cc{ tileDimensionsInPixels_.x, tileDimensionsInPixels_.y, ptr, 64 };
			if (enableGamma) {
				rglr::FilterTile<SHADER, rglr::sRGB>(cc, outcanvas, rect); }
			else {
				rglr::FilterTile<SHADER, rglr::LinearColor>(cc, outcanvas, rect); }}
		else {
			std::cerr << "not implemented: tile_StoreTrueColor for color attachment type " << state.color0AttachmentType << "\n";
			std::exit(1); }}

public:
	static
	auto MakeBltProgramPtrs() -> rglv::BltProgramPtrs {
		rglv::BltProgramPtrs out;
		out.StoreTrueColor = static_cast<decltype(out.StoreTrueColor)>(&GPUBltImpl::StoreTrueColor);
		return out; } };


template <typename SHADER>
class GPUBinImpl : GPU {

	void DrawArraysN(int count, int instanceCnt) {
		struct SequenceSource {
			int operator()(int ti) { return ti; }};
		SequenceSource indexSource{};
		BinTriangles2P<true>(count, indexSource, instanceCnt); }

	void DrawArrays1(int count) {
		struct SequenceSource {
			int operator()(int ti) { return ti; }};
		SequenceSource indexSource{};
		BinTriangles2P<false>(count, indexSource, 1); }

	void DrawElements1(int count, uint16_t* indices, uint8_t hint) {
		if ((hint & RGL_HINT_DENSE) && (hint & RGL_HINT_READ4)) {
			// special case:
			// vertex data length is a multiple of 4 (SSE ready), and
			// most/all of the vertex data will be used.
			//
			// in this case, it's better to run the vertex shader
			// on the entire buffer first, then process the primitives
			struct ArraySource {
				ArraySource(const uint16_t* data) : data_(data) {}
				int operator()(int ti) { return data_[ti]; }
				const uint16_t* const data_; };
			ArraySource indexSource{indices};
			BinTriangles2P<false>(count, indexSource, 1); }
		else {
			struct ArraySource {
				ArraySource(const uint16_t* data) : data_(data) {}
				int operator()(int ti) { return data_[ti]; }
				const uint16_t* const data_; };
			ArraySource indexSource{indices};
			BinTriangles1P<false>(count, indexSource, 1); }}

	void DrawElementsN(int count, uint16_t* indices, int instanceCnt) {
			struct ArraySource {
				ArraySource(const uint16_t* data) : data_(data) {}
				int operator()(int ti) { return data_[ti]; }
				const uint16_t* const data_; };
			ArraySource indexSource{indices};
			BinTriangles1P<true>(count, indexSource, instanceCnt); }

	/**
	 * transform and bin triangles, 2-phase impl
	 * p1: transform, clip-test whole v.buffer to temp storage
	 * p2: gather from temp store and process primitives 
	 *
	 * preferred when
	 *  - most or all of the vertex-buffer is used, and
	 *  - most vertices are shared by multiple primitives
	 *    (e.g. solid geometry)
	 */
	template <bool INSTANCED, typename INDEX_SOURCE>
	void BinTriangles2P(const int count, INDEX_SOURCE& indexSource, const int instanceCnt) {
		using std::min, std::max, std::swap;
		using rmlv::ivec2, rmlv::qfloat, rmlv::qfloat2, rmlv::qfloat3, rmlv::qfloat4, rmlm::qmat4;
		const auto& state = *binState;

		const auto matrices = MakeMatrices(state);
		const typename SHADER::UniformsMD uniforms(*static_cast<const typename SHADER::UniformsSD*>(binUniforms));
		const auto cullingEnabled = state.cullingEnabled;
		const auto cullFace = state.cullFace;
		typename SHADER::Loader loader( state.buffers, state.bufferFormat );
		const auto frustum = ViewFrustum{ bufferDimensionsInPixels_ };

		typename SHADER::VertexInput vertex;

		clipQueue_.clear();
		int paddedSize = (loader.Size() + 3) & (~0x3);
		clipFlagBuffer_.reserve(paddedSize);
		devCoordXBuffer_.reserve(paddedSize);
		devCoordYBuffer_.reserve(paddedSize);

		const auto scissorRect = ScissorRect(state);

		const auto [DS, DO] = DSDO(state);

		const rmlv::mvec4i tlx{ scissorRect.top_left.x };
		const rmlv::mvec4i tly{ scissorRect.top_left.y };
		const rmlv::mvec4i brx{ scissorRect.bottom_right.x-1 };
		const rmlv::mvec4i bry{ scissorRect.bottom_right.y-1 };

		const auto keepBacks = bits2float(rmlv::mvec4i{ cullingEnabled && (cullFace == GL_BACK) ? 0 : -1 });
		const auto keepFronts = bits2float(rmlv::mvec4i{ cullingEnabled && (cullFace == GL_FRONT) ? 0 : -1 });

		if (!INSTANCED) {
			// coding error if this is not true
			// XXX could use constexpr instead of template param?
			assert(instanceCnt == 1); }

		const auto cmd = INSTANCED ? CMD_DRAW_INLINE_INSTANCED : CMD_DRAW_INLINE;
		for (auto& h : tilesHead_) {
			AppendByte(&h, cmd);
			Mark(&h); }

		for (int iid=0; iid<instanceCnt; ++iid) {
			if (INSTANCED) {
				loader.LoadInstance(iid, vertex); }

			const auto siz = loader.Size();
			// xxx const int rag = siz % 4;  assume vaos are always padded to size()%4=0

			// rmlv::mvec4i axFlags{ -1 };
			for (int vi=0; vi<siz; vi+=4) {
				loader.LoadMD(vi, vertex);

				qfloat4 coord;
				typename SHADER::VertexOutputMD unused;
				SHADER::ShadeVertex(matrices, uniforms, vertex, coord, unused);

				auto flags = frustum.Test(coord);
				// axFlags = axFlags & flags;
				store_bytes(clipFlagBuffer_.data() + vi, flags);

				auto devCoord = pdiv(coord).xy() * DS + DO;
				devCoord.template store<false>(devCoordXBuffer_.data() + vi,
				                               devCoordYBuffer_.data() + vi); }


			int numPrims{count / 3};
			int numLanes{4};
			int laneMask{ (1<<4)-1 };
			for (int pi=0; pi<numPrims; pi+=4) {
				if (pi + 4 > numPrims) {
					numLanes = numPrims - pi;
					laneMask = (1<<numLanes)-1; }

				rmlv::qfloat2 dc0, dc1, dc2;
				rmlv::mvec4i ii0, ii1, ii2;
				rmlv::mvec4i cf0, cf1, cf2;

				// gather
				for (int li=0; li<numLanes; ++li) {

					int ti = (pi+li)*3;
					auto i0 = static_cast<uint16_t>(indexSource(ti));
					auto i1 = static_cast<uint16_t>(indexSource(ti+1));
					auto i2 = static_cast<uint16_t>(indexSource(ti+2));

					ii0.si[li] = i0;
					ii1.si[li] = i1;
					ii2.si[li] = i2;

					cf0.si[li] = clipFlagBuffer_[i0];
					cf1.si[li] = clipFlagBuffer_[i1];
					cf2.si[li] = clipFlagBuffer_[i2];

					dc0.setLane(li, { devCoordXBuffer_.data()[i0], devCoordYBuffer_.data()[i0] });
					dc1.setLane(li, { devCoordXBuffer_.data()[i1], devCoordYBuffer_.data()[i1] });
					dc2.setLane(li, { devCoordXBuffer_.data()[i2], devCoordYBuffer_.data()[i2] }); }

				/*
				 some-out  all-out
				     0        0  = normal tri
				     1        0  = needs clipping
				     0        1  = impossible
				     1        1  = cull
				*/
				auto pointsOutside = cmpgt(cf0|cf1|cf2, rmlv::mvec4i::zero());
				auto primsOutside  = cmpgt(cf0&cf1&cf2, rmlv::mvec4i::zero());
				auto intersectsPlanes = andnot(primsOutside, pointsOutside);

				if ((_mm_movemask_ps(bits2float(primsOutside).v) & laneMask) == laneMask) {
					continue; }
				uint32_t needClip = _mm_movemask_ps(bits2float(intersectsPlanes).v) & laneMask;
				while (needClip) {
					int li = FindAndClearLSB(needClip);
					clipQueue_.push_back({ iid, ii0.si[li], ii1.si[li], ii2.si[li] }); }

				// handle backfacing tris and culling
				auto area = rmlg::Area(dc0, dc1, dc2);
				auto front = cmpgt(area, rmlv::mvec4f::zero());

				/*
				 * alternative:
				 * reverse the vertex order and use sign-bit
				 * to create the mask with _mm_srai_epi32.
				 * but it's slower! (at least on Broadwell)
				 *
				 * auto area = rmlg::Area(dc2, dc1, dc0);
				 * auto front = rmlv::sar<31>(float2bits(area));
				 */

				auto ix0 = ftoi(dc0.x), iy0 = ftoi(dc0.y);
				auto ix1 = ftoi(dc1.x), iy1 = ftoi(dc1.y);
				auto ix2 = ftoi(dc2.x), iy2 = ftoi(dc2.y);

				auto vminx = vmax(vmin(ix0, vmin(ix1, ix2)), tlx);
				auto vminy = vmax(vmin(iy0, vmin(iy1, iy2)), tly);
				auto vmaxx = vmin(vmax(ix0, vmax(ix1, ix2))+1, brx);
				auto vmaxy = vmin(vmax(iy0, vmax(iy1, iy2))+1, bry);

				auto notCulled = andnot(front, keepBacks) | (front&keepFronts);
				notCulled = andnot(bits2float(pointsOutside), notCulled);
				auto nonemptyX = bits2float(cmpgt(vmaxx, vminx));
				auto nonemptyY = bits2float(cmpgt(vmaxy, vminy));
				auto accept = notCulled & (nonemptyX & nonemptyY);

				// correct order of back-facing tris
				auto ni0 = SelectBits(ii2, ii0, front);
				auto ni1 = ii1;
				auto ni2 = SelectBits(ii0, ii2, front);

				uint32_t good = _mm_movemask_ps(accept.v) & laneMask;
				while (good) {
					int li = FindAndClearLSB(good);

					auto topLeft = ivec2{ vminx.si[li], vminy.si[li] } / tileDimensionsInPixels_;
					auto bottomRight = ivec2{ vmaxx.si[li], vmaxy.si[li] } / tileDimensionsInPixels_;
					int i0 = ni0.si[li];
					int i1 = ni1.si[li];
					int i2 = ni2.si[li];
					if (!front.lane[li]) {
						i0 |= 0x8000; }

					int stride = bufferDimensionsInTiles_.x;
					int tidRow = topLeft.y * stride;
					for (int ty = topLeft.y; ty <= bottomRight.y; ++ty, tidRow+=stride) {
						for (int tx = topLeft.x; tx <= bottomRight.x; ++tx) {
							auto th = &tilesHead_[tidRow + tx];
							if (INSTANCED) {
								assert(0 <= iid && iid < 65536);
								AppendUShort(th, static_cast<uint16_t>(iid));}
							AppendUShort(th, static_cast<uint16_t>(i0));
							AppendUShort(th, static_cast<uint16_t>(i1));
							AppendUShort(th, static_cast<uint16_t>(i2)); }}}}

			}  // instance loop

		for (auto& h : tilesHead_) {
			if (Touched(&h)) {
				AppendUShort(&h, 0xffff); }
			else {
				// remove the command from any tiles
				// that weren't covered by this draw
				Unappend(&h, 1); }}

		stats0_.totalPrimitivesClipped = static_cast<int>(clipQueue_.size());
		if (!clipQueue_.empty()) {
			ClipTriangles<INSTANCED>(state); }}

	/**
	 * transform and bin triangles, 1-phase impl
	 *
	 * preferred when
	 *  - a subset of a vertex-buffer is used, or
	 *  - vertices are not shared by multiple primitives
	 */
	template <bool INSTANCED, typename INDEX_SOURCE>
	void BinTriangles1P(const int count, INDEX_SOURCE& indexSource, const int instanceCnt) {
		using std::min, std::max, std::swap;
		using rmlv::ivec2, rmlv::qfloat, rmlv::qfloat2, rmlv::qfloat3, rmlv::qfloat4, rmlm::qmat4;
		const auto& state = *binState;

		clipQueue_.clear();

		const auto matrices = MakeMatrices(state);
		const typename SHADER::UniformsMD uniforms(*static_cast<const typename SHADER::UniformsSD*>(binUniforms));
		const auto cullingEnabled = state.cullingEnabled;
		const auto cullFace = state.cullFace;
		typename SHADER::Loader loader( state.buffers, state.bufferFormat );
		const auto frustum = ViewFrustum{ bufferDimensionsInPixels_ };

		const auto scissorRect = ScissorRect(state);

		const auto [DS, DO] = DSDO(state);

		const rmlv::mvec4i tlx{ scissorRect.top_left.x };
		const rmlv::mvec4i tly{ scissorRect.top_left.y };
		const rmlv::mvec4i brx{ scissorRect.bottom_right.x-1 };
		const rmlv::mvec4i bry{ scissorRect.bottom_right.y-1 };

		const auto keepBacks = bits2float(rmlv::mvec4i{ cullingEnabled && (cullFace == GL_BACK) ? 0 : -1 });
		const auto keepFronts = bits2float(rmlv::mvec4i{ cullingEnabled && (cullFace == GL_FRONT) ? 0 : -1 });

		if (!INSTANCED) {
			// coding error if this is not true
			// XXX could use constexpr instead of template param?
			assert(instanceCnt == 1); }

		const auto cmd = INSTANCED ? CMD_DRAW_INLINE_INSTANCED : CMD_DRAW_INLINE;
		for (auto& h : tilesHead_) {
			AppendByte(&h, cmd);
			Mark(&h); }

		for (int iid=0; iid<instanceCnt; ++iid) {
			typename SHADER::VertexInput vi0, vi1, vi2;
			typename SHADER::VertexOutputMD vo0, vo1, vo2;
			rmlv::qfloat2 dc0, dc1, dc2;
			rmlv::mvec4i cf0, cf1, cf2;
			rmlv::mvec4i ii0, ii1, ii2;

			if (INSTANCED) {
				loader.LoadInstance(iid, vi0);
				loader.LoadInstance(iid, vi1);
				loader.LoadInstance(iid, vi2); }

			int numPrims{count / 3};
			int numLanes{4};
			int laneMask{(1<<4)-1};
			for (int pi=0; pi<numPrims; pi+=4) {
				if (pi + 4 > numPrims) {
					numLanes = numPrims - pi;
					laneMask = (1<<numLanes)-1; }

				stats0_.totalPrimitivesSubmitted++;

				// gather
				for (int li=0; li<numLanes; ++li) {
					int ti = (pi+li)*3;
					auto i0 = static_cast<uint16_t>(indexSource(ti));
					auto i1 = static_cast<uint16_t>(indexSource(ti+1));
					auto i2 = static_cast<uint16_t>(indexSource(ti+2));
					ii0.si[li] = i0;
					ii1.si[li] = i1;
					ii2.si[li] = i2;
					loader.LoadLane(i0, li, vi0);
					loader.LoadLane(i1, li, vi1);
					loader.LoadLane(i2, li, vi2); }

				{
					qfloat4 gl_Position;
					SHADER::ShadeVertex(matrices, uniforms, vi0, gl_Position, vo0);
					cf0 = frustum.Test(gl_Position);
					dc0 = pdiv(gl_Position).xy() * DS + DO; }
				{
					qfloat4 gl_Position;
					SHADER::ShadeVertex(matrices, uniforms, vi1, gl_Position, vo1);
					cf1 = frustum.Test(gl_Position);
					dc1 = pdiv(gl_Position).xy() * DS + DO; }
				{
					qfloat4 gl_Position;
					SHADER::ShadeVertex(matrices, uniforms, vi2, gl_Position, vo2);
					cf2 = frustum.Test(gl_Position);
					dc2 = pdiv(gl_Position).xy() * DS + DO; }

				auto pointsOutside = cmpgt(cf0|cf1|cf2, rmlv::mvec4i::zero());
				auto primsOutside  = cmpgt(cf0&cf1&cf2, rmlv::mvec4i::zero());
				auto intersectsPlanes = andnot(primsOutside, pointsOutside);

				if ((_mm_movemask_ps(bits2float(primsOutside).v) & laneMask) == laneMask) {
					continue; }
				uint32_t needClip = _mm_movemask_ps(bits2float(intersectsPlanes).v) & laneMask;
				while (needClip) {
					int li = FindAndClearLSB(needClip);
					clipQueue_.push_back({ iid, ii0.si[li], ii1.si[li], ii2.si[li] }); }

				auto area = rmlg::Area(dc0, dc1, dc2);
				auto front = cmpgt(area, rmlv::mvec4f::zero());

				auto ix0 = ftoi(dc0.x), iy0 = ftoi(dc0.y);
				auto ix1 = ftoi(dc1.x), iy1 = ftoi(dc1.y);
				auto ix2 = ftoi(dc2.x), iy2 = ftoi(dc2.y);

				auto vminx = vmax(vmin(ix0, vmin(ix1, ix2)), tlx);
				auto vminy = vmax(vmin(iy0, vmin(iy1, iy2)), tly);
				auto vmaxx = vmin(vmax(ix0, vmax(ix1, ix2))+1, brx);
				auto vmaxy = vmin(vmax(iy0, vmax(iy1, iy2))+1, bry);

				auto notCulled = andnot(front, keepBacks) | (front&keepFronts);
				notCulled = andnot(bits2float(pointsOutside), notCulled);
				auto nonemptyX = bits2float(cmpgt(vmaxx, vminx));
				auto nonemptyY = bits2float(cmpgt(vmaxy, vminy));
				auto accept = notCulled & (nonemptyX & nonemptyY);

				// correct order of back-facing tris
				auto ni0 = SelectBits(ii2, ii0, front);
				auto ni1 = ii1;
				auto ni2 = SelectBits(ii0, ii2, front);

				uint32_t good = _mm_movemask_ps(accept.v) & laneMask;
				while (good) {
					int li = FindAndClearLSB(good);

					auto topLeft = ivec2{ vminx.si[li], vminy.si[li] } / tileDimensionsInPixels_;
					auto bottomRight = ivec2{ vmaxx.si[li], vmaxy.si[li] } / tileDimensionsInPixels_;
					int i0 = ni0.si[li];
					int i1 = ni1.si[li];
					int i2 = ni2.si[li];
					if (!front.lane[li]) {
						i0 |= 0x8000; }

					int stride = bufferDimensionsInTiles_.x;
					int tidRow = topLeft.y * stride;
					for (int ty = topLeft.y; ty <= bottomRight.y; ++ty, tidRow+=stride) {
						for (int tx = topLeft.x; tx <= bottomRight.x; ++tx) {
							auto th = &tilesHead_[tidRow + tx];
							if (INSTANCED) {
								assert(0 <= iid && iid < 65536);
								AppendUShort(th, static_cast<uint16_t>(iid));}
							AppendUShort(th, static_cast<uint16_t>(i0));
							AppendUShort(th, static_cast<uint16_t>(i1));
							AppendUShort(th, static_cast<uint16_t>(i2)); }}}}

			}  // instance loop

		for (auto& h : tilesHead_) {
			if (Touched(&h)) {
				AppendUShort(&h, 0xffff); }
			else {
				// remove the command from any tiles
				// that weren't covered by this draw
				Unappend(&h, 1); }}

		stats0_.totalPrimitivesClipped = static_cast<int>(clipQueue_.size());
		if (!clipQueue_.empty()) {
			ClipTriangles<INSTANCED>(state); }}

	template <bool INSTANCED>
	void ClipTriangles(const GLState& state) {
		using rmlm::mat4;
		using rmlv::ivec2, rmlv::vec2, rmlv::vec3, rmlv::vec4, rmlv::qfloat4;
		using std::array, std::swap, std::min, std::max;

		typename SHADER::Loader loader( state.buffers, state.bufferFormat );
		const auto frustum = ViewFrustum{ bufferDimensionsInPixels_.x };

		const auto [DS, DO] = DSDO(state);

		const auto scissorRect = ScissorRect(state);

		const auto matrices = MakeMatrices(state);
		const typename SHADER::UniformsMD uniforms(*static_cast<const typename SHADER::UniformsSD*>(binUniforms));

		auto CVMix = [&](const ClippedVertex& a, const ClippedVertex& b, const float d) {
			static_assert(sizeof(typename SHADER::VertexOutputSD) <= sizeof(ClippedVertex::data), "shader output too large for clipped vertex buffer");
			ClippedVertex out;
			out.coord = mix(a.coord, b.coord, d);
			auto& ad = *reinterpret_cast<const typename SHADER::VertexOutputSD*>(&a.data);
			auto& bd = *reinterpret_cast<const typename SHADER::VertexOutputSD*>(&b.data);
			auto& od = *reinterpret_cast<      typename SHADER::VertexOutputSD*>(&out.data);
			od = SHADER::VertexOutputSD::Mix(ad, bd, d);
			return out; };

		for (const auto& faceIndices : clipQueue_) {
			// phase 1: load _poly_ with the shaded vertex data
			clipA_.clear();
			for (int i=0; i<3; ++i) {
				const int iid = faceIndices[0];
				const auto idx = faceIndices[i+1];

				typename SHADER::VertexInput vertex;
				loader.LoadLane(idx, 0, vertex);
				if (INSTANCED) {
					loader.LoadInstance(iid, vertex); }

				qfloat4 coord;
				typename SHADER::VertexOutputMD computed;
				SHADER::ShadeVertex(matrices, uniforms, vertex, coord, computed);

				ClippedVertex cv;
				cv.coord = coord.lane(0);
				auto* tmp = reinterpret_cast<typename SHADER::VertexOutputSD*>(&cv.data);
				*tmp = computed.Lane(0);
				clipA_.emplace_back(cv); }

			// phase 2: sutherland-hodgman clipping
			for (const auto plane : rglv::clipping_panes) {

				bool hereIsInside = frustum.IsInside(plane, clipA_[0].coord);

				clipB_.clear();
				for (int here_vi = 0; here_vi < int(clipA_.size()); ++here_vi) {
					const auto next_vi = (here_vi + 1) % clipA_.size(); // wrap

					const auto& here = clipA_[here_vi];
					const auto& next = clipA_[next_vi];

					const bool nextIsInside = frustum.IsInside(plane, next.coord);

					if (hereIsInside) {
						clipB_.push_back(here); }

					if (hereIsInside != nextIsInside) {
                        // making transition across plane
                        // to avoid cracks between adjacent faces, the new
                        // vertex must be computed using the same direction
                        // across the plane
                        auto& fromVertex = hereIsInside ? here : next;
                        auto& toVertex = hereIsInside ? next : here;
                        float d = frustum.Distance(plane, fromVertex.coord, toVertex.coord);
                        auto newVertex = CVMix(fromVertex, toVertex, d);
                        clipB_.emplace_back(newVertex);

						hereIsInside = !hereIsInside; }}

				swap(clipA_, clipB_);
				if (clipA_.size() == 0) {
					break; }
				assert(clipA_.size() >= 3); }

			if (clipA_.size() == 0) {
				continue; }

			for (auto& vertex : clipA_) {
				// convert clip-coord to device-coord
				vertex.coord = pdiv(vertex.coord);
				vertex.coord.x = (vertex.coord.x * DS.x + DO.x).get_x();
				vertex.coord.y = (vertex.coord.y * DS.y + DO.y).get_x(); }
			// end of phase 2: poly contains a clipped N-gon

			// check direction, maybe cull, maybe reorder
			const bool backfacing = rmlg::Area(clipA_[0].coord.xy(), clipA_[1].coord.xy(), clipA_[2].coord.xy()) < 0;
			bool willCull = true;
			if (backfacing) {
				if (state.cullingEnabled == false || ((state.cullFace & GL_BACK) == 0)) {
					willCull = false; }}
			else {
				if (state.cullingEnabled == false || ((state.cullFace & GL_FRONT) == 0)) {
					willCull = false; }}
			if (willCull) {
				continue; }
			if (backfacing) {
				reverse(begin(clipA_), end(clipA_)); }

			// phase 3: project, convert to triangles and add to bins
			const auto bi = static_cast<int>(clippedVertexBuffer0_.size());
			clippedVertexBuffer0_.insert(end(clippedVertexBuffer0_), begin(clipA_), end(clipA_));
			for (int vi=1; vi<int(clipA_.size()) - 1; ++vi) {
				int i0{0}, i1{vi}, i2{vi+1};
				ForEachCoveredTile(scissorRect, clipA_[i0].coord.xy(), clipA_[i1].coord.xy(), clipA_[i2].coord.xy(), [&](uint8_t** th) {
					AppendByte(th, CMD_CLIPPED_TRI);
					AppendUShort(th, static_cast<uint16_t>((bi+i0) | (backfacing ? 0x8000 : 0)));
					AppendUShort(th, static_cast<uint16_t>(bi+i1));
					AppendUShort(th, static_cast<uint16_t>(bi+i2)); }); }}}

	template <typename FUNC>
	auto ForEachCoveredTile(const rmlg::irect rect, const rmlv::vec2 dc0, const rmlv::vec2 dc1, const rmlv::vec2 dc2, FUNC func) -> int {
		using std::min, std::max, rmlv::ivec2;
		const ivec2 idev0{ dc0 };
		const ivec2 idev1{ dc1 };
		const ivec2 idev2{ dc2 };

		const int vminx = max(rmlv::Min(idev0.x, idev1.x, idev2.x),   rect.top_left.x);
		const int vminy = max(rmlv::Min(idev0.y, idev1.y, idev2.y),   rect.top_left.y);
		const int vmaxx = min(rmlv::Max(idev0.x, idev1.x, idev2.x)+1, rect.bottom_right.x-1);
		const int vmaxy = min(rmlv::Max(idev0.y, idev1.y, idev2.y)+1, rect.bottom_right.y-1);

		auto topLeft = ivec2{ vminx, vminy } / tileDimensionsInPixels_;
		auto bottomRight = ivec2{ vmaxx, vmaxy } / tileDimensionsInPixels_;

		int stride = bufferDimensionsInTiles_.x;
		int tidRow = topLeft.y * stride;
		int hits{0};
		for (int ty = topLeft.y; ty <= bottomRight.y; ++ty, tidRow+=stride) {
			for (int tx = topLeft.x; tx <= bottomRight.x; ++tx) {
				++hits;
				func(&tilesHead_[tidRow + tx]); }}
		return hits; }

public:
	static
	auto MakeBinProgramPtrs() -> rglv::BinProgramPtrs {
		rglv::BinProgramPtrs out;
		out.DrawArrays1   = static_cast<decltype(out.DrawArrays1)  >(&GPUBinImpl::DrawArrays1);
		out.DrawArraysN   = static_cast<decltype(out.DrawArraysN)  >(&GPUBinImpl::DrawArraysN);
		out.DrawElements1 = static_cast<decltype(out.DrawElements1)>(&GPUBinImpl::DrawElements1);
		out.DrawElementsN = static_cast<decltype(out.DrawElementsN)>(&GPUBinImpl::DrawElementsN);
		return out; }};

			
template <typename COLOR_IO, typename DEPTH_IO, typename SHADER, bool SCISSOR_ENABLED, bool DEPTH_TEST, typename DEPTH_FUNC, bool DEPTH_WRITEMASK, bool COLOR_WRITEMASK, typename BLEND_FUNC>
class GPUTileImpl : GPU {

	void DrawElements1(void* c0, void* d, const GLState& state, const void* uniformsPtr, rmlg::irect rect, rmlv::ivec2 tileOrigin, int tileIdx, FastPackedReader& cs) {
		DrawTriangles<false>(c0, d, state, uniformsPtr, rect, tileOrigin, tileIdx, cs); }

	void DrawElementsN(void* c0, void* d, const GLState& state, const void* uniformsPtr, rmlg::irect rect, rmlv::ivec2 tileOrigin, int tileIdx, FastPackedReader& cs) {
		DrawTriangles<true>(c0, d, state, uniformsPtr, rect, tileOrigin, tileIdx, cs); }

	template<bool INSTANCED>
	void DrawTriangles(void* color0Buf, void* depthBuf, const GLState& state, const void* uniformsPtr, rmlg::irect rect, rmlv::ivec2 tileOrigin, int tileIdx, FastPackedReader& cs) {
		using rmlm::mat4;
		using rmlv::vec2, rmlv::vec3, rmlv::vec4;
		using rmlv::mvec4i;
		using rmlv::qfloat, rmlv::qfloat2, rmlv::qfloat4;
		using std::array, std::swap;

		if (SCISSOR_ENABLED) {
			rect = Intersect(rect, gl_offset_and_size_to_irect(state.scissorOrigin, state.scissorSize)); }

		COLOR_IO colorCursor( color0Buf, tileOrigin );
		DEPTH_IO depthCursor( depthBuf, tileOrigin );

		auto loader = typename SHADER::Loader{ state.buffers, state.bufferFormat };

		const int targetHeightInPixels_ = bufferDimensionsInPixels_.y;

		// XXX workaround for lambda-capture of vars from structured binding
		rmlv::qfloat2 DS, DO; std::tie(DS, DO) = DSDO(state);
		// const auto [DS, DO] = DSDO(state);

		const auto matrices = MakeMatrices(state);
		const typename SHADER::UniformsMD uniforms(*static_cast<const typename SHADER::UniformsSD*>(uniformsPtr));

		using sampler = const rglr::TextureUnit*;

		alignas(64) std::byte tu0mem[128];
		alignas(64) std::byte tu1mem[128];
		const auto tu0 = rglr::MakeTextureUnit(reinterpret_cast<const float*>(state.tus[0].ptr), state.tus[0].width, state.tus[0].height, state.tus[0].stride, state.tus[0].filter, &tu0mem);
		const auto tu1 = rglr::MakeTextureUnit(reinterpret_cast<const float*>(state.tus[1].ptr), state.tus[1].width, state.tus[1].height, state.tus[1].stride, state.tus[1].filter, &tu1mem);
		const auto tu3 = rglr::DepthTextureUnit(state.tu3ptr, state.tu3dim);

		array<typename SHADER::VertexInput, 3> vertex;
		array<typename SHADER::VertexOutputMD, 3> computed;
		array<qfloat4, 3> devCoord;
		array<mvec4i, 3> fx, fy;
		bool backfacing[4];
		int li{0};  // sse lane being loaded

		auto flush = [&]() {
			for (int i=0; i<3; ++i) {
				qfloat4 gl_Position;
				SHADER::ShadeVertex(matrices, uniforms, vertex[i], gl_Position, computed[i]);
				devCoord[i] = pdiv(gl_Position);
				fx[i] = ftoi(rglv::FP_MUL * (devCoord[i].x * DS.x + DO.x));
				fy[i] = ftoi(rglv::FP_MUL * (devCoord[i].y * DS.y + DO.y)); }

			auto area = rmlg::Area(devCoord[0].xy(), devCoord[1].xy(), devCoord[2].xy());
			auto backfacing = rmlv::sar<31>(float2bits(area));

			// draw up to 4 triangles
			auto triPgm = TriangleProgram<SHADER,
			                              COLOR_IO, DEPTH_IO,
			                              sampler, sampler, rglr::DepthTextureUnit,
			                              DEPTH_TEST, DEPTH_FUNC, DEPTH_WRITEMASK, COLOR_WRITEMASK, BLEND_FUNC>{
				colorCursor, depthCursor,
				tu0, tu1, tu3,
				matrices, uniforms,
				// XXX backfacing,
				devCoord[0].w, devCoord[1].w, devCoord[2].w,
				devCoord[0].z, devCoord[1].z, devCoord[2].z,
				computed[0], computed[1], computed[2] };
			auto rasterizer = VTriangleRasterizer<SCISSOR_ENABLED, decltype(triPgm)>{triPgm, rect, targetHeightInPixels_};
			rasterizer.Draw(fx[0], fx[1], fx[2],
							fy[0], fy[1], fy[2], li);

			// reset the SIMD lane counter
			li = 0; };

		int loadedInstanceId;
		if (INSTANCED) {
			loadedInstanceId = cs.PeekUShort();
			loader.LoadInstance(loadedInstanceId, vertex[0]);
			loader.LoadInstance(loadedInstanceId, vertex[1]);
			loader.LoadInstance(loadedInstanceId, vertex[2]); }

		while (1) {
			uint16_t firstWord = cs.ConsumeUShort();
			if (firstWord == 0xffff) {
				break; }

			if (INSTANCED) {
				const int iid = firstWord;
				if (iid != loadedInstanceId) {
					flush();
					loadedInstanceId = iid;
					loader.LoadInstance(iid, vertex[0]);
					loader.LoadInstance(iid, vertex[1]);
					loader.LoadInstance(iid, vertex[2]); }}

			uint16_t tmp;
			if (INSTANCED) {
				tmp = cs.ConsumeUShort(); }
			else {
				tmp = firstWord; }
			const uint16_t i1 = cs.ConsumeUShort();
			const uint16_t i2 = cs.ConsumeUShort();
			backfacing[li] = tmp & 0x8000;
			const uint16_t i0 = tmp & 0x7fff;
			loader.LoadLane(i0, li, vertex[0]);
			loader.LoadLane(i1, li, vertex[1]);
			loader.LoadLane(i2, li, vertex[2]);
			++li;

			if (li == 4) {
				flush(); }}
		flush(); }

	void DrawClipped(void* color0Buf, void* depthBuf, const GLState& state, const void* uniformsPtr, rmlg::irect rect, rmlv::ivec2 tileOrigin, int tileIdx, FastPackedReader& cs) {
		using rmlm::mat4;
		using rmlv::vec2, rmlv::vec3, rmlv::vec4;

		if (SCISSOR_ENABLED) {
			rect = Intersect(rect, gl_offset_and_size_to_irect(state.scissorOrigin, state.scissorSize)); }

		COLOR_IO colorCursor( color0Buf, tileOrigin );
		DEPTH_IO depthCursor( depthBuf, tileOrigin );

		const int targetHeightInPixels_ = bufferDimensionsInPixels_.y;

		const auto matrices = MakeMatrices(state);
		const typename SHADER::UniformsMD uniforms(*static_cast<const typename SHADER::UniformsSD*>(uniformsPtr));

		const auto tmp = cs.ConsumeUShort();
		const bool backfacing = (tmp & 0x8000) != 0;
		const auto i0 = tmp & 0x7fff;
		const auto i1 = cs.ConsumeUShort();
		const auto i2 = cs.ConsumeUShort();

		vec4 dev0 = clippedVertexBuffer1_[i0].coord;
		vec4 dev1 = clippedVertexBuffer1_[i1].coord;
		vec4 dev2 = clippedVertexBuffer1_[i2].coord;
		auto& data0 = *reinterpret_cast<typename SHADER::VertexOutputSD*>(&(clippedVertexBuffer1_[i0].data));
		auto& data1 = *reinterpret_cast<typename SHADER::VertexOutputSD*>(&(clippedVertexBuffer1_[i1].data));
		auto& data2 = *reinterpret_cast<typename SHADER::VertexOutputSD*>(&(clippedVertexBuffer1_[i2].data));

		using sampler = const rglr::TextureUnit*;
		alignas(64) std::byte tu0mem[128];
		alignas(64) std::byte tu1mem[128];
		const auto tu0 = rglr::MakeTextureUnit(reinterpret_cast<const float*>(state.tus[0].ptr), state.tus[0].width, state.tus[0].height, state.tus[0].stride, state.tus[0].filter, &tu0mem);
		const auto tu1 = rglr::MakeTextureUnit(reinterpret_cast<const float*>(state.tus[1].ptr), state.tus[1].width, state.tus[1].height, state.tus[1].stride, state.tus[1].filter, &tu1mem);
		const auto tu3 = rglr::DepthTextureUnit(state.tu3ptr, state.tu3dim);

		auto triPgm = TriangleProgram<SHADER,
		                              COLOR_IO, DEPTH_IO,
		                              sampler, sampler, rglr::DepthTextureUnit,
		                              DEPTH_TEST, DEPTH_FUNC, DEPTH_WRITEMASK, COLOR_WRITEMASK, BLEND_FUNC>{
			colorCursor, depthCursor,
			tu0, tu1, tu3,
			matrices,
			uniforms,
			// XXX backfacing,
			VertexFloat1{ dev0.w, dev1.w, dev2.w },
			VertexFloat1{ dev0.z, dev1.z, dev2.z },
			data0, data1, data2 };
		auto rasterizer = TriangleRasterizer<SCISSOR_ENABLED, decltype(triPgm)>{triPgm, rect, targetHeightInPixels_};
		rasterizer.Draw(int(dev0.x*rglv::FP_MUL), int(dev1.x*rglv::FP_MUL), int(dev2.x*rglv::FP_MUL),
		                int(dev0.y*rglv::FP_MUL), int(dev1.y*rglv::FP_MUL), int(dev2.y*rglv::FP_MUL)); }

public:
	static
	auto MakeDrawProgramPtrs() -> rglv::DrawProgramPtrs {
		rglv::DrawProgramPtrs out;
		out.DrawElements1 = static_cast<decltype(out.DrawElements1)>(&GPUTileImpl::DrawElements1);
		out.DrawElementsN = static_cast<decltype(out.DrawElementsN)>(&GPUTileImpl::DrawElementsN);
		out.DrawClipped   = static_cast<decltype(out.DrawClipped)  >(&GPUTileImpl::DrawClipped);
		return out; }};


}  // close package namespace
}  // close enterprise namespace

#ifdef _MSC_VER
#pragma warning(pop)
#endif
