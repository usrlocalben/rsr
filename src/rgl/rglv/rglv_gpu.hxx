#pragma once
#include <algorithm>
#include <array>
#include <cassert>
#include <iostream>
#include <numeric>
#include <optional>
#include <vector>

#include "src/rcl/rclmt/rclmt_jobsys.hxx"
#include "src/rcl/rclr/rclr_algorithm.hxx"
#include "src/rcl/rcls/rcls_aligned_containers.hxx"
#include "src/rgl/rglr/rglr_algorithm.hxx"
#include "src/rgl/rglr/rglr_blend.hxx"
#include "src/rgl/rglr/rglr_canvas.hxx"
#include "src/rgl/rglr/rglr_canvas_util.hxx"
#include "src/rgl/rglr/rglr_texture_sampler.hxx"
#include "src/rgl/rglv/rglv_fragment.hxx"
#include "src/rgl/rglv/rglv_gl.hxx"
#include "src/rgl/rglv/rglv_gpu_protocol.hxx"
#include "src/rgl/rglv/rglv_gpu_shaders.hxx"
#include "src/rgl/rglv/rglv_interpolate.hxx"
#include "src/rgl/rglv/rglv_math.hxx"
#include "src/rgl/rglv/rglv_packed_stream.hxx"
#include "src/rgl/rglv/rglv_triangle.hxx"
#include "src/rgl/rglv/rglv_vao.hxx"
#include "src/rgl/rglv/rglv_view_frustum.hxx"
#include "src/rml/rmlg/rmlg_irect.hxx"
#include "src/rml/rmlg/rmlg_triangle.hxx"
#include "src/rml/rmlm/rmlm_mat4.hxx"
#include "src/rml/rmlv/rmlv_soa.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"

#include "3rdparty/fmt/include/fmt/printf.h"

namespace rqdq {
namespace rglv {

constexpr auto blockDimensionsInPixels = rmlv::ivec2{8, 8};

constexpr auto maxVAOSizeInVertices = 500000L;

constexpr auto kMaxSizeInBytes = 100000L;

extern bool doubleBuffer;

struct ClippedVertex {
	rmlv::vec4 coord;  // either clip-coord or device-coord
	uint8_t data[64]; };


struct GPUStats {
	int totalTiles;
	int totalCommands;
	int totalStates;
	int totalCommandBytes;
	int totalTileCommandBytes;
	int minTileCommandBytes;
	int maxTileCommandBytes;
	int totalTrianglesSubmitted;
	int totalTrianglesCulled;
	int totalTrianglesClipped;
	int totalTrianglesDrawn; };


inline Matrices MakeMatrices(const GLState& s) {
	Matrices m;
	m.vm = s.viewMatrix;
	m.pm = s.projectionMatrix;
	m.nm = transpose(inverse(s.viewMatrix));
	m.vpm = s.projectionMatrix * s.viewMatrix;
	return m; }


template <typename TEXTURE_UNIT, typename SHADER_PROGRAM, typename BLEND_PROGRAM>
struct TriangleProgram {
	rmlv::qfloat4* cb_;
	rmlv::qfloat* db_;
	int ofsX_, ofsY_;
	const TEXTURE_UNIT& tu0_, tu1_;

	const int stride_;
	// const int height_;
	// const rmlv::qfloat2 targetDimensions_;
	int offs_, offsLeft_;

	const rglv::VertexFloat1 oneOverW_;
	const rglv::VertexFloat1 zOverW_;

	const typename SHADER_PROGRAM::Interpolants vo_;

	const Matrices matrices_;

	const typename SHADER_PROGRAM::UniformsMD uniforms_;

	TriangleProgram(
		const TEXTURE_UNIT& tu0,
		const TEXTURE_UNIT& tu1,
		rglr::QFloat4Canvas& cc,
		rglr::QFloatCanvas& dc,
		int ofsX,
		int ofsY,
		Matrices matrices,
		typename SHADER_PROGRAM::UniformsMD uniforms,
		VertexFloat1 oneOverW,
		VertexFloat1 zOverW,
		typename SHADER_PROGRAM::VertexOutputSD computed0,
		typename SHADER_PROGRAM::VertexOutputSD computed1,
		typename SHADER_PROGRAM::VertexOutputSD computed2) :
		cb_(cc.data()),
		db_(dc.data()),
		ofsX_(ofsX),
		ofsY_(ofsY),
		tu0_(tu0),
		tu1_(tu1),
		stride_(cc.width() >> 1),
		oneOverW_(oneOverW),
		zOverW_(zOverW),
		vo_(computed0, computed1, computed2),
		matrices_(matrices),
		uniforms_(uniforms) {}

	inline void Begin(int x, int y) {
		x -= ofsX_;
		y -= ofsY_;
		offs_ = offsLeft_ = (y>>1)*stride_ + (x>>1); }

	inline void CR() {
		offsLeft_ += stride_;
		offs_ = offsLeft_; }

	inline void Right2() {
		offs_++; }

	inline void LoadDepth(rmlv::qfloat& destDepth) {
		// from depthbuffer
		// destDepth = _mm_load_ps(reinterpret_cast<float*>(&db[offs]));

		// from alpha
		destDepth = _mm_load_ps(reinterpret_cast<float*>(&(cb_[offs_].a))); }

	inline void StoreDepth(rmlv::qfloat destDepth,
	                       rmlv::qfloat sourceDepth,
	                       rmlv::mvec4i fragMask) {
		// auto addr = &db_[offs_];   // depthbuffer
		auto addr = &(cb_[offs_].a);  // alpha-channel
		auto result = selectbits(destDepth, sourceDepth, fragMask).v;
		_mm_store_ps(reinterpret_cast<float*>(addr), result); }

	inline void LoadColor(rmlv::qfloat4& destColor) {
		auto& cell = cb_[offs_];
		rglr::QFloat4Canvas::Load(cb_+offs_, destColor.x.v, destColor.y.v, destColor.z.v); }

