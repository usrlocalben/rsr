#pragma once

#include <tuple>
#include <algorithm>
#include <array>
#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

#include "src/rcl/rclmt/rclmt_jobsys.hxx"
#include "src/rcl/rclr/rclr_algorithm.hxx"
#include "src/rcl/rcls/rcls_aligned_containers.hxx"
#include "src/rgl/rglr/rglr_algorithm.hxx"
#include "src/rgl/rglr/rglr_canvas.hxx"
#include "src/rgl/rglv/rglv_gl.hxx"
#include "src/rgl/rglv/rglv_gpu_protocol.hxx"
#include "src/rml/rmlg/rmlg_irect.hxx"
#include "src/rml/rmlv/rmlv_soa.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"

#include "3rdparty/fmt/include/fmt/printf.h"

namespace rqdq {
namespace rglv {

constexpr auto kBlockDimensionsInPixels = rmlv::ivec2{8, 8};

constexpr auto kMaxSizeInBytes = 100000L;

constexpr auto kTileColorSizeInBytes = 0x40000;  // 256KiB, up to 128x128 RGBA float32
constexpr auto kTileDepthSizeInBytes = 0x10000;  // 64KiB,  up to 128x128 1-ch float32


extern bool doubleBuffer;


struct ClippedVertex {
	rmlv::vec4 coord;  // either clip-coord or device-coord
	std::byte data[64]; };


struct ThreadStat {
	std::vector<int> tiles;
	std::byte padding[64 - sizeof(std::vector<int>)];

	void Reset() {
		tiles.clear(); } };


struct GPUStats {
	int totalTiles;
	int totalCommands;
	int totalStates;
	int totalCommandBytes;
	int totalTileCommandBytes;
	int minTileCommandBytes;
	int maxTileCommandBytes;
	int totalPrimitivesSubmitted;
	int totalPrimitivesCulled;
	int totalPrimitivesClipped;
	int totalPrimitivesDrawn; };


struct TileStat {
	int tileId;
	int cost; };


inline void PrintStatistics(const GPUStats& stats) {
	fmt::printf("tiles: % 4d   commands: % 4d   states: % 4d\n", stats.totalTiles, stats.totalCommands, stats.totalStates);
	fmt::printf("command bytes: %d\n", stats.totalCommandBytes);
	fmt::printf("tile bytes total: %d, min: %d, max: %d\n", stats.totalTileCommandBytes, stats.minTileCommandBytes, stats.maxTileCommandBytes);
	fmt::printf("primitives submitted: %d, culled: %d, clipped: %d, drawn: %d\n", stats.totalPrimitivesSubmitted, stats.totalPrimitivesCulled, stats.totalPrimitivesClipped, stats.totalPrimitivesDrawn); }


class GPU;


struct VertexProgramPtrs {
	void (GPU::*bin_DrawArraysSingle)(int /*count*/);
	void (GPU::*bin_DrawArraysInstanced)(int /*count*/, int /*instanceCnt*/);
	void (GPU::*bin_DrawElementsSingle)(int count, uint16_t* indices, uint8_t /*hint*/);
	void (GPU::*bin_DrawElementsInstanced)(int count, uint16_t* indices, int /*instanceCnt*/);
	// void (GPU::*tile_DrawClipped)(const GLState& state, const void* uniformsPtr, const rmlg::irect& rect, rmlv::ivec2 tileOrigin, int tileIdx, FastPackedReader& cs);
};


struct FragmentProgramPtrs {
	void (GPU::*tile_DrawClipped)(void*, void*, const GLState& state, const void* uniformsPtr, rmlg::irect rect, rmlv::ivec2 tileOrigin, int tileIdx, FastPackedReader& cs);
	void (GPU::*tile_DrawElementsSingle)(void*, void*, const GLState& state, const void* uniformsPtr, rmlg::irect rect, rmlv::ivec2 tileOrigin, int tileIdx, FastPackedReader& cs);
	void (GPU::*tile_DrawElementsInstanced)(void*, void*, const GLState& state, const void* uniformsPtr, rmlg::irect rect, rmlv::ivec2 tileOrigin, int tileIdx, FastPackedReader& cs);
	};


struct BltProgramPtrs {
	void (GPU::*tile_StoreTrueColor)(const GLState& state, const void* uniformsPtr, rmlg::irect rect, bool enableGamma, rglr::TrueColorCanvas& outcanvas);
	};


class GPU {
private:
	std::unordered_map<uint32_t, VertexProgramPtrs> vertexDispatch_{};
	std::unordered_map<uint32_t, FragmentProgramPtrs> fragmentDispatch_{};
	std::unordered_map<uint32_t, BltProgramPtrs> bltDispatch_{};

