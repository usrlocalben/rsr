#pragma once
#include <rclmt_jobsys.hxx>
#include <rclr_algorithm.hxx>
#include <rcls_aligned_containers.hxx>
#include <rglr_blend.hxx>
#include <rglr_canvas.hxx>
#include <rglr_canvas_util.hxx>
#include <rglr_texture.hxx>
#include <rglv_clipper.hxx>
#include <rglv_fragment.hxx>
#include <rglv_gl.hxx>
#include <rglv_gpu_protocol.hxx>
#include <rglv_math.hxx>
#include <rglv_packed_stream.hxx>
#include <rglv_triangle.hxx>
#include <rglv_vao.hxx>
#include <rmlg_irect.hxx>
#include <rmlg_triangle.hxx>
#include <rmlm_mat4.hxx>
#include <rmlv_soa.hxx>
#include <rmlv_vec.hxx>

#include <algorithm>
#include <array>
#include <cassert>
#include <iostream>
#include <numeric>
#include <optional>
#include <vector>
#include <fmt/printf.h>


namespace rqdq {
namespace {

constexpr auto blockDimensionsInPixels = rmlv::ivec2{8, 8};
constexpr auto maxVAOSizeInVertices = 500000L;

}  // close unnamed namespace
namespace rglv {


struct VertexInput {
	rmlv::qfloat4 a0;
	rmlv::qfloat4 a1;
	rmlv::qfloat4 a2;
	rmlv::qfloat4 a3; };


struct ShaderUniforms {
	rmlv::qfloat4 u0;
	rmlv::qfloat4 u1;
	rmlm::qmat4 mvm;
	rmlm::qmat4 pm;
	rmlm::qmat4 nm;
	rmlm::qmat4 mvpm; };


struct VertexOutputx1 {
	rmlv::vec3 r0;
	rmlv::vec3 r1;
	rmlv::vec3 r2;
	rmlv::vec3 r3; };

inline VertexOutputx1 mix(VertexOutputx1 a, VertexOutputx1 b, float t) {
	return { mix(a.r0, b.r0, t),
	         mix(a.r1, b.r1, t),
	         mix(a.r2, b.r2, t),
	         mix(a.r3, b.r3, t) }; }


struct VertexOutput {
	rmlv::qfloat3 r0;
	rmlv::qfloat3 r1;
	rmlv::qfloat3 r2;
	rmlv::qfloat3 r3;

	VertexOutputx1 lane(const int li) {
		return VertexOutputx1{
			r0.lane(li),
			r1.lane(li),
			r2.lane(li),
			r3.lane(li), }; } };


struct ClippedVertex {
	rmlv::vec4 coord;  // either clip-coord or device-coord
	VertexOutputx1 data; };


struct BaseProgram {
	static int id;

	inline static void shadeVertex(
		const VertexInput& v,
		const ShaderUniforms& u,
		rmlv::qfloat4& gl_Position,
		VertexOutput& outs
		) {
		gl_Position = rmlm::mul(u.mvpm, v.a0); }

	template <typename TEXTURE_UNIT>
	inline static void shadeFragment(
		// built-in
		const rmlv::qfloat2& gl_FragCoord, /* gl_FrontFacing, */ const rmlv::qfloat& gl_FragDepth,
		// uniforms
		const ShaderUniforms& u,
		// vertex shader output
		const VertexOutput& v,
		// special
		const rglv::tri_qfloat& BS, const rglv::tri_qfloat& BP,
		// texture units
		const TEXTURE_UNIT& tu0,
		const TEXTURE_UNIT& tu1,
		// outputs
		rmlv::qfloat4& gl_FragColor
		) {
		gl_FragColor = rmlv::mvec4f::one(); }

	inline static rmlv::qfloat3 shadeCanvas(
		rmlv::qfloat2 gl_FragCoord,
		rmlv::qfloat3 source
		) {
		return source; } };


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

	rmlv::qfloat4* cb;
	rmlv::qfloat4* cbx;
	rmlv::qfloat* db;
	rmlv::qfloat* dbx;

	const TEXTURE_UNIT& tu0, tu1;

	const int width;
	const int height;
	const rmlv::qfloat2 target_dimensions;
	int offs, offs_left_start, offs_inc;

	const rglv::tri_qfloat vertexInvW;
	const rglv::tri_qfloat vertexDepth;

	const rglv::tri_qfloat3 vo0;
	const rglv::tri_qfloat3 vo1;
	const rglv::tri_qfloat3 vo2;
	const rglv::tri_qfloat3 vo3;

	const ShaderUniforms uniforms;

	DefaultTargetProgram(
		const TEXTURE_UNIT& tu0,
		const TEXTURE_UNIT& tu1,
		rglr::QFloat4Canvas& cc,
		rglr::QFloatCanvas& dc,
		const ShaderUniforms uniforms,
		const rmlv::vec4 v1,
		const rmlv::vec4 v2,
		const rmlv::vec4 v3,
		const VertexOutputx1 computed0,
		const VertexOutputx1 computed1,
		const VertexOutputx1 computed2
		)
		:width(cc.width()),
		height(cc.height()),
		target_dimensions(rmlv::qfloat2{float(cc.width()), float(cc.height())}),
		tu0(tu0),
		tu1(tu1),
		cb(cc.data()),
		db(dc.data()),
		uniforms(uniforms),
		vertexInvW(rglv::tri_qfloat{v1.w, v2.w,v3.w}),
		vertexDepth(rglv::tri_qfloat{v1.z, v2.z, v3.z}),
		vo0({ computed0.r0, computed1.r0, computed2.r0 }),
		vo1({ computed0.r1, computed1.r1, computed2.r1 }),
		vo2({ computed0.r2, computed1.r2, computed2.r2 }),
		vo3({ computed0.r3, computed1.r3, computed2.r3 })
	{}

