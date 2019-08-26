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


struct ClippedVertex {
	rmlv::vec4 coord;  // either clip-coord or device-coord
	VertexOutputx1 data; };


inline ClippedVertex mix(ClippedVertex a, ClippedVertex b, float d) {
	return { mix(a.coord, b.coord, d),
	         mix(a.data, b.data, d) }; }


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


template <typename TEXTURE_UNIT, typename FRAGMENT_PROGRAM, typename BLEND_PROGRAM>
struct DefaultTargetProgram {

	// const FRAGMENT_PROGRAM& fp;
	// const BLEND_PROGRAM& bp;

	rmlv::qfloat4* cb_;
	rmlv::qfloat4* cbx_;
	rmlv::qfloat* db_;
	rmlv::qfloat* dbx_;

	const TEXTURE_UNIT& tu0_, tu1_;

	const int width_;
	const int height_;
	const rmlv::qfloat2 targetDimensions_;
	int offs_, offsLeft_;

	const rglv::VertexFloat1 oneOverW_;
	const rglv::VertexFloat1 zOverW_;

	const rglv::VertexFloat3 vo0_;
	const rglv::VertexFloat3 vo1_;
	const rglv::VertexFloat3 vo2_;
	const rglv::VertexFloat3 vo3_;

	const ShaderUniforms uniforms_;

	DefaultTargetProgram(
		const TEXTURE_UNIT& tu0,
		const TEXTURE_UNIT& tu1,
		rglr::QFloat4Canvas& cc,
		rglr::QFloatCanvas& dc,
		ShaderUniforms uniforms,
		VertexFloat1 oneOverW,
		VertexFloat1 zOverW,
		VertexOutputx1 computed0,
		VertexOutputx1 computed1,
		VertexOutputx1 computed2) :
		cb_(cc.data()),
		db_(dc.data()),
		tu0_(tu0),
		tu1_(tu1),
		width_(cc.width()),
		height_(cc.height()),
		targetDimensions_({ float(cc.width()), float(cc.height()) }),
		oneOverW_(oneOverW),
		zOverW_(zOverW),
		vo0_({ computed0.r0, computed1.r0, computed2.r0 }),
		vo1_({ computed0.r1, computed1.r1, computed2.r1 }),
		vo2_({ computed0.r2, computed1.r2, computed2.r2 }),
		vo3_({ computed0.r3, computed1.r3, computed2.r3 }),
		uniforms_(uniforms) {}

	inline void Begin(int x, int y) {
		offs_ = offsLeft_ = (y >> 1) * (width_ >> 1) + (x >> 1); }

	inline void CR() {
		offsLeft_ += width_ >> 1;
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
		destColor.r = _mm_load_ps(reinterpret_cast<float*>(&(cb_[offs_].r)));
		destColor.g = _mm_load_ps(reinterpret_cast<float*>(&(cb_[offs_].g)));
		destColor.b = _mm_load_ps(reinterpret_cast<float*>(&(cb_[offs_].b))); }

	inline void StoreColor(rmlv::qfloat4 destColor,
	                       rmlv::qfloat4 sourceColor,
	                       rmlv::mvec4i fragMask) {
		_mm_store_ps(reinterpret_cast<float*>(&(cb_[offs_].r)), selectbits(destColor.r, sourceColor.r, fragMask).v);
		_mm_store_ps(reinterpret_cast<float*>(&(cb_[offs_].g)), selectbits(destColor.g, sourceColor.g, fragMask).v);
		_mm_store_ps(reinterpret_cast<float*>(&(cb_[offs_].b)), selectbits(destColor.b, sourceColor.b, fragMask).v); }

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

		VertexOutput vertexInterpolants{
			Interpolate(bary, vo0_),
			Interpolate(bary, vo1_),
			Interpolate(bary, vo2_),
			Interpolate(bary, vo3_) };

		qfloat4 fragColor;
		FRAGMENT_PROGRAM::ShadeFragment(fragCoord,
		                                fragDepth,
										uniforms_,
		                                vertexInterpolants,
		                                bary, tu0_, tu1_, fragColor);

		qfloat4 destColor;
		LoadColor(destColor);

		// qfloat4 blendedColor = fragColor; // no blending

		StoreColor(destColor, fragColor, fragMask);
		StoreDepth(destDepth, fragDepth, fragMask); } };


struct TileStat {
	int tileId;
	int cost; };


struct Tile {
	Tile(const int id, const rmlg::irect& rect) :
		id(id),
		rect(rect) {}

	void Reset() {
		commands0.reset(); }

	const int id;
	const rmlg::irect rect;
	int threadId;
	FastPackedStream commands0;
	char padding[64 - sizeof(FastPackedStream)];  // prevent false-sharing
	FastPackedStream commands1; };