	inline void StoreColor(rmlv::qfloat4 destColor,
	                       rmlv::qfloat4 sourceColor,
	                       rmlv::mvec4i fragMask) {
		auto sr = selectbits(destColor.r, sourceColor.r, fragMask).v;
		auto sg = selectbits(destColor.g, sourceColor.g, fragMask).v;
		auto sb = selectbits(destColor.b, sourceColor.b, fragMask).v;
		rglr::QFloat4Canvas::Store(sr, sg, sb, cb_+offs_); }

	inline void Render(const rmlv::qfloat2 fragCoord, const rmlv::mvec4i triMask, rglv::BaryCoord bary, const bool frontfacing) {
		using rmlv::qfloat, rmlv::qfloat3, rmlv::qfloat4;

		const auto fragDepth = rglv::Interpolate(bary, zOverW_);

		// read depth buffer
		qfloat destDepth; LoadDepth(destDepth);

		const auto depthMask = float2bits(cmple(fragDepth, destDepth));
		const auto fragMask = andnot(triMask, depthMask);

		if (movemask(bits2float(fragMask)) == 0) {
			return; }  // early out if whole quad fails depth test

		// restore perspective
		const auto fragW = rmlv::oneover(Interpolate(bary, oneOverW_));
		bary.x = oneOverW_.v0 * bary.x * fragW;
		bary.z = oneOverW_.v2 * bary.z * fragW;
		bary.y = 1.0f - bary.x - bary.z;

		auto attrs = vo_.Interpolate(bary);

		qfloat4 fragColor;
		SHADER_PROGRAM::ShadeFragment(
			matrices_,
			uniforms_,
			tu0_, tu1_,
			bary, attrs,
			fragCoord,
			/*frontFacing,*/
			fragDepth,
			fragColor);

		qfloat4 destColor;
		LoadColor(destColor);

		// qfloat4 blendedColor = fragColor; // no blending

		StoreColor(destColor, fragColor, fragMask);
		StoreDepth(destDepth, fragDepth, fragMask); } };


struct TileStat {
	int tileId;
	int cost; };


inline void PrintStatistics(const GPUStats& stats) {
	fmt::printf("tiles: % 4d   commands: % 4d   states: % 4d\n", stats.totalTiles, stats.totalCommands, stats.totalStates);
	fmt::printf("command bytes: %d\n", stats.totalCommandBytes);
	fmt::printf("tile bytes total: %d, min: %d, max: %d\n", stats.totalTileCommandBytes, stats.minTileCommandBytes, stats.maxTileCommandBytes);
	fmt::printf("triangles submitted: %d, culled: %d, clipped: %d, drawn: %d\n", stats.totalTrianglesSubmitted, stats.totalTrianglesCulled, stats.totalTrianglesClipped, stats.totalTrianglesDrawn); }


template <typename T, int SZ>
struct SubStack {
	alignas(32) std::array<T, SZ> buffer;
	int sp = 0;

	template <int MANY>
	T* alloc() {
		auto ptr = &buffer[sp];
		sp += MANY;
		return ptr; }

	T operator[](int idx) const {
		return buffer[idx]; }

	void clear() { sp = 0; } };


template<typename ...SHADERS>
class GPU {
public:
	GPU(int concurrency) :
		concurrency_(concurrency),
		threadColorBufs_(concurrency),
		threadDepthBufs_(concurrency) {}

	void Reset(rmlv::ivec2 newBufferDimensionsInPixels, rmlv::ivec2 newTileDimensionsInBlocks) {
		SetSize(newBufferDimensionsInPixels, newTileDimensionsInBlocks);
		for (int t=0, siz=tilesHead_.size(); t<siz; ++t) {
			tilesHead_[t] = WriteRange(t).first; }
		clippedVertexBuffer0_.clear();
		stats0_ = GPUStats{};
		IC().Reset(); }

	GL& IC() {
		return (userIC_ == 0 ? IC0_ : IC1_); }

	void SetSize(rmlv::ivec2 newBufferDimensionsInPixels, rmlv::ivec2 newTileDimensionsInBlocks) {
		if (newBufferDimensionsInPixels != bufferDimensionsInPixels_ ||
		    newTileDimensionsInBlocks != tileDimensionsInBlocks_) {
			bufferDimensionsInPixels_ = newBufferDimensionsInPixels;
			tileDimensionsInBlocks_ = newTileDimensionsInBlocks;
			deviceScale_ = rmlv::qfloat2( bufferDimensionsInPixels_.x/2,
			                             -bufferDimensionsInPixels_.y/2 );
			deviceOffset_ = rmlv::qfloat2( bufferDimensionsInPixels_.x/2,
			                               bufferDimensionsInPixels_.y/2 );
			Retile(); }}

private:
	void Retile() {
		using rmlv::ivec2;
		using std::min, std::max;
		tileDimensionsInPixels_ = tileDimensionsInBlocks_ * blockDimensionsInPixels;
		bufferDimensionsInTiles_ = (bufferDimensionsInPixels_ + (tileDimensionsInPixels_ - ivec2{1, 1})) / tileDimensionsInPixels_;

		for (int ti=0; ti<concurrency_; ++ti) {
			threadColorBufs_[ti].resize(tileDimensionsInPixels_.x, tileDimensionsInPixels_.y);
			threadDepthBufs_[ti].resize(tileDimensionsInPixels_.x, tileDimensionsInPixels_.y); }

		int numTiles = bufferDimensionsInTiles_.x * bufferDimensionsInTiles_.y;
		tilesThreadId_.clear();
		tilesThreadId_.resize(numTiles, 0);

		tilesMem0_.reserve(kMaxSizeInBytes * numTiles);
		tilesMem1_.reserve(kMaxSizeInBytes * numTiles);

		tilesRect_.clear();
		tilesHead_.clear();
		tilesMark_.clear();
		int tid = 0;
		for (int ty=0; ty<bufferDimensionsInTiles_.y; ++ty) {
			for (int tx=0; tx<bufferDimensionsInTiles_.x; ++tx) {
				auto left = tx * tileDimensionsInPixels_.x;
				auto top = ty * tileDimensionsInPixels_.y;
				auto right = min(left + tileDimensionsInPixels_.x, bufferDimensionsInPixels_.x);
				auto bottom = min(top + tileDimensionsInPixels_.y, bufferDimensionsInPixels_.y);
				rmlg::irect bbox{ {left, top}, {right, bottom} };
				tilesRect_.emplace_back(bbox);
				tilesHead_.emplace_back(WriteRange(tid).first);
				tilesMark_.emplace_back(nullptr);
				tilesMem1_[kMaxSizeInBytes * tid] = CMD_EOF;
				++tid; }}}


public:
	auto Run() {
		return rclmt::jobsys::make_job(GPU::RunJmp, std::tuple{this}); }
private:
	static void RunJmp(rclmt::jobsys::Job* job, unsigned tid, std::tuple<GPU*>* data) {
		auto&[self] = *data;
		self->RunImpl(job); }
	void RunImpl(rclmt::jobsys::Job* job) {
		namespace jobsys = rclmt::jobsys;
		auto finalizeJob = Finalize();
		if (job != nullptr) {
			jobsys::move_links(job, finalizeJob); }

		if (!doubleBuffer) {
			BinImpl();
			SwapBuffers(); }

		// sort tiles by their estimated cost before queueing
		tileStats_.clear();
		for (int t=0, siz=tilesHead_.size(); t<siz; ++t) {
			tileStats_.push_back({ t, RenderCost(t) }); }
		rclr::sort(tileStats_, [](auto a, auto b) { return a.cost > b.cost; });
		//std::rotate(tileStats_.begin(), tileStats_.begin() + 1, tileStats_.end())

		std::vector<rclmt::jobsys::Job*> allJobs;
		for (const auto& item : tileStats_) {
			int bi = item.tileId;
			allJobs.push_back(Draw(finalizeJob, bi)); }
		if (doubleBuffer) {
			allJobs.push_back(Bin(finalizeJob)); }

		for (auto& job : allJobs) {
			jobsys::run(job); }
		jobsys::run(finalizeJob); }