	const int concurrency_;
	const std::string guid_;

protected:
	rcls::vector<uint8_t> color0Buf_{};
	rcls::vector<uint8_t> depthBuf_{};

	std::vector<ThreadStat> threadStats_;

	// tile/bin collection
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

	// IC users can write to
	int userIC_{0};

	// double-buffered GL contexts: binning
	GL IC0_;
	rcls::vector<ClippedVertex> clippedVertexBuffer0_;
	GPUStats stats0_;

	// buffers used during binning and clipping
	const GLState* binState{nullptr};
	const void* binUniforms{nullptr};
	rcls::vector<uint8_t> clipFlagBuffer_{};
	rcls::vector<float> devCoordXBuffer_{};
	rcls::vector<float> devCoordYBuffer_{};
	rcls::vector<std::array<int, 4>> clipQueue_;
	rcls::vector<ClippedVertex> clipA_;
	rcls::vector<ClippedVertex> clipB_;

	// double-buffered GL contexts: drawing
	GL IC1_;
	rcls::vector<ClippedVertex> clippedVertexBuffer1_;
	GPUStats stats1_;

public:
	GPU(int concurrency, std::string guid="unnamed");
	void Install(int programId, uint32_t stateKey, VertexProgramPtrs ptrs);
	void Install(int programId, uint32_t stateKey, FragmentProgramPtrs ptrs);
	void Install(int programId, uint32_t stateKey, BltProgramPtrs ptrs);

	auto IC() -> GL&;
	void Reset(rmlv::ivec2 newBufferDimensionsInPixels, rmlv::ivec2 newTileDimensionsInBlocks);
	void SetSize(rmlv::ivec2 newBufferDimensionsInPixels, rmlv::ivec2 newTileDimensionsInBlocks);

protected:
	void Retile();
	auto TileRect(int idx) const -> rmlg::irect;

public:
	auto Run() {
		return rclmt::jobsys::make_job(GPU::RunJmp, std::tuple{this}); }
protected:
	static
	void RunJmp(rclmt::jobsys::Job* job, unsigned threadId [[maybe_unused]], std::tuple<GPU*>* data) {
		auto&[self] = *data;
		self->RunImpl(job); }
	void RunImpl(rclmt::jobsys::Job* job);