	inline void goto_xy(const int x, const int y) {
		offs = offs_left_start = (y >> 1) * (width >> 1) + (x >> 1); }

	inline void inc_y() {
		offs_left_start += width >> 1;
		offs = offs_left_start; }

	inline void inc_x() {
		offs++; }

	inline void loadDepth(rmlv::qfloat& destDepth) {
		// from depthbuffer
		// destDepth = _mm_load_ps(reinterpret_cast<float*>(&db[offs]));

		// from alpha
		destDepth = _mm_load_ps(reinterpret_cast<float*>(&(cb[offs].a))); }

	inline void storeDepth(rmlv::qfloat destDepth,
	                rmlv::qfloat sourceDepth,
	                rmlv::mvec4i fragMask) {
		// auto addr = &db[offs];   // depthbuffer
		auto addr = &(cb[offs].a);  // alpha-channel
		auto result = selectbits(destDepth, sourceDepth, fragMask).v;
		_mm_store_ps(reinterpret_cast<float*>(addr), result); }

	inline void loadColor(rmlv::qfloat4& destColor) {
		destColor.r = _mm_load_ps(reinterpret_cast<float*>(&(cb[offs].r)));
		destColor.g = _mm_load_ps(reinterpret_cast<float*>(&(cb[offs].g)));
		destColor.b = _mm_load_ps(reinterpret_cast<float*>(&(cb[offs].b))); }

	inline void storeColor(rmlv::qfloat4 destColor,
	                rmlv::qfloat4 sourceColor,
	                rmlv::mvec4i fragMask) {
		_mm_store_ps(reinterpret_cast<float*>(&(cb[offs].r)), selectbits(destColor.r, sourceColor.r, fragMask).v);
		_mm_store_ps(reinterpret_cast<float*>(&(cb[offs].g)), selectbits(destColor.g, sourceColor.g, fragMask).v);
		_mm_store_ps(reinterpret_cast<float*>(&(cb[offs].b)), selectbits(destColor.b, sourceColor.b, fragMask).v); }

	inline void render(const rmlv::qfloat2& fragCoord, const rmlv::mvec4i& triMask, const rglv::tri_qfloat& BS, const bool frontfacing) {
		using rmlv::qfloat, rmlv::qfloat4;

		const auto fragDepth = rglv::interpolate(BS, vertexDepth);

		// read depth buffer
		qfloat destDepth;
		loadDepth(destDepth);

		const auto depthMask = float2bits(cmple(fragDepth, destDepth));
		const auto fragMask = andnot(triMask, depthMask);

		if (movemask(bits2float(fragMask)) == 0) {
			return; }  // early out if whole quad fails depth test

		// restore perspective
		const auto fragW = rmlv::oneover(rglv::interpolate(BS, vertexInvW));
		rglv::tri_qfloat BP;
		BP.v0 = vertexInvW.v0 * BS.v0 * fragW;
		BP.v1 = vertexInvW.v1 * BS.v1 * fragW;
		BP.v2 = qfloat{ 1.0f } - (BP.v0 + BP.v1);

		VertexOutput interpolatedVertexData{
			interpolate(BP, vo0),
			interpolate(BP, vo1),
			interpolate(BP, vo2),
			interpolate(BP, vo3) };

		qfloat4 fragColor;
		FRAGMENT_PROGRAM::shadeFragment(fragCoord,
		                                fragDepth,
										uniforms,
		                                interpolatedVertexData,
		                                BS, BP, tu0, tu1, fragColor);

		qfloat4 destColor;
		loadColor(destColor);

		// qfloat4 blendedColor = fragColor; // no blending

		storeColor(destColor, fragColor, fragMask);
		storeDepth(destDepth, fragDepth, fragMask); } };


struct BinStat {
	int binId;
	int cost; };


struct BinStream {
	BinStream(const int id, const rmlg::irect& rect)
		:id(id), rect(rect) {}

	void reset() {
		d_binStream.reset(); }

	const int id;
	const rmlg::irect rect;
	int d_threadId;
	FastPackedStream d_binStream;
	char padding[64 - sizeof(FastPackedStream)];  // prevent false-sharing
	FastPackedStream d_renderStream; };


/*
 * use the command list length as an approximate compute cost
 */
inline int renderCost(const BinStream& bin) {
	return int(bin.d_renderStream.size()); }


inline void printStatistics(const GPUStats& stats) {
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

	T at(int idx) const {
		return buffer[idx]; }

	void clear() { sp = 0; } };


template<typename ...SHADERS>
class GPU {
public:
	void reset(const rmlv::ivec2& newBufferDimensionsInPixels, const rmlv::ivec2& newTileDimensionsInBlocks) {
		setSize(newBufferDimensionsInPixels, newTileDimensionsInBlocks);
		for (auto& tile : d_bins) {
			tile.reset(); }
		d_clippedVertexBuffer0.clear();
		d_stats0 = GPUStats{};
		IC().reset(); }