	auto Finalize() {
		return rclmt::jobsys::make_job(GPU::FinalizeJmp, std::tuple{this}); }
	static void FinalizeJmp(rclmt::jobsys::Job* job, unsigned tid, std::tuple<GPU*>* data) {
		auto&[self] = *data;
		self->FinalizeImpl(); }
	void FinalizeImpl() {
		//printStatistics(stats1_);
		if (doubleBuffer) {
			SwapBuffers(); } }

	void SwapBuffers() {
		// double-buffer swap
		std::swap(tilesMem0_, tilesMem1_);
		std::swap(clippedVertexBuffer0_, clippedVertexBuffer1_);
		std::swap(stats0_, stats1_);
		userIC_ = (userIC_+1) & 0x01; }

	/*
	 * process the command stream to create the tile streams
	 */
	auto Bin(rclmt::jobsys::Job *parent) {
		return rclmt::jobsys::make_job_as_child(parent, GPU::BinJmp, std::tuple{this}); }
	static void BinJmp(rclmt::jobsys::Job* job, unsigned tid, std::tuple<GPU*>* data) {
		auto&[self] = *data;
		self->BinImpl(); }
	void BinImpl() {
		using fmt::printf;
		// printf("----- BEGIN BINNING -----\n");

		binState = nullptr;
		binUniforms = nullptr;
		auto& cs = IC().commands_;
		cs.appendByte(CMD_EOF);
		int totalCommands = 0;
		while (1) {
			totalCommands += 1;
			auto cmd = cs.consumeByte();
			// printf("cmd %02x %d:", cmd, cmd);
			if (cmd == CMD_EOF) {
				for (int t=0, siz=tilesHead_.size(); t<siz; ++t) {
					AppendByte(t, cmd); }
				break; }
			else if (cmd == CMD_STATE) {
				binState = static_cast<GLState*>(cs.consumePtr());
				binUniforms = IC().GetUniformBufferAddr(binState->uniformsOfs);
				// printf(" state is now at %p\n", binState);
				for (int t=0, siz=tilesHead_.size(); t<siz; ++t) {
					AppendByte(t, cmd);
					AppendPtr(t, binState);
					AppendPtr(t, binUniforms); }}
			else if (cmd == CMD_CLEAR) {
				auto arg = cs.consumeByte();
				// printf(" clear with color ");
				// std::cout << color << std::endl;
				for (int t=0, siz=tilesHead_.size(); t<siz; ++t) {
					AppendByte(t, cmd);
					AppendByte(t, arg); }}
			else if (cmd == CMD_STORE_COLOR_HALF_LINEAR_FP) {
				auto ptr = cs.consumePtr();
				// printf(" store halfsize colorbuffer @ %p\n", ptr);
				for (int t=0, siz=tilesHead_.size(); t<siz; ++t) {
					AppendByte(t, cmd);
					AppendPtr(t, ptr); }}
			else if (cmd == CMD_STORE_COLOR_FULL_LINEAR_FP) {
				auto ptr = cs.consumePtr();
				// printf(" store unswizzled colorbuffer @ %p\n", ptr);
				for (int t=0, siz=tilesHead_.size(); t<siz; ++t) {
					AppendByte(t, cmd);
					AppendPtr(t, ptr); }}
			else if (cmd == CMD_STORE_COLOR_FULL_QUADS_FP) {
				auto ptr = cs.consumePtr();
				// printf(" store swizzled colorbuffer @ %p\n", ptr);
				for (int t=0, siz=tilesHead_.size(); t<siz; ++t) {
					AppendByte(t, cmd);
					AppendPtr(t, ptr); }}
			else if (cmd == CMD_STORE_COLOR_FULL_LINEAR_TC) {
				auto enableGamma = cs.consumeByte();
				auto ptr = cs.consumePtr();
				// printf(" store truecolor @ %p, gamma corrected? %s\n", ptr, (enableGamma?"Yes":"No"));
				for (int t=0, siz=tilesHead_.size(); t<siz; ++t) {
					AppendByte(t, cmd);
					AppendByte(t, enableGamma);
					AppendPtr(t, ptr); }}
			else if (cmd == CMD_DRAW_ARRAYS) {
				//auto flags = cs.consumeByte();
				//assert(flags == 0x14);  // videocore: 16-bit indices, triangles
				auto count = cs.consumeInt();
				struct SequenceSource {
					int operator()(int ti) { return ti; }};
				SequenceSource indexSource{};
				bin_Draw<false, SequenceSource, SHADERS...>(count, indexSource, 1); }
			else if (cmd == CMD_DRAW_ARRAYS_INSTANCED) {
				//auto flags = cs.consumeByte();
				//assert(flags == 0x14);  // videocore: 16-bit indices, triangles
				auto count = cs.consumeInt();
				auto instanceCnt = cs.consumeInt();
				struct SequenceSource {
					int operator()(int ti) { return ti; }};
				SequenceSource indexSource{};
				bin_Draw<true, SequenceSource, SHADERS...>(count, indexSource, instanceCnt); }
			else if (cmd == CMD_DRAW_ELEMENTS) {
				auto flags = cs.consumeByte();
				assert(flags == 0x14);  // videocore: 16-bit indices, triangles
				auto count = cs.consumeInt();
				auto indices = static_cast<uint16_t*>(cs.consumePtr());
				struct ArraySource {
					ArraySource(const uint16_t* data) : data_(data) {}
					int operator()(int ti) { return data_[ti]; }
					const uint16_t* const data_; };
				ArraySource indexSource{indices};
				bin_Draw<false, ArraySource, SHADERS...>(count, indexSource, 1); }
			else if (cmd == CMD_DRAW_ELEMENTS_INSTANCED) {
				auto flags = cs.consumeByte();
				assert(flags == 0x14);  // videocore: 16-bit indices, triangles
				auto count = cs.consumeInt();
				auto indices = static_cast<uint16_t*>(cs.consumePtr());
				auto instanceCnt = cs.consumeInt();
				struct ArraySource {
					ArraySource(const uint16_t* data) : data_(data) {}
					int operator()(int ti) { return data_[ti]; }
					const uint16_t* const data_; };
				ArraySource indexSource{indices};
				bin_Draw<true, ArraySource, SHADERS...>(count, indexSource, instanceCnt); } }

		// stats...
		int total_ = 0;
		int min_ = 0x7fffffff;
		int max_ = 0;
		for (int t=0, siz=tilesHead_.size(); t<siz; ++t) {
			const int thisTileBytes = tilesHead_[t] - WriteRange(t).first;
			total_ += thisTileBytes;
			min_ = std::min(min_, thisTileBytes);
			max_ = std::max(max_, thisTileBytes); }
		stats0_.totalTiles = tilesHead_.size();
		stats0_.totalCommands = totalCommands;
		// stats0_.totalStates = IC().d_si;
		stats0_.totalCommandBytes = cs.size();
		stats0_.totalTileCommandBytes = total_;
		stats0_.minTileCommandBytes = min_;
		stats0_.maxTileCommandBytes = max_; }

