#pragma once
#include <algorithm>
#include <array>
#include <cassert>
#include <iostream>
#include <numeric>
#include <optional>
#include <string>
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
#include "src/rml/rmlm/rmlm_soa.hxx"
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
	std::uint8_t data[64]; };


struct ThreadStat {
	std::vector<int> tiles;
	uint8_t padding[64 - sizeof(std::vector<int>)];

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
	int totalTrianglesSubmitted;
	int totalTrianglesCulled;
	int totalTrianglesClipped;
	int totalTrianglesDrawn; };


inline Matrices MakeMatrices(const GLState& s) {
	Matrices m;
	m.vm = rmlm::qmat4{ s.viewMatrix };
	m.pm = rmlm::qmat4{ s.projectionMatrix };
	m.nm = rmlm::qmat4{ transpose(inverse(s.viewMatrix)) };
	m.vpm = rmlm::qmat4{ s.projectionMatrix * s.viewMatrix };
	return m; }


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


struct QFloatRenderbufferCursor {

	using dataType = rmlv::qfloat;
	using elemType = rmlv::qfloat;
	static constexpr int strideInQuads = 64;
	static constexpr int tileAreaInQuads = 64*64;

	rmlv::qfloat* buf_;
	const rmlv::ivec2 origin_;

	int offs_, offsLeft_;

	QFloatRenderbufferCursor(void* buf, rmlv::ivec2 origin) :
		buf_(static_cast<elemType*>(buf)),
		origin_(origin) {}

	void Begin(int x, int y) {
		x -= origin_.x;
		y -= origin_.y;
		offs_ = offsLeft_ = (y/2)*strideInQuads + (x/2); }

	void CR() {
		offsLeft_ += strideInQuads;
		offs_ = offsLeft_; }

	void Right2() {
		offs_++; }

	void Load(dataType& data) {
		data = _mm_load_ps(reinterpret_cast<float*>(&(buf_[offs_]))); }

	void Store(dataType destDepth, dataType sourceDepth, rmlv::mvec4i fragMask) {
		auto addr = &(buf_[offs_]);  // alpha-channel
		auto result = selectbits(destDepth, sourceDepth, fragMask).v;
		_mm_store_ps(reinterpret_cast<float*>(addr), result); }};


/**
 * qfloat3 in a qfloat4 buffer's rgb channels
 */
struct QFloat3RenderbufferCursor {

	using dataType = rmlv::qfloat3;
	using elemType = rmlv::qfloat3;
	static constexpr int strideInQuads = 64;
	static constexpr int tileAreaInQuads = 64*64;

	rmlv::qfloat3* buf_;
	const rmlv::ivec2 origin_;

	int offs_, offsLeft_;

	QFloat3RenderbufferCursor(void* buf, rmlv::ivec2 origin) :
		buf_(static_cast<elemType*>(buf)),
		origin_(origin) {}

	void Begin(int x, int y) {
		x -= origin_.x;
		y -= origin_.y;
		offs_ = offsLeft_ = (y/2)*strideInQuads + (x/2); }

	void CR() {
		offsLeft_ += strideInQuads;
		offs_ = offsLeft_; }

	void Right2() {
		offs_++; }

	void Load(dataType& data) {
		rglr::QFloat3Canvas::Load(buf_+offs_, data.x.v, data.y.v, data.z.v); }

	void Store(dataType destColor, dataType sourceColor, rmlv::mvec4i fragMask) {
		auto sr = selectbits(destColor.r, sourceColor.r, fragMask).v;
		auto sg = selectbits(destColor.g, sourceColor.g, fragMask).v;
		auto sb = selectbits(destColor.b, sourceColor.b, fragMask).v;
		rglr::QFloat3Canvas::Store(sr, sg, sb, buf_+offs_); } };

/**
 * qfloat3 in a qfloat4 buffer's rgb channels
 */
struct QFloat4RGBRenderbufferCursor {