	GL& IC() {
		return (d_activeIC == 0 ? d_IC0 : d_IC1); }

	void setSize(const rmlv::ivec2& newBufferDimensionsInPixels, const rmlv::ivec2& newTileDimensionsInBlocks) {
		if (newBufferDimensionsInPixels != d_bufferDimensionsInPixels ||
		    newTileDimensionsInBlocks != d_tileDimensionsInBlocks) {

			d_deviceMatrix = rglv::make_device_matrix(newBufferDimensionsInPixels.x,
			                                          newBufferDimensionsInPixels.y);
			d_bufferDimensionsInPixels = newBufferDimensionsInPixels;
			d_tileDimensionsInBlocks = newTileDimensionsInBlocks;
			retile(); }}

private:
	void retile() {
		using rmlv::ivec2;
		using std::min, std::max;
		d_tileDimensionsInPixels = d_tileDimensionsInBlocks * blockDimensionsInPixels;

		d_bufferDimensionsInTiles = (d_bufferDimensionsInPixels + (d_tileDimensionsInPixels - ivec2{1, 1})) / d_tileDimensionsInPixels;

		d_bins.clear();
		int i = 0;
		for (int ty=0; ty<d_bufferDimensionsInTiles.y; ++ty) {
			for (int tx=0; tx<d_bufferDimensionsInTiles.x; ++tx) {

				auto left = tx * d_tileDimensionsInPixels.x;
				auto top = ty * d_tileDimensionsInPixels.y;
				auto right = min(left + d_tileDimensionsInPixels.x, d_bufferDimensionsInPixels.x);
				auto bottom = min(top + d_tileDimensionsInPixels.y, d_bufferDimensionsInPixels.y);
				rmlg::irect bbox{ {left, top}, {right, bottom} };
				d_bins.push_back(BinStream{i++, bbox}); }}}

public:
	auto render() {
		return rclmt::jobsys::make_job(GPU::renderJmp, std::tuple{this}); }
private:
	static void renderJmp(rclmt::jobsys::Job* job, unsigned tid, std::tuple<GPU*>* data) {
		auto&[self] = *data;
		self->renderImpl(job); }
	void renderImpl(rclmt::jobsys::Job* job) {
		namespace jobsys = rclmt::jobsys;
		auto finalizeJob = finalize();
		if (job != nullptr) {
			jobsys::move_links(job, finalizeJob); }

		if (!enableDoubleBuffering) {
			binImpl();
			swapBuffers(); }

		// sort tiles by their estimated cost before queueing
		d_binStat.clear();
		for (int bi = 0; bi < d_bins.size(); ++bi) {
			d_binStat.push_back({ bi, renderCost(d_bins[bi]) }); }
		rclr::sort(d_binStat, [](auto a, auto b) { return a.cost > b.cost; });
		//std::rotate(d_binStat.begin(), d_binStat.begin() + 1, d_binStat.end())

		std::vector<rclmt::jobsys::Job*> allJobs;
		for (const auto& item : d_binStat) {
			int bi = item.binId;
			allJobs.push_back(tile(finalizeJob, bi)); }
		if (enableDoubleBuffering) {
			allJobs.push_back(bin(finalizeJob)); }

		for (auto& job : allJobs) {
			jobsys::run(job); }
		jobsys::run(finalizeJob); }

	auto finalize() {
		return rclmt::jobsys::make_job(GPU::finalizeJmp, std::tuple{this}); }
	static void finalizeJmp(rclmt::jobsys::Job* job, unsigned tid, std::tuple<GPU*>* data) {
		auto&[self] = *data;
		self->finalizeImpl(); }
	void finalizeImpl() {
		//printStatistics(d_stats1);
		if (enableDoubleBuffering) {
			swapBuffers(); } }

	void swapBuffers() {
		// double-buffer swap
		for (auto& tile : d_bins) {
			std::swap(tile.d_binStream, tile.d_renderStream); }
		std::swap(d_clippedVertexBuffer0, d_clippedVertexBuffer1);
		std::swap(d_stats0, d_stats1);
		d_activeIC = (d_activeIC+1) & 0x01; }