	auto Draw(rclmt::jobsys::Job* parent, int tileId) {
		return rclmt::jobsys::make_job_as_child(parent, GPU::DrawJmp, std::tuple{this, tileId}); }
	static void DrawJmp(rclmt::jobsys::Job* job, unsigned tid, std::tuple<GPU*, int>* data) {
		auto&[self, tileId] = *data;
		self->DrawImpl(tid, tileId); }
	void DrawImpl(const unsigned tid, const int tileIdx) {
		const auto rect = tilesRect_[tileIdx];
		tilesThreadId_[tileIdx] = tid;

		FastPackedReader cs{ ReadRange(tileIdx).first };
		int cmdCnt = 0;

		auto& dc = threadDepthBufs_[rclmt::jobsys::thread_id];
		auto& cc = threadColorBufs_[rclmt::jobsys::thread_id];

		const GLState* stateptr = nullptr;
		const void* uniformptr = nullptr;
		while (1) {
			++cmdCnt;
			auto cmd = cs.ConsumeByte();
			// fmt::printf("tile(%d) cmd(%02x, %d)\n", tileIdx, cmd, cmd);
			if (cmd == CMD_EOF) {
				break; }
			else if (cmd == CMD_STATE) {
				stateptr = static_cast<const GLState*>(cs.ConsumePtr());
				uniformptr = cs.ConsumePtr(); }
			else if (cmd == CMD_CLEAR) {
				int bits = cs.ConsumeByte();
				if ((bits & GL_COLOR_BUFFER_BIT) != 0) {
					// std::cout << "clearing to " << color << std::endl;
					Fill(cc, stateptr->clearColor, rect); }
				if ((bits & GL_DEPTH_BUFFER_BIT) != 0) {
					Fill(dc, stateptr->clearDepth, rect); }
				if ((bits & GL_STENCIL_BUFFER_BIT) != 0) {
					// XXX not implemented
					assert(false); }}
			else if (cmd == CMD_STORE_COLOR_HALF_LINEAR_FP) {
				auto dst = static_cast<rglr::FloatingPointCanvas*>(cs.ConsumePtr());
				Downsample(cc, *dst, rect); }
			else if (cmd == CMD_STORE_COLOR_FULL_LINEAR_FP) {
				auto dst = static_cast<rglr::FloatingPointCanvas*>(cs.ConsumePtr());
				Copy(cc, *dst, rect); }
			else if (cmd == CMD_STORE_COLOR_FULL_QUADS_FP) {
				auto dst = static_cast<rglr::QFloat4Canvas*>(cs.ConsumePtr());
				Copy(cc, *dst, rect); }
			else if (cmd == CMD_STORE_COLOR_FULL_LINEAR_TC) {
				auto enableGamma = cs.ConsumeByte();
				auto& outcanvas = *static_cast<rglr::TrueColorCanvas*>(cs.ConsumePtr());
				tile_StoreTrueColor<SHADERS...>(*stateptr, uniformptr, rect, enableGamma, outcanvas);
				// XXX draw cpu assignment indicators draw_border(rect, cpu_colors[tid], canvas);
				}
			else if (cmd == CMD_CLIPPED_TRI) {
				tile_DrawClipped<SHADERS...>(*stateptr, uniformptr, rect, tileIdx, cs); }
			else if (cmd == CMD_DRAW_INLINE) {
				tile_DrawElements<false, SHADERS...>(*stateptr, uniformptr, rect, tileIdx, cs); }
			else if (cmd == CMD_DRAW_INLINE_INSTANCED) {
				tile_DrawElements<true, SHADERS...>(*stateptr, uniformptr, rect, tileIdx, cs); }}}

