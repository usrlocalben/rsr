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

template <typename SHADER, typename COLOR_IO, typename DEPTH_IO, typename TU0, typename TU1, typename TU3, bool DEPTH_TEST, typename DEPTH_FUNC, bool DEPTH_WRITEMASK, bool COLOR_WRITEMASK, typename BLEND_FUNC>
struct TriangleProgram {
	COLOR_IO cc_;
	DEPTH_IO dc_;
	const Matrices matrices_;
	const rglv::VertexFloat1 invW_;
	const rglv::VertexFloat1 ndcZ_;
	const typename SHADER::UniformsMD uniforms_;
	const typename SHADER::Interpolants vo_;
	const TU0 tu0_;
	const TU1 tu1_;
	const TU3& tu3_;

	TriangleProgram(
		COLOR_IO cc,
		DEPTH_IO dc,
		Matrices matrices,
		VertexFloat1 invW_,
		VertexFloat1 ndcZ_,
		typename SHADER::UniformsMD uniforms,
		typename SHADER::VertexOutputSD computed0,
		typename SHADER::VertexOutputSD computed1,
		typename SHADER::VertexOutputSD computed2,
		const TU0 tu0,
		const TU1 tu1,
		const TU3& tu3) :
		cc_(cc),
		dc_(dc),
		matrices_(matrices),
		invW_(invW_),
		ndcZ_(ndcZ_),
		uniforms_(uniforms),
		vo_(computed0, computed1, computed2),
		tu0_(tu0),
		tu1_(tu1),
		tu3_(tu3)
		{}

	void Begin(int x, int y) {
		cc_.Begin(x, y);
		dc_.Begin(x, y); }

	void CR() {
		cc_.CR();
		dc_.CR(); }

	void Right2() {
		cc_.Right2();
		dc_.Right2(); }

	void Render(const rmlv::qfloat2 fragCoord, const rmlv::mvec4i triMask, rglv::BaryCoord BS, const bool frontfacing) {
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

	void tile_StoreTrueColor(const GLState& state, const void* uniformsPtr, const rmlg::irect rect, const bool enableGamma, rglr::TrueColorCanvas& outcanvas) {
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
		out.tile_StoreTrueColor = static_cast<decltype(out.tile_StoreTrueColor)>(&GPUBltImpl::tile_StoreTrueColor);
		return out; } };


template <typename SHADER>
class GPUBinImpl : GPU {

	void bin_DrawArraysInstanced(int count, int instanceCnt) {
		struct SequenceSource {
			int operator()(int ti) { return ti; }};
		SequenceSource indexSource{};
		bin_DrawArrays<true>(count, indexSource, instanceCnt); }

	void bin_DrawArraysSingle(int count) {
		struct SequenceSource {
			int operator()(int ti) { return ti; }};
		SequenceSource indexSource{};
		bin_DrawArrays<false>(count, indexSource, 1); }

	void bin_DrawElementsSingle(int count, uint16_t* indices, uint8_t hint) {
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
			bin_DrawArrays<false>(count, indexSource, 1); }
		else {
			struct ArraySource {
				ArraySource(const uint16_t* data) : data_(data) {}
				int operator()(int ti) { return data_[ti]; }
				const uint16_t* const data_; };
			ArraySource indexSource{indices};
			bin_DrawElements<false>(count, indexSource, 1); }}

	void bin_DrawElementsInstanced(int count, uint16_t* indices, int instanceCnt) {
			struct ArraySource {
				ArraySource(const uint16_t* data) : data_(data) {}
				int operator()(int ti) { return data_[ti]; }
				const uint16_t* const data_; };
			ArraySource indexSource{indices};
			bin_DrawElements<true>(count, indexSource, instanceCnt); }