	using dataType = rmlv::qfloat3;
	using elemType = rmlv::qfloat4;
	static constexpr int strideInQuads = 64;
	static constexpr int tileAreaInQuads = 64*64;

	rmlv::qfloat4* buf_;
	const rmlv::ivec2 origin_;

	int offs_, offsLeft_;

	QFloat4RGBRenderbufferCursor(void* buf, rmlv::ivec2 origin) :
		buf_(static_cast<elemType*>(buf)),
		origin_(origin) {}

	void Begin(int x, int y) {
		x -= origin_.x;
		y -= origin_.y;
		offs_ = offsLeft_ = (y/2)*strideInQuads + (x/2); }

	void CR() {
		offsLeft_ += strideInQuads;
		offs_ = offsLeft_; }

	void Right2() {
		offs_++; }

	void Load(dataType& data) {
		rglr::QFloat4Canvas::Load(buf_+offs_, data.x.v, data.y.v, data.z.v); }

	void Store(dataType destColor, dataType sourceColor, rmlv::mvec4i fragMask) {
		auto sr = selectbits(destColor.r, sourceColor.r, fragMask).v;
		auto sg = selectbits(destColor.g, sourceColor.g, fragMask).v;
		auto sb = selectbits(destColor.b, sourceColor.b, fragMask).v;
		rglr::QFloat4Canvas::Store(sr, sg, sb, buf_+offs_); } };


/**
 * qfloat in a qfloat4 buffer's alpha channel
 */
struct QFloat4ARenderbufferCursor {

	using dataType = rmlv::qfloat;
	using elemType = rmlv::qfloat4;
	static constexpr int strideInQuads = 64;
	static constexpr int tileAreaInQuads = 64*64;

	rmlv::qfloat4* buf_;
	const rmlv::ivec2 origin_;

	int offs_, offsLeft_;

	QFloat4ARenderbufferCursor(void* base, rmlv::ivec2 origin) :
		buf_(static_cast<elemType*>(base)),
		origin_(origin) {}

	void Begin(int x, int y) {
		x -= origin_.x;
		y -= origin_.y;
		offs_ = offsLeft_ = (y/2)*strideInQuads + (x/2); }

	void CR() {
		offsLeft_ += strideInQuads;
		offs_ = offsLeft_; }

	void Right2() {
		offs_++; }

	void Load(dataType& data) {
		data = _mm_load_ps(reinterpret_cast<float*>(&(buf_[offs_].a))); }

	void Store(dataType destDepth, dataType sourceDepth, rmlv::mvec4i fragMask) {
		auto addr = &(buf_[offs_].a);  // alpha-channel
		auto result = selectbits(destDepth, sourceDepth, fragMask).v;
		_mm_store_ps(reinterpret_cast<float*>(addr), result); }};


struct QFloatRenderBufferCursor {

	using dataType = rmlv::qfloat;
	using elemType = rmlv::qfloat;

	static constexpr int strideInQuads = 64;
	static constexpr int tileAreaInQuads = 64*64;

	rmlv::qfloat* buf_;
	const rmlv::ivec2 origin_;

	int offs_, offsLeft_;

	QFloatRenderBufferCursor(void* buf, rmlv::ivec2 origin) :
		buf_(static_cast<elemType*>(buf)),
		origin_(origin) {}

	void Begin(int x, int y) {
		x -= origin_.x;
		y -= origin_.y;
		offs_ = offsLeft_ = (y/2)*strideInQuads + (x/2); }

	void CR() {
		offsLeft_ += strideInQuads;
		offs_ = offsLeft_; }

	void Right2() {
		offs_++; }

	void Load(dataType& data) {
		data = _mm_load_ps(reinterpret_cast<float*>(&buf_[offs_])); }

	void Store(dataType destDepth, dataType sourceDepth, rmlv::mvec4i fragMask) {
		auto addr = &buf_[offs_];
		auto result = selectbits(destDepth, sourceDepth, fragMask).v;
		_mm_store_ps(reinterpret_cast<float*>(addr), result); }};