	template <typename ...PGMs>
	typename std::enable_if<sizeof...(PGMs) == 0>::type tile_StoreTrueColor(const GLState& state, const void* uniformsPtr, const rmlg::irect rect, const bool enableGamma, rglr::TrueColorCanvas& outcanvas) {}

	template <typename PGM, typename ...PGMs>
	void tile_StoreTrueColor(const GLState& state, const void* uniformsPtr, const rmlg::irect rect, const bool enableGamma, rglr::TrueColorCanvas& outcanvas) {
		if (state.programId != PGM::id) {
			return tile_StoreTrueColor<PGMs...>(state, uniformsPtr, rect, enableGamma, outcanvas); }

		// XXX add support for uniforms in stream-out shader

		auto& cc = threadColorBufs_[rclmt::jobsys::thread_id];
		if (enableGamma) {
			rglr::FilterTile<PGM, rglr::sRGB>(cc, outcanvas, rect); }
		else {
			rglr::FilterTile<PGM, rglr::LinearColor>(cc, outcanvas, rect); }}

	template <bool INSTANCED, typename INDEX_SOURCE, typename ...PGMs>
	typename std::enable_if<sizeof...(PGMs) == 0>::type bin_Draw(const int count, INDEX_SOURCE& indexSource, const int instanceCnt) {}

	template <bool INSTANCED, typename INDEX_SOURCE, typename PGM, typename ...PGMs>
	void bin_Draw(const int count, INDEX_SOURCE& indexSource, const int instanceCnt) {
		if (PGM::id != binState->programId) {
			return bin_Draw<INSTANCED, INDEX_SOURCE, PGMs...>(count, indexSource, instanceCnt); }
		using std::min, std::max, std::swap;
		using rmlv::ivec2, rmlv::qfloat, rmlv::qfloat2, rmlv::qfloat3, rmlv::qfloat4, rmlm::qmat4;
		const auto& state = *binState;

		const auto matrices = MakeMatrices(state);
		const typename PGM::UniformsMD uniforms(*static_cast<const typename PGM::UniformsSD*>(binUniforms));
		const auto cullingEnabled = state.cullingEnabled;
		const auto cullFace = state.cullFace;
		typename PGM::Loader loader( state.buffers, state.bufferFormat );
		const auto frustum = ViewFrustum{ bufferDimensionsInPixels_ };

		typename PGM::VertexInput vertex;

		clipQueue_.clear();

		if (!INSTANCED) {
			// coding error if this is not true
			// XXX could use constexpr instead of template param?
			assert(instanceCnt == 1); }

		const auto cmd = INSTANCED ? CMD_DRAW_INLINE_INSTANCED : CMD_DRAW_INLINE;
		for (int t=0, siz=tilesHead_.size(); t<siz; ++t) {
			AppendByte(t, cmd);
			Mark(t); }

		for (int iid=0; iid<instanceCnt; ++iid) {
			if (INSTANCED) {
				loader.LoadInstance(iid, vertex); }

			clipFlagBuffer_.clear();
			devCoordBuffer_.clear();

			const auto siz = loader.Size();
			// xxx const int rag = siz % 4;  assume vaos are always padded to size()%4=0

			for (int vi=0; vi<siz; vi+=4) {
				loader.LoadMD(vi, vertex);

				qfloat4 coord;
				typename PGM::VertexOutputMD unused;
				PGM::ShadeVertex(matrices, uniforms, vertex, coord, unused);

				auto flags = frustum.Test(coord);
				store_bytes(clipFlagBuffer_.alloc<4>(), flags);

				auto devCoord = pdiv(coord).xy() * deviceScale_ + deviceOffset_;
				devCoord.copyTo(devCoordBuffer_.alloc<4>()); }

			for (int ti=0; ti<count; ti+=3) {
				stats0_.totalTrianglesSubmitted++;

				uint16_t ibuf[4];
				uint16_t& i0 = ibuf[1];
				uint16_t& i1 = ibuf[2];
				uint16_t& i2 = ibuf[3];
				uint8_t* ibufptr;
				if (INSTANCED) {
					ibufptr = reinterpret_cast<uint8_t*>(ibuf);
					ibuf[0] = iid; }
				else {
					ibufptr = reinterpret_cast<uint8_t*>(ibuf+1); }

				i0 = indexSource(ti);
				i1 = indexSource(ti+1);
				i2 = indexSource(ti+2);

				// check for triangles that need clipping
				const auto cf0 = clipFlagBuffer_[i0];
				const auto cf1 = clipFlagBuffer_[i1];
				const auto cf2 = clipFlagBuffer_[i2];
				if (cf0 | cf1 | cf2) {
					if (cf0 & cf1 & cf2) {
						// all points outside of at least one plane
						stats0_.totalTrianglesCulled++;
						continue; }
					// queue for clipping
					clipQueue_.push_back({ iid, i0, i1, i2 });
					continue; }

				auto devCoord0 = devCoordBuffer_[i0];
				auto devCoord1 = devCoordBuffer_[i1];
				auto devCoord2 = devCoordBuffer_[i2];

				// handle backfacing tris and culling
				const bool backfacing = rmlg::triangle2Area(devCoord0, devCoord1, devCoord2) < 0;
				if (backfacing) {
					if (cullingEnabled && cullFace == GL_BACK) {
						stats0_.totalTrianglesCulled++;
						continue; }
					// devCoord is _not_ swapped, but relies on the aabb method that ForEachCoveredBin uses!
					swap(i0, i2);
					i0 |= 0x8000; }  // add backfacing flag
				else {
					if (cullingEnabled && cullFace == GL_FRONT) {
						stats0_.totalTrianglesCulled++;
						continue; }}

				ForEachCoveredTile(devCoord0, devCoord1, devCoord2, ibufptr, INSTANCED ? 8 : 6); }

			}  // instance loop

		for (int t=0, siz=tilesHead_.size(); t<siz; ++t) {
			if (Touched(t)) {
				AppendUShort(t, 0xffff); }
			else {
				// remove the command from any tiles
				// that weren't covered by this draw
				Unappend(t, 1); }}

		stats0_.totalTrianglesClipped = clipQueue_.size();
		if (!clipQueue_.empty()) {
			bin_DrawElementsClipped<INSTANCED, PGM>(state); }}