	/*
	 * process the command stream to create the tile streams
	 */
	auto bin(rclmt::jobsys::Job *parent) {
		return rclmt::jobsys::make_job_as_child(parent, GPU::binJmp, std::tuple{this}); }
	static void binJmp(rclmt::jobsys::Job* job, unsigned tid, std::tuple<GPU*>* data) {
		auto&[self] = *data;
		self->binImpl(); }
	void binImpl() {
		using fmt::printf;
		// printf("----- BEGIN BINNING -----\n");

		GLState *stateptr = nullptr;
		auto& cs = IC().d_commands;
		int totalCommands = 0;
		while (!cs.eof()) {
			totalCommands += 1;
			auto cmd = cs.consumeByte();
			// printf("cmd %02x %d:", cmd, cmd);
			if (cmd == CMD_STATE) {
				stateptr = static_cast<GLState*>(cs.consumePtr());
				// printf(" state is now at %p\n", stateptr);
				for (auto& tile : d_bins) {
					tile.d_binStream.appendByte(cmd);
					tile.d_binStream.appendPtr(stateptr); }}
			else if (cmd == CMD_CLEAR) {
				auto color = cs.consumeVec4();
				// printf(" clear with color ");
				// std::cout << color << std::endl;
				for (auto& tile : d_bins) {
					tile.d_binStream.appendByte(cmd);
					tile.d_binStream.appendVec4(color); }}
			else if (cmd == CMD_STORE_FP32_HALF) {
				auto ptr = cs.consumePtr();
				// printf(" store halfsize colorbuffer @ %p\n", ptr);
				for (auto& tile : d_bins) {
					tile.d_binStream.appendByte(cmd);
					tile.d_binStream.appendPtr(ptr); }}
			else if (cmd == CMD_STORE_FP32) {
				auto ptr = cs.consumePtr();
				// printf(" store unswizzled colorbuffer @ %p\n", ptr);
				for (auto& tile : d_bins) {
					tile.d_binStream.appendByte(cmd);
					tile.d_binStream.appendPtr(ptr); }}
			else if (cmd == CMD_STORE_TRUECOLOR) {
				auto enableGamma = cs.consumeByte();
				auto ptr = cs.consumePtr();
				// printf(" store truecolor @ %p, gamma corrected? %s\n", ptr, (enableGamma?"Yes":"No"));
				for (auto& tile : d_bins) {
					tile.d_binStream.appendByte(cmd);
					tile.d_binStream.appendByte(enableGamma);
					tile.d_binStream.appendPtr(ptr); }}
			else if (cmd == CMD_DRAW_ARRAY) {
				//auto flags = cs.consumeByte();
				//assert(flags == 0x14);  // videocore: 16-bit indices, triangles
				auto enableClipping = bool(cs.consumeByte());
				auto count = cs.consumeInt();
				// printf(" drawElements %02x %d %p", flags, count, indices);
				if (enableClipping) {
					bin_drawArray<true, SHADERS...>(*stateptr, count); }
				else {
					bin_drawArray<false, SHADERS...>(*stateptr, count); }}
			else if (cmd == CMD_DRAW_ELEMENTS) {
				auto flags = cs.consumeByte();
				assert(flags == 0x14);  // videocore: 16-bit indices, triangles
				auto enableClipping = bool(cs.consumeByte());
				auto count = cs.consumeInt();
				auto indices = static_cast<uint16_t*>(cs.consumePtr());
				// printf(" drawElements %02x %d %p", flags, count, indices);
				if (enableClipping) {
					bin_drawElements<true, SHADERS...>(*stateptr, count, indices); }
				else {
					bin_drawElements<false, SHADERS...>(*stateptr, count, indices); }}}

		// stats...
		int total_ = 0;
		int min_ = 0x7fffffff;
		int max_ = 0;
		for (auto& tile : d_bins) {
			const auto thisTileBytes = tile.d_binStream.size();
			total_ += thisTileBytes;
			min_ = std::min(min_, thisTileBytes);
			max_ = std::max(max_, thisTileBytes); }
		d_stats0.totalTiles = d_bins.size();
		d_stats0.totalCommands = totalCommands;
		// d_stats0.totalStates = IC().d_si;
		d_stats0.totalCommandBytes = cs.size();
		d_stats0.totalTileCommandBytes = total_;
		d_stats0.minTileCommandBytes = min_;
		d_stats0.maxTileCommandBytes = max_; }

	auto tile(rclmt::jobsys::Job* parent, int binId) {
		return rclmt::jobsys::make_job_as_child(parent, GPU::tileJmp, std::tuple{this, binId}); }
	static void tileJmp(rclmt::jobsys::Job* job, unsigned tid, std::tuple<GPU*, int>* data) {
		auto&[self, bin_id] = *data;
		self->tileImpl(tid, bin_id); }
	void tileImpl(const unsigned tid, const int bin_idx) {
		const auto& rect = d_bins[bin_idx].rect;
		auto& cs = d_bins[bin_idx].d_renderStream;
		d_bins[bin_idx].d_threadId = tid;

		auto& cc = *d_cc;

		const GLState* stateptr = nullptr;
		while (!cs.eof()) {
			auto cmd = cs.consumeByte();
			// fmt::printf("tile(%d) cmd(%02x, %d)\n", bin_idx, cmd, cmd);
			if (cmd == CMD_STATE) {
				stateptr = static_cast<const GLState*>(cs.consumePtr()); }
			else if (cmd == CMD_CLEAR) {
				auto color = cs.consumeVec4();
				// std::cout << "clearing to " << color << std::endl;
				fillRect(rect, color, cc); }
			else if (cmd == CMD_STORE_FP32_HALF) {
				auto smallcanvas = static_cast<rglr::FloatingPointCanvas*>(cs.consumePtr());
				downsampleRect(rect, cc, *smallcanvas); }
			else if (cmd == CMD_STORE_FP32) {
				auto smallcanvas = static_cast<rglr::FloatingPointCanvas*>(cs.consumePtr());
				copyRect(rect, cc, *smallcanvas); }
			else if (cmd == CMD_STORE_TRUECOLOR) {
				auto enableGamma = cs.consumeByte();
				auto& outcanvas = *static_cast<rglr::TrueColorCanvas*>(cs.consumePtr());
				tile_storeTrueColor<SHADERS...>(*stateptr, rect, enableGamma, outcanvas);
				// XXX draw cpu assignment indicators draw_border(rect, cpu_colors[tid], canvas);
				}
			else if (cmd == CMD_CLIPPED_TRI) {
				tile_drawClipped(*stateptr, rect, cs); }
			else if (cmd == CMD_DRAW_INLINE) {
				tile_drawElements<SHADERS...>(*stateptr, rect, cs); }}}