	auto Finalize() {
		return rclmt::jobsys::make_job(GPU::FinalizeJmp, std::tuple{this}); }
	static
	void FinalizeJmp(rclmt::jobsys::Job* job [[maybe_unused]], unsigned tid [[maybe_unused]], std::tuple<GPU*>* data) {
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
	static void BinJmp(rclmt::jobsys::Job* job [[maybe_unused]], unsigned tid [[maybe_unused]], std::tuple<GPU*>* data) {
		auto&[self] = *data;
		self->BinImpl(); }
	void BinImpl();

	auto Draw(rclmt::jobsys::Job* parent, int tileId) {
		return rclmt::jobsys::make_job_as_child(parent, GPU::DrawJmp, std::tuple{this, tileId}); }
	static void DrawJmp(rclmt::jobsys::Job* job [[maybe_unused]], unsigned tid, std::tuple<GPU*, int>* data) {
		auto&[self, tileId] = *data;
		self->DrawImpl(tid, tileId); }
	void DrawImpl(const unsigned tid, const int tileIdx);

	void AppendByte(uint8_t** h, uint8_t a) {
		*reinterpret_cast<uint8_t*>(*h) = a; *h += sizeof(uint8_t); }
	void AppendUShort(uint8_t** h, uint16_t a) {
		*reinterpret_cast<uint16_t*>(*h) = a; *h += sizeof(uint16_t); }
	void AppendInt(uint8_t** h, int a) {
		*reinterpret_cast<int*>(*h) = a; *h += sizeof(int); }
	void AppendFloat(uint8_t** h, float a) {
		*reinterpret_cast<float*>(*h) = a;  *h += sizeof(float); }
	void AppendPtr(uint8_t** h, const void* a) {
		*reinterpret_cast<const void**>(*h) = a;  *h += sizeof(void*); }
	void AppendVec4(uint8_t** h, rmlv::vec4 a) {
		AppendFloat(h, a.x);
		AppendFloat(h, a.y);
		AppendFloat(h, a.z);
		AppendFloat(h, a.w); }

	auto Unappend(uint8_t** h, int many) {
		*h -= many; }

	auto Mark(uint8_t** h) {
		auto dist = h - tilesHead_.data();
		tilesMark_[dist] = *h; }

	auto Touched(uint8_t** h) const -> bool {
		auto dist = h - tilesHead_.data();
		return tilesMark_[dist] != *h; }

	/* XXX bool Eof(int t) const {
		return tilesTail_[t*16] == tilesHead_[t]; }*/

	/*
	 * use the command list length as an approximate compute cost
	 */
	auto RenderCost(int t) const -> int {
		auto begin = WriteRange(t).first;
		return static_cast<int>(tilesHead_[t] - begin); }

	auto WriteRange(int t) const -> std::pair<uint8_t*, uint8_t*> {
		uint8_t* begin = const_cast<uint8_t*>(tilesMem0_.data()) + t*kMaxSizeInBytes;
		uint8_t* end = begin + kMaxSizeInBytes;
		return { begin, end }; }

	auto ReadRange(int t) const -> std::pair<uint8_t*, uint8_t*> {
		uint8_t* begin = const_cast<uint8_t*>(tilesMem1_.data()) + t*kMaxSizeInBytes;
		uint8_t* end = begin + kMaxSizeInBytes;
		return { begin, end }; }

	auto gl_offset_and_size_to_irect(rmlv::ivec2 origin, rmlv::ivec2 size) const -> rmlg::irect {
		int left = origin.x;
		int right = left + size.x;
		int bottom = bufferDimensionsInPixels_.y - origin.y - 1;
		int top = bottom - size.y;
		return { { left, top }, { right, bottom } }; }

	auto DSDO(const GLState& s) const -> std::pair<rmlv::qfloat2, rmlv::qfloat2> {
		auto vsz = s.viewportSize.value_or(bufferDimensionsInPixels_);
		auto vpx = s.viewportOrigin;
		auto DS = rmlv::qfloat2( static_cast<float>(vsz.x/2), static_cast<float>(-vsz.y/2) );
		auto DO = rmlv::qfloat2( static_cast<float>(vsz.x/2+vpx.x), static_cast<float>(bufferDimensionsInPixels_.y-(vsz.y/2+vpx.y)) );
		return { DS, DO }; }

	auto ScissorRect(const GLState& s) const -> rmlg::irect {
		if (!s.scissorEnabled) {
			return { { 0, 0 }, bufferDimensionsInPixels_ }; }
		else {
			return gl_offset_and_size_to_irect(s.scissorOrigin, s.scissorSize); }}};


//=============================================================================
//                            INLINE DEFINITIONS
//=============================================================================

							// ---------
							// class GPU
							// ---------
inline
auto GPU::IC() -> GL& {
	return (userIC_ == 0 ? IC0_ : IC1_); }


inline
auto GPU::TileRect(int idx) const -> rmlg::irect {
	auto ty = idx / bufferDimensionsInTiles_.x;
	auto tx = idx % bufferDimensionsInTiles_.x;
	auto tpos = rmlv::ivec2{ tx, ty };
	auto ltpx = tpos * tileDimensionsInPixels_;
	auto rbpx = ltpx + tileDimensionsInPixels_;
	rbpx = vmin(rbpx, bufferDimensionsInPixels_);  // clip against screen edge
	return { ltpx, rbpx }; }


}  // namespace rglv
}  // namespace rqdq