	template <bool INSTANCED, typename INDEX_SOURCE>
	void bin_DrawArrays(const int count, INDEX_SOURCE& indexSource, const int instanceCnt) {
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

		if (!INSTANCED) {
			// coding error if this is not true
			// XXX could use constexpr instead of template param?
			assert(instanceCnt == 1); }

		inMem0_.reserve(count/3+4);
		inMem1_.reserve(count/3+4);
		inMem2_.reserve(count/3+4);

		const auto cmd = INSTANCED ? CMD_DRAW_INLINE_INSTANCED : CMD_DRAW_INLINE;
		for (auto& h : tilesHead_) {
			AppendByte(&h, cmd);
			Mark(&h); }

		for (int iid=0; iid<instanceCnt; ++iid) {
			if (INSTANCED) {
				loader.LoadInstance(iid, vertex); }

			const auto siz = loader.Size();
			// xxx const int rag = siz % 4;  assume vaos are always padded to size()%4=0

			for (int vi=0; vi<siz; vi+=4) {
				loader.LoadMD(vi, vertex);

				qfloat4 coord;
				typename SHADER::VertexOutputMD unused;
				SHADER::ShadeVertex(matrices, uniforms, vertex, coord, unused);

				auto flags = frustum.Test(coord);
				store_bytes(clipFlagBuffer_.data() + vi, flags);

				auto devCoord = pdiv(coord).xy() * DS + DO;
				devCoord.template store<false>(devCoordXBuffer_.data() + vi,
				                               devCoordYBuffer_.data() + vi); }

			int numPrims{0};
			for (int ti=0; ti<count; ti+=3) {
				stats0_.totalPrimitivesSubmitted++;

				auto i0 = static_cast<uint16_t>(indexSource(ti));
				auto i1 = static_cast<uint16_t>(indexSource(ti+1));
				auto i2 = static_cast<uint16_t>(indexSource(ti+2));

				// check for triangles that need clipping
				const auto cf0 = clipFlagBuffer_.data()[i0];
				const auto cf1 = clipFlagBuffer_.data()[i1];
				const auto cf2 = clipFlagBuffer_.data()[i2];
				if (cf0 | cf1 | cf2) {
					if (cf0 & cf1 & cf2) {
						// all points outside of at least one plane
						stats0_.totalPrimitivesCulled++;
						continue; }
					// queue for clipping
					clipQueue_.push_back({ iid, i0, i1, i2 });
					continue; }

				inMem0_[numPrims] = i0;
				inMem1_[numPrims] = i1;
				inMem2_[numPrims] = i2;
				++numPrims; }

			for (int i=0; i<4; ++i) {
				inMem0_[numPrims+i] = 0;
				inMem1_[numPrims+i] = 0;
				inMem2_[numPrims+i] = 0; }

			const auto keepBacks = bits2float(rmlv::mvec4i{ cullingEnabled && (cullFace == GL_BACK) ? 0 : -1 });
			const auto keepFronts = bits2float(rmlv::mvec4i{ cullingEnabled && (cullFace == GL_FRONT) ? 0 : -1 });

			int numLanes{4};
			int laneMask{ (1<<4)-1 };
			for (int pi=0; pi<numPrims; pi+=4) {
				if (pi + 4 > numPrims) {
					numLanes = numPrims - pi;
					laneMask = (1<<numLanes)-1; }

				rmlv::qfloat2 dc0, dc1, dc2;
				int ii0[4], ii1[4], ii2[4];

				// gather
				for (int li=0; li<4; ++li) {
					ii0[li] = inMem0_[pi+li];
					ii1[li] = inMem1_[pi+li];
					ii2[li] = inMem2_[pi+li];

					dc0.setLane(li, { devCoordXBuffer_.data()[ii0[li]], devCoordYBuffer_.data()[ii0[li]] });
					dc1.setLane(li, { devCoordXBuffer_.data()[ii1[li]], devCoordYBuffer_.data()[ii1[li]] });
					dc2.setLane(li, { devCoordXBuffer_.data()[ii2[li]], devCoordYBuffer_.data()[ii2[li]] }); }

				// handle backfacing tris and culling
				auto area = rmlg::Area(dc0, dc1, dc2);
				auto front = cmpgt(area, rmlv::mvec4f::zero());

				auto ix0 = ftoi(dc0.x), iy0 = ftoi(dc0.y);
				auto ix1 = ftoi(dc1.x), iy1 = ftoi(dc1.y);
				auto ix2 = ftoi(dc2.x), iy2 = ftoi(dc2.y);

				//XXX auto& rect = scissorRect;
				rmlv::mvec4i tlx{ scissorRect.top_left.x };
				rmlv::mvec4i tly{ scissorRect.top_left.y };
				rmlv::mvec4i brx{ scissorRect.bottom_right.x-1 };
				rmlv::mvec4i bry{ scissorRect.bottom_right.y-1 };

				auto vminx = vmax(vmin(ix0, vmin(ix1, ix2)), tlx);
				auto vminy = vmax(vmin(iy0, vmin(iy1, iy2)), tly);
				auto vmaxx = vmin(vmax(ix0, vmax(ix1, ix2)), brx);
				auto vmaxy = vmin(vmax(iy0, vmax(iy1, iy2)), bry);

				auto notCulled = andnot(front, keepBacks) | (front&keepFronts);
				auto nonemptyX = bits2float(cmpgt(vmaxx, vminx));
				auto nonemptyY = bits2float(cmpgt(vmaxy, vminy));
				auto accept = notCulled & (nonemptyX & nonemptyY);

				uint32_t good = _mm_movemask_ps(accept.v) & laneMask;

				while (good) {
					int li = FindAndClearLSB(good);

					auto topLeft = ivec2{ vminx.si[li], vminy.si[li] } / tileDimensionsInPixels_;
					auto bottomRight = ivec2{ vmaxx.si[li], vmaxy.si[li] } / tileDimensionsInPixels_;
					int i0 = ii0[li];
					int i1 = ii1[li];
					int i2 = ii2[li];

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
			bin_DrawElementsClipped<INSTANCED>(state); }}

	template <bool INSTANCED, typename INDEX_SOURCE>
	void bin_DrawElements(const int count, INDEX_SOURCE& indexSource, const int instanceCnt) {
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

		// XXX workaround for lambda-capture of vars from structured binding
		rmlv::qfloat2 DS, DO; std::tie(DS, DO) = DSDO(state);
		// const auto [DS, DO] = DSDO(state);

		const auto scissorRect = ScissorRect(state);

		if (!INSTANCED) {
			// coding error if this is not true
			// XXX could use constexpr instead of template param?
			assert(instanceCnt == 1); }

		const auto cmd = INSTANCED ? CMD_DRAW_INLINE_INSTANCED : CMD_DRAW_INLINE;
		for (auto& h : tilesHead_) {
			AppendByte(&h, cmd);
			Mark(&h); }

		for (int iid=0; iid<instanceCnt; ++iid) {
			std::array<typename SHADER::VertexInput, 3> vertex;
			std::array<typename SHADER::VertexOutputMD, 3> computed;
			std::array<rmlv::qfloat2, 3> devCoord;
			std::array<rmlv::mvec4i, 3> clipFlags;
			std::array<std::array<int, 4>, 3> srcIdx;
			int li{0};

			if (INSTANCED) {
				loader.LoadInstance(iid, vertex[0]);
				loader.LoadInstance(iid, vertex[1]);
				loader.LoadInstance(iid, vertex[2]); } 

			auto flush = [&]() {
				for (int i{0}; i<3; ++i) {
					qfloat4 gl_Position;
					SHADER::ShadeVertex(matrices, uniforms, vertex[i], gl_Position, computed[i]);
					clipFlags[i] = frustum.Test(gl_Position);
					devCoord[i] = pdiv(gl_Position).xy() * DS + DO; }

				auto area2 = rmlg::Area(devCoord[0], devCoord[1], devCoord[2]);
				auto backfacing = float2bits(cmplt(area2, rmlv::mvec4f::zero()));

				for (int ti{0}; ti<li; ++ti) {
					stats0_.totalPrimitivesSubmitted++;

					auto cf0 = clipFlags[0].si[ti];
					auto cf1 = clipFlags[1].si[ti];
					auto cf2 = clipFlags[2].si[ti];

					auto i0 = srcIdx[0][ti];
					auto i1 = srcIdx[1][ti];
					auto i2 = srcIdx[2][ti];

					auto dc0 = devCoord[0].lane(ti);
					auto dc1 = devCoord[1].lane(ti);
					auto dc2 = devCoord[2].lane(ti);

					if (cf0 | cf1 | cf2) {
						if (cf0 & cf1 & cf2) {
							// all points outside of at least one plane
							stats0_.totalPrimitivesCulled++;
							continue; }
						// queue for clipping
						clipQueue_.push_back({ iid, i0, i1, i2 });
						continue; }

					if (backfacing.ui[ti]) {
						if (cullingEnabled && cullFace == GL_BACK) {
							stats0_.totalPrimitivesCulled++;
							continue; }
						swap(i0, i2);
						i0 |= 0x8000; }  // add backfacing flag
					else {
						if (cullingEnabled && cullFace == GL_FRONT) {
							stats0_.totalPrimitivesCulled++;
							continue; }}

					ForEachCoveredTile(scissorRect, dc0, dc1, dc2, [&](uint8_t** th) {
						if (INSTANCED) {
							assert(0 <= iid && iid < 65536);
							AppendUShort(th, static_cast<uint16_t>(iid)); }
						AppendUShort(th, static_cast<uint16_t>(i0));  // also includes backfacing flag
						AppendUShort(th, static_cast<uint16_t>(i1));
						AppendUShort(th, static_cast<uint16_t>(i2)); }); }

				// reset the SIMD lane counter
				li = 0; };

			for (int ti=0; ti<count; ti+=3) {
				stats0_.totalPrimitivesSubmitted++;

				auto i0 = static_cast<uint16_t>(indexSource(ti));
				auto i1 = static_cast<uint16_t>(indexSource(ti+1));
				auto i2 = static_cast<uint16_t>(indexSource(ti+2));
				srcIdx[0][li] = i0;
				srcIdx[1][li] = i1;
				srcIdx[2][li] = i2;
				loader.LoadLane(i0, li, vertex[0]);
				loader.LoadLane(i1, li, vertex[1]);
				loader.LoadLane(i2, li, vertex[2]);
				if (++li == 4) {
					flush(); }}
			flush(); }  // end instance loop

		for (auto& h : tilesHead_) {
			if (Touched(&h)) {
				AppendUShort(&h, 0xffff); }
			else {
				// remove the command from any tiles
				// that weren't covered by this draw
				Unappend(&h, 1); }}

		stats0_.totalPrimitivesClipped = static_cast<int>(clipQueue_.size());
		if (!clipQueue_.empty()) {
			bin_DrawElementsClipped<INSTANCED>(state); }}

	template <bool INSTANCED>
	void bin_DrawElementsClipped(const GLState& state) {
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
	auto MakeVertexProgramPtrs() -> rglv::VertexProgramPtrs {
		rglv::VertexProgramPtrs out;
		out.bin_DrawArraysSingle = static_cast<decltype(out.bin_DrawArraysSingle)>(&GPUBinImpl::bin_DrawArraysSingle);
		out.bin_DrawArraysInstanced = static_cast<decltype(out.bin_DrawArraysInstanced)>(&GPUBinImpl::bin_DrawArraysInstanced);
		out.bin_DrawElementsSingle = static_cast<decltype(out.bin_DrawElementsSingle)>(&GPUBinImpl::bin_DrawElementsSingle);
		out.bin_DrawElementsInstanced = static_cast<decltype(out.bin_DrawElementsInstanced)>(&GPUBinImpl::bin_DrawElementsInstanced);
		return out; }};

			
template <typename COLOR_IO, typename DEPTH_IO, typename SHADER, bool SCISSOR_ENABLED, bool DEPTH_TEST, typename DEPTH_FUNC, bool DEPTH_WRITEMASK, bool COLOR_WRITEMASK, typename BLEND_FUNC>
class GPUTileImpl : GPU {

	void tile_DrawElementsSingle(void* c0, void* d, const GLState& state, const void* uniformsPtr, rmlg::irect rect, rmlv::ivec2 tileOrigin, int tileIdx, FastPackedReader& cs) {
		tile_DrawElements<false>(c0, d, state, uniformsPtr, rect, tileOrigin, tileIdx, cs); }

	void tile_DrawElementsInstanced(void* c0, void* d, const GLState& state, const void* uniformsPtr, rmlg::irect rect, rmlv::ivec2 tileOrigin, int tileIdx, FastPackedReader& cs) {
		tile_DrawElements<true>(c0, d, state, uniformsPtr, rect, tileOrigin, tileIdx, cs); }

	template<bool INSTANCED>
	void tile_DrawElements(void* color0Buf, void* depthBuf, const GLState& state, const void* uniformsPtr, rmlg::irect rect, rmlv::ivec2 tileOrigin, int tileIdx, FastPackedReader& cs) {
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

			// draw up to 4 triangles
			for (int ti=0; ti<li; ti++) {
				auto triPgm = TriangleProgram<SHADER, COLOR_IO, DEPTH_IO, sampler, sampler, rglr::DepthTextureUnit, DEPTH_TEST, DEPTH_FUNC, DEPTH_WRITEMASK, COLOR_WRITEMASK, BLEND_FUNC>{
					colorCursor,
					depthCursor,
					matrices,
					VertexFloat1{ devCoord[0].w.lane[ti], devCoord[1].w.lane[ti], devCoord[2].w.lane[ti] },
					VertexFloat1{ devCoord[0].z.lane[ti], devCoord[1].z.lane[ti], devCoord[2].z.lane[ti] },
					uniforms,
					computed[0].Lane(ti), computed[1].Lane(ti), computed[2].Lane(ti),
					tu0, tu1, tu3
					};
				auto rasterizer = TriangleRasterizer<SCISSOR_ENABLED, decltype(triPgm)>{triPgm, rect, targetHeightInPixels_};
				rasterizer.Draw(fx[0].si[ti], fx[1].si[ti], fx[2].si[ti],
				                fy[0].si[ti], fy[1].si[ti], fy[2].si[ti],
				                !backfacing[ti]); }

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

	void tile_DrawClipped(void* color0Buf, void* depthBuf, const GLState& state, const void* uniformsPtr, rmlg::irect rect, rmlv::ivec2 tileOrigin, int tileIdx, FastPackedReader& cs) {
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

		auto triPgm = TriangleProgram<SHADER, COLOR_IO, DEPTH_IO, sampler, sampler, rglr::DepthTextureUnit, DEPTH_TEST, DEPTH_FUNC, DEPTH_WRITEMASK, COLOR_WRITEMASK, BLEND_FUNC>{
			colorCursor,
			depthCursor,
			matrices,
			VertexFloat1{ dev0.w, dev1.w, dev2.w },
			VertexFloat1{ dev0.z, dev1.z, dev2.z },
			uniforms,
			data0, data1, data2,
			tu0, tu1, tu3
			};
		auto rasterizer = TriangleRasterizer<SCISSOR_ENABLED, decltype(triPgm)>{triPgm, rect, targetHeightInPixels_};
		rasterizer.Draw(int(dev0.x*rglv::FP_MUL), int(dev1.x*rglv::FP_MUL), int(dev2.x*rglv::FP_MUL),
		                int(dev0.y*rglv::FP_MUL), int(dev1.y*rglv::FP_MUL), int(dev2.y*rglv::FP_MUL),
		                !backfacing);}

public:
	static
	auto MakeFragmentProgramPtrs() -> rglv::FragmentProgramPtrs {
		rglv::FragmentProgramPtrs out;
		out.tile_DrawClipped = static_cast<decltype(out.tile_DrawClipped)>(&GPUTileImpl::tile_DrawClipped);
		out.tile_DrawElementsSingle = static_cast<decltype(out.tile_DrawElementsSingle)>(&GPUTileImpl::tile_DrawElementsSingle);
		out.tile_DrawElementsInstanced = static_cast<decltype(out.tile_DrawElementsInstanced)>(&GPUTileImpl::tile_DrawElementsInstanced);
		return out; }};


}  // close package namespace
}  // close enterprise namespace