	template <typename ...PGMs>
	typename std::enable_if<sizeof...(PGMs) == 0>::type tile_storeTrueColor(const GLState& state, const rmlg::irect rect, const bool enableGamma, rglr::TrueColorCanvas& outcanvas) {}

	template <typename PGM, typename ...PGMs>
	void tile_storeTrueColor(const GLState& state, const rmlg::irect rect, const bool enableGamma, rglr::TrueColorCanvas& outcanvas) {
		if (state.programId != PGM::id) {
			tile_storeTrueColor<PGMs...>(state, rect, enableGamma, outcanvas);
			return; }

		auto& cc = *d_cc;
		if (enableGamma) {
			rglr::copyRect<PGM, rglr::sRGB>(rect, cc, outcanvas); }
		else {
			rglr::copyRect<PGM, rglr::LinearColor>(rect, cc, outcanvas); }}

	template <bool ENABLE_CLIPPING, typename ...PGMs>
	typename std::enable_if<sizeof...(PGMs) == 0>::type bin_drawArray(const GLState& state, const int count) {}

	template <bool ENABLE_CLIPPING, typename PGM, typename ...PGMs>
	void bin_drawArray(const GLState& state, const int count) {
		if (state.programId != PGM::id) {
			bin_drawArray<ENABLE_CLIPPING, PGMs...>(state, count);
			return; }
		using std::min, std::max;
		using rmlv::ivec2, rmlv::qfloat, rmlv::qfloat2, rmlv::qfloat3, rmlv::qfloat4, rmlm::qmat4;
		assert(state.arrayFormat == AF_VAO_F3F3F3);
		assert(state.array != nullptr);

		const auto& vao = *static_cast<const VertexArray_F3F3F3*>(state.array);

		const auto clipper = Clipper{};

		d_clipFlagBuffer.clear();
		d_devCoordBuffer.clear();
		d_clipQueue.clear();

		for (auto& tile : d_bins) {
			tile.d_binStream.appendByte(CMD_DRAW_INLINE);
			tile.d_binStream.mark(); }

		VertexInput vi_;

		const ShaderUniforms ui = generateUniforms(state);

		const qmat4 qm_dm{ d_deviceMatrix };

		const auto siz = int(vao.size());
		const int rag = siz % 4;
		int vi = 0;
		int ti = 0;

		auto processAsManyFacesAsPossible = [&]() {
			for (; ti < count; ti += 3) {
				int i0 = ti;
				int i1 = ti + 1;
				int i2 = ti + 2;
				if (i0 >= vi || i1 >= vi || i2 >= vi) {
					break; }

				d_stats0.totalTrianglesSubmitted++;

				if (ENABLE_CLIPPING) {
					// check for triangles that need clipping
					const auto cf0 = d_clipFlagBuffer.at(i0);
					const auto cf1 = d_clipFlagBuffer.at(i1);
					const auto cf2 = d_clipFlagBuffer.at(i2);
					if (cf0 | cf1 | cf2) {
						if (cf0 & cf1 & cf2) {
							// all points outside of at least one plane
							d_stats0.totalTrianglesCulled++;
							continue; }
						// queue for clipping
						//d_clipQueue.push_back({ i0, i1, i2 });
						continue; }}

				auto devCoord0 = d_devCoordBuffer.at(i0);
				auto devCoord1 = d_devCoordBuffer.at(i1);
				auto devCoord2 = d_devCoordBuffer.at(i2);

				// handle backfacing tris and culling
				const bool backfacing = rmlg::triangle2Area(devCoord0, devCoord1, devCoord2) < 0;
				if (backfacing) {
					if (state.cullingEnabled && state.cullFace == GL_BACK) {
						d_stats0.totalTrianglesCulled++;
						continue; }
					// devCoord is _not_ swapped, but relies on the aabb method that forEachCoveredBin uses!
					std::swap(i0, i2);
					i0 |= 0x8000; }  // add backfacing flag
				else {
					if (state.cullingEnabled && state.cullFace == GL_FRONT) {
						d_stats0.totalTrianglesCulled++;
						continue; }}

				forEachCoveredBin(devCoord0, devCoord1, devCoord2, [&i0, &i1, &i2](auto& bin) {
					bin.d_binStream.appendUShort(i0);  // also includes backfacing flag
					bin.d_binStream.appendUShort(i1);
					bin.d_binStream.appendUShort(i2); });}};

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
			PGM::shadeVertex(vi_, ui, coord, unused);

			if (ENABLE_CLIPPING) {
				auto flags = clipper.clip_point(coord);
				store_bytes(d_clipFlagBuffer.alloc<4>(), flags); }

			auto devCoord = pdiv(devmatmul(qm_dm, coord)).xy();
			devCoord.copyTo(d_devCoordBuffer.alloc<4>()); }

		processAsManyFacesAsPossible();

		for (auto & tile : d_bins) {
			if (tile.d_binStream.touched()) {
				tile.d_binStream.appendUShort(0xffff);}
			else {
				// remove the CMD_DRAW_INLINE from any tiles
				// that weren't covered by this draw
				tile.d_binStream.unappend(1); }}

		d_stats0.totalTrianglesClipped = d_clipQueue.size();
		if (ENABLE_CLIPPING && d_clipQueue.size()) {
			bin_drawElementsClipped<PGM>(state); }}