	template <bool INSTANCED, typename ...PGMs>
	typename std::enable_if<sizeof...(PGMs) == 0>::type tile_DrawElements(const GLState& state, const void* uniformsPtr, const rmlg::irect& rect, int tileIdx, FastPackedReader& cs) {}

	template<bool INSTANCED, typename PGM, typename ...PGMs>
	void tile_DrawElements(const GLState& state, const void* uniformsPtr, const rmlg::irect& rect, int tileIdx, FastPackedReader& cs) {
		if (state.programId != PGM::id) {
			return tile_DrawElements<INSTANCED, PGMs...>(state, uniformsPtr, rect, tileIdx, cs); }

		using rmlm::mat4;
		using rmlv::vec2, rmlv::vec3, rmlv::vec4;
		using rmlv::mvec4i;
		using rmlv::qfloat, rmlv::qfloat2, rmlv::qfloat4;
		using std::array, std::swap;

		typename PGM::Loader loader( state.buffers, state.bufferFormat );

		auto& cbc = threadColorBufs_[rclmt::jobsys::thread_id];
		auto& dbc = threadDepthBufs_[rclmt::jobsys::thread_id];
		const int target_height = cbc.height();

		const auto matrices = MakeMatrices(state);
		const typename PGM::UniformsMD uniforms(*static_cast<const typename PGM::UniformsSD*>(uniformsPtr));

		using sampler = rglr::ts_pow2_mipmap;
		const sampler tu0(state.tus[0].ptr, state.tus[0].width, state.tus[0].height, state.tus[0].stride, state.tus[0].filter);
		const sampler tu1(state.tus[1].ptr, state.tus[1].width, state.tus[1].height, state.tus[1].stride, state.tus[1].filter);

		array<typename PGM::VertexInput, 3> vertex;
		array<typename PGM::VertexOutputMD, 3> computed;
		array<qfloat4, 3> devCoord;
		array<mvec4i, 3>  fx;
		array<mvec4i, 3>  fy;
		bool backfacing;
		int li = 0;  // sse lane being loaded

		auto flush = [&]() {
			for (int i=0; i<3; ++i) {
				qfloat4 gl_Position;
				PGM::ShadeVertex(matrices, uniforms, vertex[i], gl_Position, computed[i]);
				devCoord[i] = pdiv(gl_Position);
				fx[i] = ftoi(16.0F * (devCoord[i].x * deviceScale_.x + deviceOffset_.x));
				fy[i] = ftoi(16.0F * (devCoord[i].y * deviceScale_.y + deviceOffset_.y)); }

			// draw up to 4 triangles
			for (int ti=0; ti<li; ti++) {
				TriangleProgram<sampler, PGM, rglr::BlendProgram::Set> rasterizerProgram{
					tu0, tu1,
					cbc, dbc,
					rect.left.x, rect.top.y,
					matrices,
					uniforms,
					VertexFloat1{ devCoord[0].w.lane[ti], devCoord[1].w.lane[ti], devCoord[2].w.lane[ti] },
					VertexFloat1{ devCoord[0].z.lane[ti], devCoord[1].z.lane[ti], devCoord[2].z.lane[ti] },
					computed[0].Lane(ti),
					computed[1].Lane(ti),
					computed[2].Lane(ti) };
				TriangleRasterizer rasterizesr(rasterizerProgram, rect, target_height);
				rasterizesr.Draw(fx[0].si[ti], fx[1].si[ti], fx[2].si[ti],
				                 fy[0].si[ti], fy[1].si[ti], fy[2].si[ti],
				                 !backfacing); }

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
				flush();
				break; }

			if (INSTANCED) {
				const int iid = firstWord;
				if (iid != loadedInstanceId) {
					flush();
					loadedInstanceId = iid;
					loader.LoadInstance(iid, vertex[0]);
					loader.LoadInstance(iid, vertex[1]);
					loader.LoadInstance(iid, vertex[2]); }}

			{
				uint16_t tmp;
				if (INSTANCED) {
					tmp = cs.ConsumeUShort(); }
				else {
					tmp = firstWord; }
				const uint16_t i1 = cs.ConsumeUShort();
				const uint16_t i2 = cs.ConsumeUShort();
				backfacing = tmp & 0x8000;
				const uint16_t i0 = tmp & 0x7fff;
				loader.LoadLane(i0, li, vertex[0]);
				loader.LoadLane(i1, li, vertex[1]);
				loader.LoadLane(i2, li, vertex[2]);
				li += 1; }

			if (li == 4) {
				flush(); }}


		}  // func