/*
 * use the command list length as an approximate compute cost
 */
inline int RenderCost(const Tile& tile) {
	return int(tile.commands1.size()); }


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
	void Reset(rmlv::ivec2 newBufferDimensionsInPixels, rmlv::ivec2 newTileDimensionsInBlocks) {
		SetSize(newBufferDimensionsInPixels, newTileDimensionsInBlocks);
		for (auto& tile : tiles_) {
			tile.Reset(); }
		clippedVertexBuffer0_.clear();
		stats0_ = GPUStats{};
		IC().reset(); }

	GL& IC() {
		return (userIC_ == 0 ? IC0_ : IC1_); }

	void SetSize(rmlv::ivec2 newBufferDimensionsInPixels, rmlv::ivec2 newTileDimensionsInBlocks) {
		if (newBufferDimensionsInPixels != bufferDimensionsInPixels_ ||
		    newTileDimensionsInBlocks != tileDimensionsInBlocks_) {
			bufferDimensionsInPixels_ = newBufferDimensionsInPixels;
			tileDimensionsInBlocks_ = newTileDimensionsInBlocks;
			Retile(); }}

private:
	void Retile() {
		using rmlv::ivec2;
		using std::min, std::max;
		tileDimensionsInPixels_ = tileDimensionsInBlocks_ * blockDimensionsInPixels;
		bufferDimensionsInTiles_ = (bufferDimensionsInPixels_ + (tileDimensionsInPixels_ - ivec2{1, 1})) / tileDimensionsInPixels_;

		tiles_.clear();
		int seq = 0;
		for (int ty=0; ty<bufferDimensionsInTiles_.y; ++ty) {
			for (int tx=0; tx<bufferDimensionsInTiles_.x; ++tx) {
				auto left = tx * tileDimensionsInPixels_.x;
				auto top = ty * tileDimensionsInPixels_.y;
				auto right = min(left + tileDimensionsInPixels_.x, bufferDimensionsInPixels_.x);
				auto bottom = min(top + tileDimensionsInPixels_.y, bufferDimensionsInPixels_.y);
				rmlg::irect bbox{ {left, top}, {right, bottom} };
				tiles_.push_back(Tile{seq++, bbox}); }}}

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

		if (!doubleBuffer_) {
			BinImpl();
			SwapBuffers(); }

		// sort tiles by their estimated cost before queueing
		tileStats_.clear();
		for (int bi = 0; bi < tiles_.size(); ++bi) {
			tileStats_.push_back({ bi, RenderCost(tiles_[bi]) }); }
		rclr::sort(tileStats_, [](auto a, auto b) { return a.cost > b.cost; });
		//std::rotate(tileStats_.begin(), tileStats_.begin() + 1, tileStats_.end())

		std::vector<rclmt::jobsys::Job*> allJobs;
		for (const auto& item : tileStats_) {
			int bi = item.tileId;
			allJobs.push_back(Draw(finalizeJob, bi)); }
		if (doubleBuffer_) {
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
		if (doubleBuffer_) {
			SwapBuffers(); } }

	void SwapBuffers() {
		// double-buffer swap
		for (auto& tile : tiles_) {
			std::swap(tile.commands0, tile.commands1); }
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

		GLState* stateptr = nullptr;
		auto& cs = IC().d_commands;
		int totalCommands = 0;
		while (!cs.eof()) {
			totalCommands += 1;
			auto cmd = cs.consumeByte();
			// printf("cmd %02x %d:", cmd, cmd);
			if (cmd == CMD_STATE) {
				stateptr = static_cast<GLState*>(cs.consumePtr());
				// printf(" state is now at %p\n", stateptr);
				for (auto& tile : tiles_) {
					tile.commands0.appendByte(cmd);
					tile.commands0.appendPtr(stateptr); }}
			else if (cmd == CMD_CLEAR) {
				auto arg = cs.consumeByte();
				// printf(" clear with color ");
				// std::cout << color << std::endl;
				for (auto& tile : tiles_) {
					tile.commands0.appendByte(cmd);
					tile.commands0.appendByte(arg); }}
			else if (cmd == CMD_STORE_FP32_HALF) {
				auto ptr = cs.consumePtr();
				// printf(" store halfsize colorbuffer @ %p\n", ptr);
				for (auto& tile : tiles_) {
					tile.commands0.appendByte(cmd);
					tile.commands0.appendPtr(ptr); }}
			else if (cmd == CMD_STORE_FP32) {
				auto ptr = cs.consumePtr();
				// printf(" store unswizzled colorbuffer @ %p\n", ptr);
				for (auto& tile : tiles_) {
					tile.commands0.appendByte(cmd);
					tile.commands0.appendPtr(ptr); }}
			else if (cmd == CMD_STORE_TRUECOLOR) {
				auto enableGamma = cs.consumeByte();
				auto ptr = cs.consumePtr();
				// printf(" store truecolor @ %p, gamma corrected? %s\n", ptr, (enableGamma?"Yes":"No"));
				for (auto& tile : tiles_) {
					tile.commands0.appendByte(cmd);
					tile.commands0.appendByte(enableGamma);
					tile.commands0.appendPtr(ptr); }}
			else if (cmd == CMD_DRAW_ARRAY) {
				//auto flags = cs.consumeByte();
				//assert(flags == 0x14);  // videocore: 16-bit indices, triangles
				auto enableClipping = bool(cs.consumeByte());
				auto count = cs.consumeInt();
				// printf(" drawElements %02x %d %p", flags, count, indices);
				if (enableClipping) {
					bin_DrawArray<true, SHADERS...>(*stateptr, count); }
				else {
					bin_DrawArray<false, SHADERS...>(*stateptr, count); }}
			else if (cmd == CMD_DRAW_ELEMENTS) {
				auto flags = cs.consumeByte();
				assert(flags == 0x14);  // videocore: 16-bit indices, triangles
				auto enableClipping = bool(cs.consumeByte());
				auto count = cs.consumeInt();
				auto indices = static_cast<uint16_t*>(cs.consumePtr());
				// printf(" drawElements %02x %d %p", flags, count, indices);
				if (enableClipping) {
					bin_DrawElements<true, SHADERS...>(*stateptr, count, indices); }
				else {
					bin_DrawElements<false, SHADERS...>(*stateptr, count, indices); }}}

		// stats...
		int total_ = 0;
		int min_ = 0x7fffffff;
		int max_ = 0;
		for (auto& tile : tiles_) {
			const auto thisTileBytes = tile.commands0.size();
			total_ += thisTileBytes;
			min_ = std::min(min_, thisTileBytes);
			max_ = std::max(max_, thisTileBytes); }
		stats0_.totalTiles = tiles_.size();
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
		const auto& rect = tiles_[tileIdx].rect;
		auto& cs = tiles_[tileIdx].commands1;
		tiles_[tileIdx].threadId = tid;

		auto& dc = *depthCanvasPtr_;
		auto& cc = *colorCanvasPtr_;

		const GLState* stateptr = nullptr;
		while (!cs.eof()) {
			auto cmd = cs.consumeByte();
			// fmt::printf("tile(%d) cmd(%02x, %d)\n", tileIdx, cmd, cmd);
			if (cmd == CMD_STATE) {
				stateptr = static_cast<const GLState*>(cs.consumePtr()); }
			else if (cmd == CMD_CLEAR) {
				int bits = cs.consumeByte();
				if ((bits & GL_COLOR_BUFFER_BIT) != 0) {
					// std::cout << "clearing to " << color << std::endl;
					fillRect(rect, stateptr->clearColor, cc); }
				if ((bits & GL_DEPTH_BUFFER_BIT) != 0) {
					fillRect(rect, stateptr->clearDepth, dc); }
				if ((bits & GL_STENCIL_BUFFER_BIT) != 0) {
					// XXX not implemented
					assert(false); }}
			else if (cmd == CMD_STORE_FP32_HALF) {
				auto smallcanvas = static_cast<rglr::FloatingPointCanvas*>(cs.consumePtr());
				downsampleRect(rect, cc, *smallcanvas); }
			else if (cmd == CMD_STORE_FP32) {
				auto smallcanvas = static_cast<rglr::FloatingPointCanvas*>(cs.consumePtr());
				copyRect(rect, cc, *smallcanvas); }
			else if (cmd == CMD_STORE_TRUECOLOR) {
				auto enableGamma = cs.consumeByte();
				auto& outcanvas = *static_cast<rglr::TrueColorCanvas*>(cs.consumePtr());
				tile_StoreTrueColor<SHADERS...>(*stateptr, rect, enableGamma, outcanvas);
				// XXX draw cpu assignment indicators draw_border(rect, cpu_colors[tid], canvas);
				}
			else if (cmd == CMD_CLIPPED_TRI) {
				tile_DrawClipped<SHADERS...>(*stateptr, rect, cs); }
			else if (cmd == CMD_DRAW_INLINE) {
				tile_DrawElements<SHADERS...>(*stateptr, rect, cs); }}}

	template <typename ...PGMs>
	typename std::enable_if<sizeof...(PGMs) == 0>::type tile_StoreTrueColor(const GLState& state, const rmlg::irect rect, const bool enableGamma, rglr::TrueColorCanvas& outcanvas) {}

	template <typename PGM, typename ...PGMs>
	void tile_StoreTrueColor(const GLState& state, const rmlg::irect rect, const bool enableGamma, rglr::TrueColorCanvas& outcanvas) {
		if (state.programId != PGM::id) {
			return tile_StoreTrueColor<PGMs...>(state, rect, enableGamma, outcanvas); }

		auto& cc = *colorCanvasPtr_;
		if (enableGamma) {
			rglr::copyRect<PGM, rglr::sRGB>(rect, cc, outcanvas); }
		else {
			rglr::copyRect<PGM, rglr::LinearColor>(rect, cc, outcanvas); }}

	template <bool ENABLE_CLIPPING, typename ...PGMs>
	typename std::enable_if<sizeof...(PGMs) == 0>::type bin_DrawArray(const GLState& state, const int count) {}

	template <bool ENABLE_CLIPPING, typename PGM, typename ...PGMs>
	void bin_DrawArray(const GLState& state, const int count) {
		if (state.programId != PGM::id) {
			return bin_DrawArray<ENABLE_CLIPPING, PGMs...>(state, count); }
		using std::min, std::max, std::swap;
		using rmlv::ivec2, rmlv::qfloat, rmlv::qfloat2, rmlv::qfloat3, rmlv::qfloat4, rmlm::qmat4;
		assert(state.arrayFormat == AF_VAO_F3F3F3);
		assert(state.array != nullptr);

		const auto& vao = *static_cast<const VertexArray_F3F3F3*>(state.array);

		const auto frustum = ViewFrustum{ bufferDimensionsInPixels_ };

		clipFlagBuffer_.clear();
		devCoordBuffer_.clear();
		clipQueue_.clear();

		for (auto& tile : tiles_) {
			tile.commands0.appendByte(CMD_DRAW_INLINE);
			tile.commands0.mark(); }

		VertexInput vi_;

		const ShaderUniforms ui = MakeUniforms(state);

		const qfloat2 deviceScale{ float(bufferDimensionsInPixels_.x/2),
		                          -float(bufferDimensionsInPixels_.y/2) };
		const qfloat2 deviceOffset{ float(bufferDimensionsInPixels_.x/2),
		                            float(bufferDimensionsInPixels_.y/2) };

		const auto siz = int(vao.size());
		// xxx const int rag = siz % 4;  assume vaos are always padded to size()%4=0
		int vi = 0;
		int ti = 0;

		auto processAsManyFacesAsPossible = [&]() {
			for (; ti < count; ti += 3) {
				int i0 = ti;
				int i1 = ti + 1;
				int i2 = ti + 2;
				if (i0 >= vi || i1 >= vi || i2 >= vi) {
					break; }

				stats0_.totalTrianglesSubmitted++;

				if (ENABLE_CLIPPING) {
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
						clipQueue_.push_back({ i0, i1, i2 });
						continue; }}

				auto devCoord0 = devCoordBuffer_[i0];
				auto devCoord1 = devCoordBuffer_[i1];
				auto devCoord2 = devCoordBuffer_[i2];

				// handle backfacing tris and culling
				const bool backfacing = rmlg::triangle2Area(devCoord0, devCoord1, devCoord2) < 0;
				if (backfacing) {
					if (state.cullingEnabled && state.cullFace == GL_BACK) {
						stats0_.totalTrianglesCulled++;
						continue; }
					// devCoord is _not_ swapped, but relies on the aabb method that ForEachCoveredBin uses!
					swap(i0, i2);
					i0 |= 0x8000; }  // add backfacing flag
				else {
					if (state.cullingEnabled && state.cullFace == GL_FRONT) {
						stats0_.totalTrianglesCulled++;
						continue; }}

				ForEachCoveredTile(devCoord0, devCoord1, devCoord2, [&i0, &i1, &i2](auto& tile) {
					tile.commands0.appendUShort(i0);  // also includes backfacing flag
					tile.commands0.appendUShort(i1);
					tile.commands0.appendUShort(i2); });}};

		for (; vi < siz && ti < count; vi += 4) {
			if ((vi & 0x1ff) == 0) {
				processAsManyFacesAsPossible(); }

			//----- begin vao specialization -----
			vi_.a0 = vao.a0.loadxyz1(vi);
			vi_.a1 = vao.a1.loadxyz0(vi);
			vi_.a2 = vao.a2.loadxyz0(vi);
			//------ end vao specialization ------

			qfloat4 coord;
			VertexOutput unused;
			PGM::ShadeVertex(vi_, ui, coord, unused);

			if (ENABLE_CLIPPING) {
				auto flags = frustum.Test(coord);
				store_bytes(clipFlagBuffer_.alloc<4>(), flags); }

			auto devCoord = pdiv(coord).xy() * deviceScale + deviceOffset;
			devCoord.copyTo(devCoordBuffer_.alloc<4>()); }

		processAsManyFacesAsPossible();

		for (auto & tile : tiles_) {
			if (tile.commands0.touched()) {
				tile.commands0.appendUShort(0xffff);}
			else {
				// remove the CMD_DRAW_INLINE from any tiles
				// that weren't covered by this draw
				tile.commands0.unappend(1); }}

		stats0_.totalTrianglesClipped = clipQueue_.size();
		if (ENABLE_CLIPPING && !clipQueue_.empty()) {
			bin_DrawElementsClipped<PGM>(state); }}

	template <bool ENABLE_CLIPPING, typename ...PGMs>
	typename std::enable_if<sizeof...(PGMs) == 0>::type bin_DrawElements(const GLState& state, const int count, const uint16_t * const indices) {}

	template <bool ENABLE_CLIPPING, typename PGM, typename ...PGMs>
	void bin_DrawElements(const GLState& state, const int count, const uint16_t * const indices) {
		if (PGM::id != state.programId) {
			return bin_DrawElements<ENABLE_CLIPPING, PGMs...>(state, count, indices); }
		using std::min, std::max, std::swap;
		using rmlv::ivec2, rmlv::qfloat, rmlv::qfloat2, rmlv::qfloat3, rmlv::qfloat4, rmlm::qmat4;
		assert(state.arrayFormat == AF_VAO_F3F3F3);
		assert(state.array != nullptr);

		const auto& vao = *static_cast<const VertexArray_F3F3F3*>(state.array);

		const auto frustum = ViewFrustum{ bufferDimensionsInPixels_.x };

		clipFlagBuffer_.clear();
		devCoordBuffer_.clear();
		clipQueue_.clear();

		for (auto& tile : tiles_) {
			tile.commands0.appendByte(CMD_DRAW_INLINE);
			tile.commands0.mark(); }

		VertexInput vi_;

		const ShaderUniforms ui = MakeUniforms(state);

		const qfloat2 deviceScale{ float(bufferDimensionsInPixels_.x/2),
		                          -float(bufferDimensionsInPixels_.y/2) };
		const qfloat2 deviceOffset{ float(bufferDimensionsInPixels_.x/2),
		                            float(bufferDimensionsInPixels_.y/2) };

		const auto siz = int(vao.size());
		// xxx const int rag = siz % 4;  assume vaos are always padded to size()%4=0
		int vi = 0;
		int ti = 0;

		auto processAsManyFacesAsPossible = [&]() {
			for (; ti < count; ti += 3) {
				uint16_t i0 = indices[ti + 0];
				uint16_t i1 = indices[ti + 1];
				uint16_t i2 = indices[ti + 2];
				if (i0 >= vi || i1 >= vi || i2 >= vi) {
					break; }

				stats0_.totalTrianglesSubmitted++;

				if (ENABLE_CLIPPING) {
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
						clipQueue_.push_back({ i0, i1, i2 });
						continue; }}

				auto devCoord0 = devCoordBuffer_[i0];
				auto devCoord1 = devCoordBuffer_[i1];
				auto devCoord2 = devCoordBuffer_[i2];

				// handle backfacing tris and culling
				const bool backfacing = rmlg::triangle2Area(devCoord0, devCoord1, devCoord2) < 0;
				if (backfacing) {
					if (state.cullingEnabled && state.cullFace == GL_BACK) {
						stats0_.totalTrianglesCulled++;
						continue; }
					// devCoord is _not_ swapped, but relies on the aabb method that ForEachCoveredBin uses!
					swap(i0, i2);
					i0 |= 0x8000; }  // add backfacing flag
				else {
					if (state.cullingEnabled && state.cullFace == GL_FRONT) {
						stats0_.totalTrianglesCulled++;
						continue; }}

				ForEachCoveredTile(devCoord0, devCoord1, devCoord2, [&i0, &i1, &i2](auto& tile) {
					tile.commands0.appendUShort(i0);  // also includes backfacing flag
					tile.commands0.appendUShort(i1);
					tile.commands0.appendUShort(i2); });}};

		for (; vi < siz && ti < count; vi += 4) {
			if ((vi & 0x1ff) == 0) {
				processAsManyFacesAsPossible(); }

			//----- begin vao specialization -----
			vi_.a0 = vao.a0.loadxyz1(vi);
			vi_.a1 = vao.a1.loadxyz0(vi);
			vi_.a2 = vao.a2.loadxyz0(vi);
			//------ end vao specialization ------

			qfloat4 coord;
			VertexOutput unused;
			PGM::ShadeVertex(vi_, ui, coord, unused);

			if (ENABLE_CLIPPING) {
				auto flags = frustum.Test(coord);
				store_bytes(clipFlagBuffer_.alloc<4>(), flags); }

			auto devCoord = pdiv(coord).xy() * deviceScale + deviceOffset;
			devCoord.copyTo(devCoordBuffer_.alloc<4>()); }

		processAsManyFacesAsPossible();

		for (auto & tile : tiles_) {
			if (tile.commands0.touched()) {
				tile.commands0.appendUShort(0xffff);}
			else {
				// remove the CMD_DRAW_INLINE from any tiles
				// that weren't covered by this draw
				tile.commands0.unappend(1); }}

		stats0_.totalTrianglesClipped = clipQueue_.size();
		if (ENABLE_CLIPPING && !clipQueue_.empty()) {
			bin_DrawElementsClipped<PGM>(state); }}

	template <typename ...PGMs>
	typename std::enable_if<sizeof...(PGMs) == 0>::type tile_DrawElements(const GLState& state, const rmlg::irect& rect, FastPackedStream& cs) {}

	template<typename PGM, typename ...PGMs>
	void tile_DrawElements(const GLState& state, const rmlg::irect& rect, FastPackedStream& cs) {
		if (state.programId != PGM::id) {
			return tile_DrawElements<PGMs...>(state, rect, cs); }

		using rmlm::mat4;
		using rmlv::vec2, rmlv::vec3, rmlv::vec4;
		using rmlv::mvec4i;
		using rmlv::qfloat, rmlv::qfloat2, rmlv::qfloat4;
		using std::array, std::swap;

		//---- begin vao specialization ----
		assert(state.array != nullptr);
		assert(state.arrayFormat == AF_VAO_F3F3F3);
		auto& vao = *static_cast<const VertexArray_F3F3F3*>(state.array);
		//----- end vao specialization -----

		auto& cbc = *colorCanvasPtr_;
		auto& dbc = *depthCanvasPtr_;
		const int target_height = cbc.height();

		const ShaderUniforms ui = MakeUniforms(state);

		const qfloat2 deviceScale{ float(bufferDimensionsInPixels_.x/2),
		                          -float(bufferDimensionsInPixels_.y/2) };
		const qfloat2 deviceOffset{ float(bufferDimensionsInPixels_.x/2),
		                            float(bufferDimensionsInPixels_.y/2) };

		using sampler = rglr::ts_pow2_mipmap;
		const sampler tu0(state.tus[0].ptr, state.tus[0].width, state.tus[0].height, state.tus[0].stride, state.tus[0].filter);
		const sampler tu1(state.tus[1].ptr, state.tus[1].width, state.tus[1].height, state.tus[1].stride, state.tus[1].filter);

		array<VertexInput, 3> vi_;
		array<VertexOutput, 3> computed;
		array<qfloat4, 3> devCoord;
		array<mvec4i, 3>  fx;
		array<mvec4i, 3>  fy;

		int li = 0;  // sse lane being loaded
		bool eof = false;
		bool backfacing;
		while (!eof) {

			const uint16_t i0 = cs.consumeUShort();
			if (i0 == 0xffff) {
				eof = true; }

			if (!eof) {
				// load vertex data from the VAO into the current SIMD lane
				const uint16_t i1 = cs.consumeUShort();
				const uint16_t i2 = cs.consumeUShort();
				backfacing = i0 & 0x8000;
				array<uint16_t, 3> faceIndices = {uint16_t(i0 & 0x7fff), i1, i2};

				for (int i=0; i<3; ++i) {
					const auto idx = faceIndices[i];
					//---- vao specialization ----
					vi_[i].a0.setLane(li, vec4{ vao.a0.at(idx), 1 });
					vi_[i].a1.setLane(li, vec4{ vao.a1.at(idx), 0 });
					vi_[i].a2.setLane(li, vec4{ vao.a2.at(idx), 0 }); }
				li += 1; }

			if (!eof && (li < 4)) {
				// keep loading if not eof and there are lanes remaining
				continue; }

			// shade up to 4x3 verts
			for (int i=0; i<3; ++i) {
				qfloat4 gl_Position;
				PGM::ShadeVertex(vi_[i], ui, gl_Position, computed[i]);
				devCoord[i] = pdiv(gl_Position);
				fx[i] = ftoi(16.0F * (devCoord[i].x * deviceScale.x + deviceOffset.x));
				fy[i] = ftoi(16.0F * (devCoord[i].y * deviceScale.y + deviceOffset.y)); }

			// draw up to 4 triangles
			for (int ti=0; ti<li; ti++) {
				DefaultTargetProgram<sampler, PGM, rglr::BlendProgram::Set> targetProgram{
					tu0, tu1,
					cbc, dbc,
					ui,
					VertexFloat1{ devCoord[0].w.lane[ti], devCoord[1].w.lane[ti], devCoord[2].w.lane[ti] },
					VertexFloat1{ devCoord[0].z.lane[ti], devCoord[1].z.lane[ti], devCoord[2].z.lane[ti] },
					computed[0].lane(ti),
					computed[1].lane(ti),
					computed[2].lane(ti) };
				TriangleRasterizer tr(targetProgram, rect, target_height);
				tr.Draw(fx[0].si[ti], fx[1].si[ti], fx[2].si[ti],
				        fy[0].si[ti], fy[1].si[ti], fy[2].si[ti],
				        !backfacing); }

			// reset the SIMD lane counter
			li = 0; }}

	template <typename PGM>
	void bin_DrawElementsClipped(const GLState& state) {
		using rmlm::mat4;
		using rmlv::ivec2, rmlv::vec2, rmlv::vec3, rmlv::vec4, rmlv::qfloat4;
		using std::array, std::swap, std::min, std::max;

		const auto& vao = *static_cast<const VertexArray_F3F3F3*>(state.array);
		const auto frustum = ViewFrustum{ bufferDimensionsInPixels_.x };

		vec2 deviceScale{ float(bufferDimensionsInPixels_.x/2),
		                 -float(bufferDimensionsInPixels_.y/2) };
		vec2 deviceOffset{ float(bufferDimensionsInPixels_.x/2),
		                   float(bufferDimensionsInPixels_.y/2) };
		const ShaderUniforms ui = MakeUniforms(state);

		for (const auto& faceIndices : clipQueue_) {
			// phase 1: load _poly_ with the shaded vertex data
			clipA_.clear();
			for (int i=0; i<3; ++i) {
				const auto idx = faceIndices[i];

				//---- begin vao specialization ----
				VertexInput vi_;
				vi_.a0 = vec4{ vao.a0.at(idx), 1 };
				vi_.a1 = vec4{ vao.a1.at(idx), 0 };
				vi_.a2 = vec4{ vao.a2.at(idx), 0 };
				//----- end vao specialization -----

				qfloat4 coord;
				VertexOutput computed;
				PGM::ShadeVertex(vi_, ui, coord, computed);
				clipA_.push_back(ClippedVertex{ coord.lane(0), computed.lane(0) }); }

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
						auto newVertex = mix(here, next, d);
						clipB_.push_back(newVertex); }}

				swap(clipA_, clipB_);
				if (clipA_.size() == 0) {
					break; }
				assert(clipA_.size() >= 3); }

			if (clipA_.size() == 0) {
				continue; }

			for (auto& vertex : clipA_) {
				// convert clip-coord to device-coord
				vertex.coord = pdiv(vertex.coord);
				vertex.coord.x = vertex.coord.x * deviceScale.x + deviceOffset.x;
				vertex.coord.y = vertex.coord.y * deviceScale.y + deviceOffset.y; }
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
			for (int vi=1; vi<clipA_.size() - 1; ++vi) {
				int i0{0}, i1{vi}, i2{vi+1};
				ForEachCoveredTile(clipA_[i0].coord.xy(), clipA_[i1].coord.xy(), clipA_[i2].coord.xy(), [&](auto& tile) {
					if (clippedVertexBuffer0_.size() == bi) {
						std::copy(begin(clipA_), end(clipA_), back_inserter(clippedVertexBuffer0_)); }
					tile.commands0.appendByte(CMD_CLIPPED_TRI);
					tile.commands0.appendUShort((bi+i0) | (backfacing ? 0x8000 : 0));
					tile.commands0.appendUShort(bi + i1);
					tile.commands0.appendUShort(bi + i2); });}}}

	template <typename FUNC>
	void ForEachCoveredTile(const rmlv::vec2 dc0, const rmlv::vec2 dc1, const rmlv::vec2 dc2, FUNC func) {
		using std::min, std::max, rmlv::ivec2;
		const ivec2 idev0{ dc0 };
		const ivec2 idev1{ dc1 };
		const ivec2 idev2{ dc2 };

		const int vminx = max(min(idev0.x, min(idev1.x, idev2.x)), 0);
		const int vminy = max(min(idev0.y, min(idev1.y, idev2.y)), 0);
		const int vmaxx = min(max(idev0.x, max(idev1.x, idev2.x)), bufferDimensionsInPixels_.x-1);
		const int vmaxy = min(max(idev0.y, max(idev1.y, idev2.y)), bufferDimensionsInPixels_.y-1);

		auto topLeft = ivec2{ vminx, vminy } / tileDimensionsInPixels_;
		auto bottomRight = ivec2{ vmaxx, vmaxy } / tileDimensionsInPixels_;

		int ofs = topLeft.y * bufferDimensionsInTiles_.x + topLeft.x;
		for (int ty = topLeft.y; ty <= bottomRight.y; ++ty) {
			int ofsX = ofs;
			for (int tx = topLeft.x; tx <= bottomRight.x; ++tx) {
				auto& tile = tiles_[ofsX++];
				func(tile); }
			ofs += bufferDimensionsInTiles_.x; }};

	template <typename ...PGMs>
	typename std::enable_if<sizeof...(PGMs) == 0>::type tile_DrawClipped(const GLState& state, const rmlg::irect& rect, FastPackedStream& cs) {}

	template <typename PGM, typename ...PGMs>
	void tile_DrawClipped(const GLState& state, const rmlg::irect& rect, FastPackedStream& cs) {
		if (state.programId != PGM::id) {
			return tile_DrawClipped<PGMs...>(state, rect, cs); }
		using rmlm::mat4;
		using rmlv::vec2, rmlv::vec3, rmlv::vec4;

		auto& cbc = *colorCanvasPtr_;
		auto& dbc = *depthCanvasPtr_;
		const int target_height = cbc.height();

		const ShaderUniforms ui = MakeUniforms(state);

		const auto tmp = cs.consumeUShort();
		const bool backfacing = (tmp & 0x8000) != 0;
		const auto i0 = tmp & 0x7fff;
		const auto i1 = cs.consumeUShort();
		const auto i2 = cs.consumeUShort();

		vec4 dev0 = clippedVertexBuffer1_[i0].coord;
		vec4 dev1 = clippedVertexBuffer1_[i1].coord;
		vec4 dev2 = clippedVertexBuffer1_[i2].coord;
		VertexOutputx1 data0 = clippedVertexBuffer1_[i0].data;
		VertexOutputx1 data1 = clippedVertexBuffer1_[i1].data;
		VertexOutputx1 data2 = clippedVertexBuffer1_[i2].data;

		using sampler = rglr::ts_pow2_mipmap;
		const sampler tu0(state.tus[0].ptr, state.tus[0].width, state.tus[0].height, state.tus[0].stride, state.tus[0].filter);
		const sampler tu1(state.tus[1].ptr, state.tus[1].width, state.tus[1].height, state.tus[1].stride, state.tus[1].filter);

		DefaultTargetProgram<sampler, PGM, rglr::BlendProgram::Set> targetProgram{
			tu0, tu1,
			cbc, dbc,
			ui,
			VertexFloat1{ dev0.w, dev1.w, dev2.w },
			VertexFloat1{ dev0.z, dev1.z, dev2.z },
			data0,
			data1,
			data2 };
		TriangleRasterizer tr(targetProgram, rect, target_height);
		tr.Draw(int(dev0.x*16.0F), int(dev1.x*16.0F), int(dev2.x*16.0F),
		        int(dev0.y*16.0F), int(dev1.y*16.0F), int(dev2.y*16.0F),
		        !backfacing); }

	auto MakeUniforms(const GLState& state) {
		ShaderUniforms ui;
		ui.u0 = state.color;
		ui.u1 = rmlv::vec4{ state.normal, 0.0F };
		//ui.u1 =

		ui.mvm = state.modelViewMatrix;
		ui.pm = state.projectionMatrix;
		ui.nm = transpose(inverse(state.modelViewMatrix));
		ui.mvpm = state.projectionMatrix * state.modelViewMatrix;
		return ui; }