	template <bool ENABLE_CLIPPING, typename ...PGMs>
	typename std::enable_if<sizeof...(PGMs) == 0>::type bin_drawElements(const GLState& state, const int count, const uint16_t * const indices) {}

	template <bool ENABLE_CLIPPING, typename PGM, typename ...PGMs>
	void bin_drawElements(const GLState& state, const int count, const uint16_t * const indices) {
		if (PGM::id != state.programId) {
			bin_drawElements<ENABLE_CLIPPING, PGMs...>(state, count, indices);
			return; }
		using std::min, std::max;
		using rmlv::ivec2, rmlv::qfloat, rmlv::qfloat2, rmlv::qfloat3, rmlv::qfloat4, rmlm::qmat4;
		assert(state.arrayFormat == AF_VAO_F3F3F3);
		assert(state.array != nullptr);

		const auto& vao = *static_cast<const VertexArray_F3F3F3*>(state.array);

		const auto clipper = Clipper{};

		d_clipFlagBuffer.clear();
		d_devCoordBuffer.clear();
		d_clipQueue.clear();

		for (auto& tile : d_bins) {
			tile.d_binStream.appendByte(CMD_DRAW_INLINE);
			tile.d_binStream.mark(); }

		VertexInput vi_;

		const ShaderUniforms ui = generateUniforms(state);

		const qmat4 qm_dm{ d_deviceMatrix };

		const auto siz = int(vao.size());
		const int rag = siz % 4;
		int vi = 0;
		int ti = 0;

		auto processAsManyFacesAsPossible = [&]() {
			for (; ti < count; ti += 3) {
				uint16_t i0 = indices[ti + 0];
				uint16_t i1 = indices[ti + 1];
				uint16_t i2 = indices[ti + 2];
				if (i0 >= vi || i1 >= vi || i2 >= vi) {
					break; }

				d_stats0.totalTrianglesSubmitted++;

				if (ENABLE_CLIPPING) {
					// check for triangles that need clipping
					const auto cf0 = d_clipFlagBuffer.at(i0);
					const auto cf1 = d_clipFlagBuffer.at(i1);
					const auto cf2 = d_clipFlagBuffer.at(i2);
					if (cf0 | cf1 | cf2) {
						if (cf0 & cf1 & cf2) {
							// all points outside of at least one plane
							d_stats0.totalTrianglesCulled++;
							continue; }
						// queue for clipping
						d_clipQueue.push_back({ i0, i1, i2 });
						continue; }}

				auto devCoord0 = d_devCoordBuffer.at(i0);
				auto devCoord1 = d_devCoordBuffer.at(i1);
				auto devCoord2 = d_devCoordBuffer.at(i2);

				// handle backfacing tris and culling
				const bool backfacing = rmlg::triangle2Area(devCoord0, devCoord1, devCoord2) < 0;
				if (backfacing) {
					if (state.cullingEnabled && state.cullFace == GL_BACK) {
						d_stats0.totalTrianglesCulled++;
						continue; }
					// devCoord is _not_ swapped, but relies on the aabb method that forEachCoveredBin uses!
					std::swap(i0, i2);
					i0 |= 0x8000; }  // add backfacing flag
				else {
					if (state.cullingEnabled && state.cullFace == GL_FRONT) {
						d_stats0.totalTrianglesCulled++;
						continue; }}

				forEachCoveredBin(devCoord0, devCoord1, devCoord2, [&i0, &i1, &i2](auto& bin) {
					bin.d_binStream.appendUShort(i0);  // also includes backfacing flag
					bin.d_binStream.appendUShort(i1);
					bin.d_binStream.appendUShort(i2); });}};

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
			PGM::shadeVertex(vi_, ui, coord, unused);

			if (ENABLE_CLIPPING) {
				auto flags = clipper.clip_point(coord);
				store_bytes(d_clipFlagBuffer.alloc<4>(), flags); }

			auto devCoord = pdiv(devmatmul(qm_dm, coord)).xy();
			devCoord.copyTo(d_devCoordBuffer.alloc<4>()); }

		processAsManyFacesAsPossible();

		for (auto & tile : d_bins) {
			if (tile.d_binStream.touched()) {
				tile.d_binStream.appendUShort(0xffff);}
			else {
				// remove the CMD_DRAW_INLINE from any tiles
				// that weren't covered by this draw
				tile.d_binStream.unappend(1); }}