	template <bool INSTANCED, typename PGM>
	void bin_DrawElementsClipped(const GLState& state) {
		using rmlm::mat4;
		using rmlv::ivec2, rmlv::vec2, rmlv::vec3, rmlv::vec4, rmlv::qfloat4;
		using std::array, std::swap, std::min, std::max;

		typename PGM::Loader loader( state.buffers, state.bufferFormat );
		const auto frustum = ViewFrustum{ bufferDimensionsInPixels_.x };

		const auto matrices = MakeMatrices(state);
		const typename PGM::UniformsMD uniforms(*static_cast<const typename PGM::UniformsSD*>(binUniforms));

		auto CVMix = [&](const ClippedVertex& a, const ClippedVertex& b, const float d) {
			static_assert(sizeof(typename PGM::VertexOutputSD) <= sizeof(ClippedVertex::data), "shader output too large for clipped vertex buffer");
			ClippedVertex out;
			out.coord = mix(a.coord, b.coord, d);
			auto& ad = *reinterpret_cast<const typename PGM::VertexOutputSD*>(&a.data);
			auto& bd = *reinterpret_cast<const typename PGM::VertexOutputSD*>(&b.data);
			auto& od = *reinterpret_cast<      typename PGM::VertexOutputSD*>(&out.data);
			od = PGM::VertexOutputSD::Mix(ad, bd, d);
			return out; };

		for (const auto& faceIndices : clipQueue_) {
			// phase 1: load _poly_ with the shaded vertex data
			clipA_.clear();
			for (int i=0; i<3; ++i) {
				const int iid = faceIndices[0];
				const auto idx = faceIndices[i+1];

				typename PGM::VertexInput vertex;
				loader.LoadLane(idx, 0, vertex);
				if (INSTANCED) {
					loader.LoadInstance(iid, vertex); }

				qfloat4 coord;
				typename PGM::VertexOutputMD computed;
				PGM::ShadeVertex(matrices, uniforms, vertex, coord, computed);

				ClippedVertex cv;
				cv.coord = coord.lane(0);
				auto* tmp = reinterpret_cast<typename PGM::VertexOutputSD*>(&cv.data);
				*tmp = computed.Lane(0);
				clipA_.emplace_back(cv); }

			// phase 2: sutherland-hodgman clipping
			for (const auto plane : rglv::clipping_panes) {

				bool hereIsInside = frustum.IsInside(plane, clipA_[0].coord);

				clipB_.clear();
				for (int here_vi = 0; here_vi < clipA_.size(); ++here_vi) {
					const auto next_vi = (here_vi + 1) % clipA_.size(); // wrap

					const auto& here = clipA_[here_vi];
					const auto& next = clipA_[next_vi];

					const bool nextIsInside = frustum.IsInside(plane, next.coord);

					if (hereIsInside) {
						clipB_.push_back(here); }

					if (hereIsInside != nextIsInside) {
						hereIsInside = !hereIsInside;
						const float d = frustum.Distance(plane, here.coord, next.coord);
						auto newVertex = CVMix(here, next, d);
						clipB_.emplace_back(newVertex); }}

				swap(clipA_, clipB_);
				if (clipA_.size() == 0) {
					break; }
				assert(clipA_.size() >= 3); }

			if (clipA_.size() == 0) {
				continue; }

			for (auto& vertex : clipA_) {
				// convert clip-coord to device-coord
				vertex.coord = pdiv(vertex.coord);
				vertex.coord.x = (vertex.coord.x * deviceScale_.x + deviceOffset_.x).get_x();
				vertex.coord.y = (vertex.coord.y * deviceScale_.y + deviceOffset_.y).get_x(); }
			// end of phase 2: poly contains a clipped N-gon

			// check direction, maybe cull, maybe reorder
			const bool backfacing = rmlg::triangle2Area(clipA_[0].coord, clipA_[1].coord, clipA_[2].coord) < 0;
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
			int bi = clippedVertexBuffer0_.size();
			int hits{0};
			for (int vi=1; vi<clipA_.size() - 1; ++vi) {
				int i0{0}, i1{vi}, i2{vi+1};
				uint8_t buf[7];
				buf[0] = CMD_CLIPPED_TRI;
				*reinterpret_cast<uint16_t*>(buf+1) = (bi+i0) | (backfacing ? 0x8000 : 0);
				*reinterpret_cast<uint16_t*>(buf+3) = (bi+i1);
				*reinterpret_cast<uint16_t*>(buf+5) = (bi+i2);
				hits += ForEachCoveredTile(clipA_[i0].coord.xy(), clipA_[i1].coord.xy(), clipA_[i2].coord.xy(), buf, 7);  }

			if (hits > 0) {
				std::copy(begin(clipA_), end(clipA_), back_inserter(clippedVertexBuffer0_)); }}}

	int ForEachCoveredTile(const rmlv::vec2 dc0, const rmlv::vec2 dc1, const rmlv::vec2 dc2, uint8_t* buf, int cnt) {
		using std::min, std::max, rmlv::ivec2;
		const ivec2 idev0{ dc0 };
		const ivec2 idev1{ dc1 };
		const ivec2 idev2{ dc2 };

		const int vminx = max(rmlv::Min(idev0.x, idev1.x, idev2.x), 0);
		const int vminy = max(rmlv::Min(idev0.y, idev1.y, idev2.y), 0);
		const int vmaxx = min(rmlv::Max(idev0.x, idev1.x, idev2.x), bufferDimensionsInPixels_.x-1);
		const int vmaxy = min(rmlv::Max(idev0.y, idev1.y, idev2.y), bufferDimensionsInPixels_.y-1);

		auto topLeft = ivec2{ vminx, vminy } / tileDimensionsInPixels_;
		auto bottomRight = ivec2{ vmaxx, vmaxy } / tileDimensionsInPixels_;

		int stride = bufferDimensionsInTiles_.x;
		int tidRow = topLeft.y * stride;
		int numHits{0};
		for (int ty = topLeft.y; ty <= bottomRight.y; ++ty, tidRow+=stride) {
			for (int tx = topLeft.x; tx <= bottomRight.x; ++tx) {
				int tid = tidRow + tx;
				++numHits;
				std::memcpy(tilesHead_[tid], buf, cnt);
				tilesHead_[tid] += cnt; }}
		return numHits; }

	template <typename ...PGMs>
	typename std::enable_if<sizeof...(PGMs) == 0>::type tile_DrawClipped(const GLState& state, const void* uniformsPtr, const rmlg::irect& rect, int tileIdx, FastPackedReader& cs) {}