template <typename SHADER, typename COLOR_IO, typename DEPTH_IO, typename TU0, typename TU1, typename TU3, bool DEPTH_TEST, typename DEPTH_FUNC, bool DEPTH_WRITEMASK, bool COLOR_WRITEMASK, typename BLEND_FUNC>
struct TriangleProgram {
	COLOR_IO cc_;
	DEPTH_IO dc_;
	const Matrices matrices_;
	const rglv::VertexFloat1 invW_;
	const rglv::VertexFloat1 ndcZ_;
	const typename SHADER::UniformsMD uniforms_;
	const typename SHADER::Interpolants vo_;
	const TU0& tu0_;
	const TU1& tu1_;
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
		const TU0& tu0,
		const TU1& tu1,
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


struct TileStat {
	int tileId;
	int cost; };


inline void PrintStatistics(const GPUStats& stats) {
	fmt::printf("tiles: % 4d   commands: % 4d   states: % 4d\n", stats.totalTiles, stats.totalCommands, stats.totalStates);
	fmt::printf("command bytes: %d\n", stats.totalCommandBytes);
	fmt::printf("tile bytes total: %d, min: %d, max: %d\n", stats.totalTileCommandBytes, stats.minTileCommandBytes, stats.maxTileCommandBytes);
	fmt::printf("triangles submitted: %d, culled: %d, clipped: %d, drawn: %d\n", stats.totalTrianglesSubmitted, stats.totalTrianglesCulled, stats.totalTrianglesClipped, stats.totalTrianglesDrawn); }

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
		int dist = h - tilesHead_.data();
		tilesMark_[dist] = *h; }

	auto Touched(uint8_t** h) const -> bool {
		int dist = h - tilesHead_.data();
		return tilesMark_[dist] != *h; }

	/* XXX bool Eof(int t) const {
		return tilesTail_[t*16] == tilesHead_[t]; }*/

	/*
	 * use the command list length as an approximate compute cost
	 */
	auto RenderCost(int t) const -> int {
		auto begin = WriteRange(t).first;
		return tilesHead_[t] - begin; }

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
		auto DS = rmlv::qfloat2( vsz.x/2, -vsz.y/2 );
		auto DO = rmlv::qfloat2( vpx.x, bufferDimensionsInPixels_.y-vpx.y );
		return { DS, DO }; }

	auto ScissorRect(const GLState& s) const -> rmlg::irect {
		if (!s.scissorEnabled) {
			return { { 0, 0 }, bufferDimensionsInPixels_ }; }
		else {
			return gl_offset_and_size_to_irect(s.scissorOrigin, s.scissorSize); }}};





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
		int siz = (loader.Size() + 3) & (~0x3);
		clipFlagBuffer_.reserve(siz);
		devCoordXBuffer_.reserve(siz);
		devCoordYBuffer_.reserve(siz);

		const auto scissorRect = ScissorRect(state);

		const auto [DS, DO] = DSDO(state);
		const auto one = rmlv::mvec4f{ 1.0F };
		const auto _1 = rmlv::qfloat2{ one, one };

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

			for (int vi=0; vi<siz; vi+=4) {
				loader.LoadMD(vi, vertex);

				qfloat4 coord;
				typename SHADER::VertexOutputMD unused;
				SHADER::ShadeVertex(matrices, uniforms, vertex, coord, unused);

				auto flags = frustum.Test(coord);
				store_bytes(clipFlagBuffer_.data() + vi, flags);

				auto devCoord = (pdiv(coord).xy() + _1) * DS + DO;
				devCoord.template store<false>(devCoordXBuffer_.data() + vi,
				                               devCoordYBuffer_.data() + vi); }

			for (int ti=0; ti<count; ti+=3) {
				stats0_.totalTrianglesSubmitted++;

				uint16_t i0 = indexSource(ti);
				uint16_t i1 = indexSource(ti+1);
				uint16_t i2 = indexSource(ti+2);

				// check for triangles that need clipping
				const auto cf0 = clipFlagBuffer_.data()[i0];
				const auto cf1 = clipFlagBuffer_.data()[i1];
				const auto cf2 = clipFlagBuffer_.data()[i2];
				if (cf0 | cf1 | cf2) {
					if (cf0 & cf1 & cf2) {
						// all points outside of at least one plane
						stats0_.totalTrianglesCulled++;
						continue; }
					// queue for clipping
					clipQueue_.push_back({ iid, i0, i1, i2 });
					continue; }

				auto dc0 = rmlv::vec2{ devCoordXBuffer_.data()[i0], devCoordYBuffer_.data()[i0] };
				auto dc1 = rmlv::vec2{ devCoordXBuffer_.data()[i1], devCoordYBuffer_.data()[i1] };
				auto dc2 = rmlv::vec2{ devCoordXBuffer_.data()[i2], devCoordYBuffer_.data()[i2] };

				// handle backfacing tris and culling
				const bool backfacing = rmlg::triangle2Area(dc0, dc1, dc2) < 0;
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

				ForEachCoveredTile(scissorRect, dc0, dc1, dc2, [&](uint8_t** th) {
					if (INSTANCED) {
						AppendUShort(th, iid);}
					AppendUShort(th, i0);  // also includes backfacing flag
					AppendUShort(th, i1);
					AppendUShort(th, i2); }); }

			}  // instance loop

		for (auto& h : tilesHead_) {
			if (Touched(&h)) {
				AppendUShort(&h, 0xffff); }
			else {
				// remove the command from any tiles
				// that weren't covered by this draw
				Unappend(&h, 1); }}

		stats0_.totalTrianglesClipped = clipQueue_.size();
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
		const auto one = rmlv::mvec4f{ 1.0F };
		const auto _1 = rmlv::qfloat2{ one, one };

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
					devCoord[i] = (pdiv(gl_Position).xy() + _1) * DS + DO;
					/* todo compute 4x triangle area here */ }

				for (int ti{0}; ti<li; ++ti) {
					stats0_.totalTrianglesSubmitted++;

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
							stats0_.totalTrianglesCulled++;
							continue; }
						// queue for clipping
						clipQueue_.push_back({ iid, i0, i1, i2 });
						continue; }

					const bool backfacing = rmlg::triangle2Area(dc0, dc1, dc2) < 0;
					if (backfacing) {
						if (cullingEnabled && cullFace == GL_BACK) {
							stats0_.totalTrianglesCulled++;
							continue; }
						swap(i0, i2);
						i0 |= 0x8000; }  // add backfacing flag
					else {
						if (cullingEnabled && cullFace == GL_FRONT) {
							stats0_.totalTrianglesCulled++;
							continue; }}

					ForEachCoveredTile(scissorRect, dc0, dc1, dc2, [&](uint8_t** th) {
						if (INSTANCED) {
							AppendUShort(th, iid); }
						AppendUShort(th, i0);  // also includes backfacing flag
						AppendUShort(th, i1);
						AppendUShort(th, i2); }); }

				// reset the SIMD lane counter
				li = 0; };

			for (int ti=0; ti<count; ti+=3) {
				stats0_.totalTrianglesSubmitted++;

				uint16_t i0 = indexSource(ti);
				uint16_t i1 = indexSource(ti+1);
				uint16_t i2 = indexSource(ti+2);
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

		stats0_.totalTrianglesClipped = clipQueue_.size();
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
				vertex.coord.x = ((vertex.coord.x+1) * DS.x + DO.x).get_x();
				vertex.coord.y = ((vertex.coord.y+1) * DS.y + DO.y).get_x(); }
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
			const auto bi = static_cast<int>(clippedVertexBuffer0_.size());
			clippedVertexBuffer0_.insert(end(clippedVertexBuffer0_), begin(clipA_), end(clipA_));
			for (int vi=1; vi<int(clipA_.size()) - 1; ++vi) {
				int i0{0}, i1{vi}, i2{vi+1};
				ForEachCoveredTile(scissorRect, clipA_[i0].coord.xy(), clipA_[i1].coord.xy(), clipA_[i2].coord.xy(), [&](uint8_t** th) {
					AppendByte(th, CMD_CLIPPED_TRI);
					AppendUShort(th, (bi+i0) | (backfacing ? 0x8000 : 0));
					AppendUShort(th, bi+i1);
					AppendUShort(th, bi+i2); }); }}}

	template <typename FUNC>
	auto ForEachCoveredTile(const rmlg::irect rect, const rmlv::vec2 dc0, const rmlv::vec2 dc1, const rmlv::vec2 dc2, FUNC func) -> int {
		using std::min, std::max, rmlv::ivec2;
		const ivec2 idev0{ dc0 };
		const ivec2 idev1{ dc1 };
		const ivec2 idev2{ dc2 };

		const int vminx = max(rmlv::Min(idev0.x, idev1.x, idev2.x), rect.top_left.x);
		const int vminy = max(rmlv::Min(idev0.y, idev1.y, idev2.y), rect.top_left.y);
		const int vmaxx = min(rmlv::Max(idev0.x, idev1.x, idev2.x), rect.bottom_right.x-1);
		const int vmaxy = min(rmlv::Max(idev0.y, idev1.y, idev2.y), rect.bottom_right.y-1);

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
		const auto _1 = rmlv::mvec4f{ 1.0F };

		const auto matrices = MakeMatrices(state);
		const typename SHADER::UniformsMD uniforms(*static_cast<const typename SHADER::UniformsSD*>(uniformsPtr));

		using sampler = rglr::ts_pow2_mipmap;
		const sampler tu0(state.tus[0].ptr, state.tus[0].width, state.tus[0].height, state.tus[0].stride, state.tus[0].filter);
		const sampler tu1(state.tus[1].ptr, state.tus[1].width, state.tus[1].height, state.tus[1].stride, state.tus[1].filter);

		const rglr::DepthTextureUnit tu3(state.tu3ptr, state.tu3dim);

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
				fx[i] = ftoi(16.0F * ((devCoord[i].x+_1) * DS.x + DO.x));
				fy[i] = ftoi(16.0F * ((devCoord[i].y+_1) * DS.y + DO.y)); }

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

		using sampler = rglr::ts_pow2_mipmap;
		const sampler tu0(state.tus[0].ptr, state.tus[0].width, state.tus[0].height, state.tus[0].stride, state.tus[0].filter);
		const sampler tu1(state.tus[1].ptr, state.tus[1].width, state.tus[1].height, state.tus[1].stride, state.tus[1].filter);

		const rglr::DepthTextureUnit tu3(state.tu3ptr, state.tu3dim);

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
		rasterizer.Draw(int(dev0.x*16.0F), int(dev1.x*16.0F), int(dev2.x*16.0F),
		                int(dev0.y*16.0F), int(dev1.y*16.0F), int(dev2.y*16.0F),
		                !backfacing);}

public:
	static
	auto MakeFragmentProgramPtrs() -> rglv::FragmentProgramPtrs {
		rglv::FragmentProgramPtrs out;
		out.tile_DrawClipped = static_cast<decltype(out.tile_DrawClipped)>(&GPUTileImpl::tile_DrawClipped);
		out.tile_DrawElementsSingle = static_cast<decltype(out.tile_DrawElementsSingle)>(&GPUTileImpl::tile_DrawElementsSingle);
		out.tile_DrawElementsInstanced = static_cast<decltype(out.tile_DrawElementsInstanced)>(&GPUTileImpl::tile_DrawElementsInstanced);
		return out; }};


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