		d_stats0.totalTrianglesClipped = d_clipQueue.size();
		if (ENABLE_CLIPPING && d_clipQueue.size()) {
			bin_drawElementsClipped<PGM>(state); }}

	template <typename ...PGMs>
	typename std::enable_if<sizeof...(PGMs) == 0>::type tile_drawElements(const GLState& state, const rmlg::irect& rect, FastPackedStream& cs) {}

	template<typename PGM, typename ...PGMs>
	void tile_drawElements(const GLState& state, const rmlg::irect& rect, FastPackedStream& cs) {
		if (state.programId != PGM::id) {
			tile_drawElements<PGMs...>(state, rect, cs);
			return; }

		using rmlm::mat4;
		using rmlv::vec2;
		using rmlv::vec3;
		using rmlv::vec4;
		using rmlv::mvec4i;
		using rmlv::qfloat;
		using rmlv::qfloat4;
		using std::array;
		using std::swap;

		//---- begin vao specialization ----
		assert(state.array != nullptr);
		assert(state.arrayFormat == AF_VAO_F3F3F3);
		auto& vao = *static_cast<const VertexArray_F3F3F3*>(state.array);
		//----- end vao specialization -----

		auto& cbc = *d_cc;
		auto& dbc = *d_dc;
		const int target_height = cbc.height();

		const ShaderUniforms ui = generateUniforms(state);

		const rmlm::qmat4 qm_dm{ d_deviceMatrix };

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
				PGM::shadeVertex(vi_[i], ui, gl_Position, computed[i]);
				devCoord[i] = pdiv(devmatmul(qm_dm, gl_Position));
				fx[i] = ftoi(qfloat{ 16.0f } * (devCoord[i].x - qfloat{ 0.5f }));
				fy[i] = ftoi(qfloat{ 16.0f } * (devCoord[i].y - qfloat{ 0.5f })); }

			// draw up to 4 triangles
			for (int ti=0; ti<li; ti++) {
				DefaultTargetProgram<sampler, PGM, rglr::BlendProgram::Set> target_program(tu0, tu1, cbc, dbc, ui, devCoord[0].lane(ti), devCoord[1].lane(ti), devCoord[2].lane(ti), computed[0].lane(ti), computed[1].lane(ti), computed[2].lane(ti));
				draw_triangle(target_height, rect, fx[0].si[ti], fx[1].si[ti], fx[2].si[ti], fy[0].si[ti], fy[1].si[ti], fy[2].si[ti], !backfacing, target_program); }

			// reset the SIMD lane counter
			li = 0; }}

	template <typename PGM>
	void bin_drawElementsClipped(const GLState& state) {
		using rmlm::mat4;
		using rmlv::ivec2, rmlv::vec2, rmlv::vec3, rmlv::vec4, rmlv::qfloat4;
		using std::array, std::swap, std::min, std::max;

		static rcls::vector<ClippedVertex> poly;
		static rcls::vector<ClippedVertex> tmp;

		const auto& vao = *static_cast<const VertexArray_F3F3F3*>(state.array);
		const auto clipper = Clipper{};

		const rmlm::qmat4 qm_dm{ d_deviceMatrix };

		const ShaderUniforms ui = generateUniforms(state);

		for (const auto& faceIndices : d_clipQueue) {
			// phase 1: load _poly_ with the shaded vertex data
			poly.clear();
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
				PGM::shadeVertex(vi_, ui, coord, computed);
				poly.push_back(ClippedVertex{ coord.lane(0), computed.lane(0) }); }

			// phase 2: sutherland-hodgman clipping
			for (const auto plane : rglv::clipping_panes) {

				bool weAreInside = clipper.is_inside(plane, poly[0].coord);

				tmp.clear();
				for (int this_vi = 0; this_vi < poly.size(); ++this_vi) {
					const auto next_vi = (this_vi + 1) % poly.size(); // wrap

					const auto& here = poly[this_vi];
					const auto& next = poly[next_vi];

					const bool nextIsInside = clipper.is_inside(plane, next.coord);

					if (weAreInside) {
						tmp.push_back(here); }

					if (weAreInside != nextIsInside) {
						weAreInside = !weAreInside;

						const float t = clipper.clip_line(plane, here.coord, next.coord);
						auto newCoord = mix(here.coord, next.coord, t);
						auto newData = mix(here.data, next.data, t);
						tmp.push_back({ newCoord, newData }); }}

				std::swap(poly, tmp);
				if (poly.size() == 0) {
					break; }
				assert(poly.size() >= 3); }

			if (poly.size() == 0) {
				continue; }

			for (auto& vertex : poly) {
				// convert clip-coord to device-coord
				vertex.coord = pdiv(devmatmul(d_deviceMatrix, vertex.coord)); }
			// end of phase 2: poly contains a clipped N-gon

			// phase 3: project, convert to triangles and add to bins
			int bi = d_clippedVertexBuffer0.size();
			for (int vi=1; vi<poly.size() - 1; ++vi) {
				int i0{0}, i1{vi}, i2{vi+1};
				const bool backfacing = rmlg::triangle2Area(poly[i0].coord, poly[i1].coord, poly[i2].coord) < 0;
				bool willDraw = false;
				if (backfacing) {
					if (state.cullingEnabled == false || ((state.cullFace & GL_BACK) == 0)) {
						willDraw = true;
						std::swap(i0, i1); }}
				else {
					if (state.cullingEnabled == false || ((state.cullFace & GL_FRONT) == 0)) {
						willDraw = true; }}

				if (!willDraw) {
					break; }  // if one chunk would be culled, they all will

				forEachCoveredBin(poly[i0].coord.xy(), poly[i1].coord.xy(), poly[i2].coord.xy(), [&](auto& bin) {
					if (d_clippedVertexBuffer0.size() == bi) {
						std::copy(poly.begin(), poly.end(), back_inserter(d_clippedVertexBuffer0)); }
					bin.d_binStream.appendByte(CMD_CLIPPED_TRI);
					bin.d_binStream.appendUShort(bi + i0);
					bin.d_binStream.appendUShort(bi + i1);
					bin.d_binStream.appendUShort(bi + i2); });}}}

	template <typename FUNC>
	void forEachCoveredBin(const rmlv::vec2 dc0, const rmlv::vec2 dc1, const rmlv::vec2 dc2, FUNC func) {
		using std::min, std::max, rmlv::ivec2;
		const ivec2 idev0{ dc0 };
		const ivec2 idev1{ dc1 };
		const ivec2 idev2{ dc2 };

		const int vminx = max(min(idev0.x, min(idev1.x, idev2.x)), 0);
		const int vminy = max(min(idev0.y, min(idev1.y, idev2.y)), 0);
		const int vmaxx = min(max(idev0.x, max(idev1.x, idev2.x)), d_bufferDimensionsInPixels.x);
		const int vmaxy = min(max(idev0.y, max(idev1.y, idev2.y)), d_bufferDimensionsInPixels.y);

		auto top_left = ivec2{ vminx, vminy } / d_tileDimensionsInPixels;
		auto bottom_right = ivec2{ vmaxx, vmaxy } / d_tileDimensionsInPixels;

		bottom_right = vmin(bottom_right, d_bufferDimensionsInTiles - ivec2{ 1, 1 });

		int ofs = top_left.y * d_bufferDimensionsInTiles.x + top_left.x;
		for (int ty = top_left.y; ty <= bottom_right.y; ++ty) {
			int rofs = ofs;
			for (int tx = top_left.x; tx <= bottom_right.x; ++tx) {
				auto& bin = d_bins[rofs++];
				func(bin); }
			ofs += d_bufferDimensionsInTiles.x; }};

	template <typename ...PGMs>
	typename std::enable_if<sizeof...(PGMs) == 0>::type tile_drawClipped(const GLState& state, const rmlg::irect& rect, FastPackedStream& cs) {}

	template <typename PGM, typename ...PGMs>
	void tile_drawClipped(const GLState& state, const rmlg::irect& rect, FastPackedStream& cs) {
		if (state.programId != PGM::id) {
			tile_drawClipped<PGMs...>(state, rect, cs);
			return; }
		using rmlm::mat4;
		using rmlv::vec2, rmlv::vec3, rmlv::vec4;

		auto& cbc = *d_cc;
		auto& dbc = *d_dc;
		const int target_height = cbc.height();

		const ShaderUniforms ui = generateUniforms(state);

		const auto i0 = cs.consumeUShort();
		const auto i1 = cs.consumeUShort();
		const auto i2 = cs.consumeUShort();

		std::array<vec4, 3> dev = {
			d_clippedVertexBuffer1[i0].coord,
			d_clippedVertexBuffer1[i1].coord,
			d_clippedVertexBuffer1[i2].coord };
		std::array<VertexOutputx1, 3> computed = {
			d_clippedVertexBuffer1[i0].data,
			d_clippedVertexBuffer1[i1].data,
			d_clippedVertexBuffer1[i2].data };

		const bool backfacing = rmlg::triangle2Area(dev[0], dev[1], dev[2]) < 0;

		if (backfacing) {
			std::swap(computed[0], computed[2]);
			std::swap(dev[0], dev[2]); }

		using sampler = rglr::ts_pow2_mipmap;
		const sampler tu0(state.tus[0].ptr, state.tus[0].width, state.tus[0].height, state.tus[0].stride, state.tus[0].filter);
		const sampler tu1(state.tus[1].ptr, state.tus[1].width, state.tus[1].height, state.tus[1].stride, state.tus[1].filter);

		DefaultTargetProgram<sampler, PGM, rglr::BlendProgram::Set> target_program(tu0, tu1, cbc, dbc, ui, dev[0], dev[1], dev[2], computed[0], computed[1], computed[2]);
		draw_triangle(target_height, rect, dev[0], dev[1], dev[2], !backfacing, target_program); }

	auto generateUniforms(const GLState& state) {
		ShaderUniforms ui;
		ui.u0 = state.color;
		ui.u1 = rmlv::vec4{ state.normal, 0.0f };
		//ui.u1 =

		ui.mvm = state.modelViewMatrix;
		ui.pm = state.projectionMatrix;
		ui.nm = transpose(inverse(state.modelViewMatrix));
		ui.mvpm = state.projectionMatrix * state.modelViewMatrix;
		return ui; }