	template <typename PGM, typename ...PGMs>
	void tile_DrawClipped(const GLState& state, const void* uniformsPtr, const rmlg::irect& rect, int tileIdx, FastPackedReader& cs) {
		if (state.programId != PGM::id) {
			return tile_DrawClipped<PGMs...>(state, uniformsPtr, rect, tileIdx, cs); }
		using rmlm::mat4;
		using rmlv::vec2, rmlv::vec3, rmlv::vec4;

		auto& cbc = threadColorBufs_[rclmt::jobsys::thread_id];
		auto& dbc = threadDepthBufs_[rclmt::jobsys::thread_id];
		const int target_height = cbc.height();

		const auto matrices = MakeMatrices(state);
		const typename PGM::UniformsMD uniforms(*static_cast<const typename PGM::UniformsSD*>(uniformsPtr));

		const auto tmp = cs.ConsumeUShort();
		const bool backfacing = (tmp & 0x8000) != 0;
		const auto i0 = tmp & 0x7fff;
		const auto i1 = cs.ConsumeUShort();
		const auto i2 = cs.ConsumeUShort();

		vec4 dev0 = clippedVertexBuffer1_[i0].coord;
		vec4 dev1 = clippedVertexBuffer1_[i1].coord;
		vec4 dev2 = clippedVertexBuffer1_[i2].coord;
		auto& data0 = *reinterpret_cast<typename PGM::VertexOutputSD*>(&(clippedVertexBuffer1_[i0].data));
		auto& data1 = *reinterpret_cast<typename PGM::VertexOutputSD*>(&(clippedVertexBuffer1_[i1].data));
		auto& data2 = *reinterpret_cast<typename PGM::VertexOutputSD*>(&(clippedVertexBuffer1_[i2].data));

		using sampler = rglr::ts_pow2_mipmap;
		const sampler tu0(state.tus[0].ptr, state.tus[0].width, state.tus[0].height, state.tus[0].stride, state.tus[0].filter);
		const sampler tu1(state.tus[1].ptr, state.tus[1].width, state.tus[1].height, state.tus[1].stride, state.tus[1].filter);

		TriangleProgram<sampler, PGM, rglr::BlendProgram::Set> rasterizerProgram{
			tu0, tu1,
			cbc, dbc,
			rect.left.x, rect.top.y,
			matrices,
			uniforms,
			VertexFloat1{ dev0.w, dev1.w, dev2.w },
			VertexFloat1{ dev0.z, dev1.z, dev2.z },
			data0,
			data1,
			data2 };
		TriangleRasterizer rasterizer(rasterizerProgram, rect, target_height);
		rasterizer.Draw(int(dev0.x*16.0F), int(dev1.x*16.0F), int(dev2.x*16.0F),
		                int(dev0.y*16.0F), int(dev1.y*16.0F), int(dev2.y*16.0F),
		                !backfacing); }

	template<int MANY>
	uint8_t* alloc(int t) {
		auto pos = tilesHead_[t];
		tilesHead_[t] += MANY;
		// XXX assert(tilesHead_[t] < kMaxSizeInBytes);
		return pos; }
	void AppendByte(int t, uint8_t a) {
		*reinterpret_cast<uint8_t*>(alloc<sizeof(uint8_t)>(t)) = a; }
	void AppendUShort(int t, uint16_t a) {
		*reinterpret_cast<uint16_t*>(alloc<sizeof(uint16_t)>(t)) = a; }
	void AppendInt(int t, int a) {
		*reinterpret_cast<int*>(alloc<sizeof(int)>(t)) = a; }
	void AppendFloat(int t, float a) {
		*reinterpret_cast<float*>(alloc<sizeof(float)>(t)) = a; }
	void AppendPtr(int t, const void * const a) {
		*reinterpret_cast<const void**>(alloc<sizeof(void*)>(t)) = a; }
	void AppendVec4(int t, rmlv::vec4 a) {
		AppendFloat(t, a.x);
		AppendFloat(t, a.y);
		AppendFloat(t, a.z);
		AppendFloat(t, a.w); }

	auto Unappend(int t, int many) {
		tilesHead_[t] -= many; }

	auto Mark(int t) {
		tilesMark_[t] = tilesHead_[t]; }

	bool Touched(int t) const {
		return tilesMark_[t] != tilesHead_[t]; }

	/* XXX bool Eof(int t) const {
		return tilesTail_[t*16] == tilesHead_[t]; }*/

	/*
	 * use the command list length as an approximate compute cost
	 */
	inline int RenderCost(int t) const {
		auto begin = WriteRange(t).first;
		return tilesHead_[t] - begin; }

	std::pair<uint8_t*, uint8_t*> WriteRange(int t) const {
		uint8_t* begin = const_cast<uint8_t*>(tilesMem0_.data()) + t*kMaxSizeInBytes;
		uint8_t* end = begin + kMaxSizeInBytes;
		return { begin, end }; }

	std::pair<uint8_t*, uint8_t*> ReadRange(int t) const {
		uint8_t* begin = const_cast<uint8_t*>(tilesMem1_.data()) + t*kMaxSizeInBytes;
		uint8_t* end = begin + kMaxSizeInBytes;
		return { begin, end }; }

public:
	void ColorCanvas(rglr::QFloat4Canvas* ptr) {}
	void DepthCanvas(rglr::QFloatCanvas* ptr) {}

private:
	const int concurrency_;
	std::vector<rglr::QFloat4Canvas> threadColorBufs_{};
	std::vector<rglr::QFloatCanvas>  threadDepthBufs_{};

	// tile/bin collection
	std::vector<int> tilesThreadId_;
	std::vector<rmlg::irect> tilesRect_;
	std::vector<uint8_t*> tilesHead_;
	std::vector<uint8_t*> tilesMark_;
	std::vector<uint8_t> tilesMem0_;
	std::vector<uint8_t> tilesMem1_;
	std::vector<TileStat> tileStats_;

	// render target parameters
	rmlv::ivec2 bufferDimensionsInPixels_;
	rmlv::ivec2 bufferDimensionsInTiles_;
	rmlv::ivec2 tileDimensionsInBlocks_;
	rmlv::ivec2 tileDimensionsInPixels_;
	rmlv::qfloat2 deviceScale_;
	rmlv::qfloat2 deviceOffset_;

	// IC users can write to
	int userIC_{0};

	// double-buffered GL contexts: binning
	GL IC0_;
	rcls::vector<ClippedVertex> clippedVertexBuffer0_;
	GPUStats stats0_;

	// buffers used during binning and clipping
	const GLState* binState{nullptr};
	const void* binUniforms{nullptr};
	SubStack<uint8_t, maxVAOSizeInVertices> clipFlagBuffer_;
	SubStack<rmlv::vec2, maxVAOSizeInVertices> devCoordBuffer_;
	rcls::vector<std::array<int, 4>> clipQueue_;
	rcls::vector<ClippedVertex> clipA_;
	rcls::vector<ClippedVertex> clipB_;

	// double-buffered GL contexts: drawing
	GL IC1_;
	rcls::vector<ClippedVertex> clippedVertexBuffer1_;
	GPUStats stats1_; };


}  // namespace rglv
}  // namespace rqdq