public:
	bool DoubleBuffer() const {
		return doubleBuffer_; }
	void DoubleBuffer(bool value) {
		doubleBuffer_ = value; }
	void ColorCanvas(rglr::QFloat4Canvas* ptr) {
		colorCanvasPtr_ = ptr; }
	void DepthCanvas(rglr::QFloatCanvas* ptr) {
		depthCanvasPtr_ = ptr; }

private:
	// configuration
	bool doubleBuffer_{true};
	rglr::QFloat4Canvas* colorCanvasPtr_{nullptr};
	rglr::QFloatCanvas* depthCanvasPtr_{nullptr};

	// tile/bin collection
	std::vector<Tile> tiles_;
	std::vector<TileStat> tileStats_;

	// render target parameters
	rmlv::ivec2 bufferDimensionsInPixels_;
	rmlv::ivec2 bufferDimensionsInTiles_;
	rmlv::ivec2 tileDimensionsInBlocks_;
	rmlv::ivec2 tileDimensionsInPixels_;

	// IC users can write to
	int userIC_{0};

	// double-buffered GL contexts: binning
	GL IC0_;
	rcls::vector<ClippedVertex> clippedVertexBuffer0_;
	GPUStats stats0_;

	// buffers used during binning and clipping
	SubStack<uint8_t, maxVAOSizeInVertices> clipFlagBuffer_;
	SubStack<rmlv::vec2, maxVAOSizeInVertices> devCoordBuffer_;
	rcls::vector<std::array<int, 3>> clipQueue_;
	rcls::vector<ClippedVertex> clipA_;
	rcls::vector<ClippedVertex> clipB_;

	// double-buffered GL contexts: drawing
	GL IC1_;
	rcls::vector<ClippedVertex> clippedVertexBuffer1_;
	GPUStats stats1_; };


}  // namespace rglv
}  // namespace rqdq