public:
	rglr::QFloat4Canvas* d_cc;
	rglr::QFloatCanvas* d_dc;
	bool enableDoubleBuffering = true;

private:
	// double-buffered GL contexts
	GL d_IC0;
	GL d_IC1;
	int d_activeIC = 0;

	// tile/bin collection
	std::vector<BinStream> d_bins;
	std::vector<BinStat> d_binStat;

	// render target parameters
	rmlv::ivec2 d_bufferDimensionsInPixels;
	rmlv::ivec2 d_bufferDimensionsInTiles;
	rmlv::ivec2 d_tileDimensionsInBlocks;
	rmlv::ivec2 d_tileDimensionsInPixels;
	rmlm::mat4 d_deviceMatrix;

	// binning buffers and sizes
	SubStack<uint8_t, maxVAOSizeInVertices> d_clipFlagBuffer;
	SubStack<rmlv::vec2, maxVAOSizeInVertices> d_devCoordBuffer;

	// double-buffered storage for clipped vertices
	rcls::vector<std::array<int, 3>> d_clipQueue;
	rcls::vector<ClippedVertex> d_clippedVertexBuffer0;
	rcls::vector<ClippedVertex> d_clippedVertexBuffer1;

	// double-buffered statistics
	GPUStats d_stats0;
	GPUStats d_stats1; };

}  // close package namespace
}  // close enterprise namespace
