#include "src/rgl/rglr/rglr_texture_sampler.hxx"

#include <memory_resource>

// #define TILES

namespace rqdq {
namespace {

/**
 * constant from ryg's srgb tools
 */
#define SSE_CONST4(name, val) static const __declspec(align(16)) unsigned int name[4] = { (val), (val), (val), (val) }
#define _CONST(name) *(const __m128i *)&name
#define _CONSTF(name) *(const __m128 *)&name


/**
 * based on OpenGL lod formula and ideas/source from
 * http://hugi.scene.org/online/coding/hugi%2014%20-%20comipmap.htm
 *
 * "coarse" calculation: computes a single LoD value for
 * a quad.
 */
inline
auto LevelOfDetail(rmlv::qfloat2 uvPx) -> int {
	auto& uPx = uvPx.x;
	auto& vPx = uvPx.y;

	float dux = uPx.get_y() - uPx.get_x();
	float dvx = vPx.get_y() - vPx.get_x();
	auto sqd = dux*dux + dvx*dvx;

#if 0
	float duy = uPx.get_z() - uPx.get_x();
	float dvy = vPx.get_z() - vPx.get_x();
	auto dy = duy*duy + dvy*dvy;
	sqd = std::max(sqd, dy)
#endif

	int level = (reinterpret_cast<int&>(sqd) - (127 << 23)) >> 24;
	return level; }


}  // close unnamed namespace

namespace rglr {



/**
 * texture unit for FloatingPointPixel buffers
 *
 * will WRAP_X & WRAP_Y
 *
 * wants power-of-2 size with mipmaps, stacked top (NxN) to bottom (1x1)
 * supports nearest and bilinear filtering
 * equvalent to GL_NEAREST_MIPMAP_NEAREST and GL_NEAREST_MIPMAP_LINEAR
 *
 * non-power-of-2 (or stride != width) revert to a basic sampler without
 * mipmaps or filtering
 */
template <int POWER, bool TILES>
class TextureUnitRGBAF32_P2_MIPMAP_WRAP_NEAREST : public TextureUnit {
	const rmlv::qfloat baseDim_;
	const float* const buf_;

public:
	TextureUnitRGBAF32_P2_MIPMAP_WRAP_NEAREST(const float* buf) :
		TextureUnit(),
		baseDim_(float(1<<POWER)),
		buf_(buf) {}

	void sample(const rmlv::qfloat2& uv, rmlv::qfloat4& out) const override {
		using rmlv::mvec4f, rmlv::mvec4i, rmlv::ftoi, rmlv::shl, rmlv::sar, rmlv::qfloat4, rmlv::qfloat;
		SSE_CONST4(c_almostone, 0x3f7fffff);

		// const auto baseDim = qfloat{ float(1<<POWER) };
		const auto baseTexCoord = rmlv::qfloat2{ uv.s * baseDim_, uv.t * baseDim_ };

		const int lod = std::clamp(LevelOfDetail(baseTexCoord), 0, POWER);

		const mvec4f levelDim{ float(1 << (POWER-lod)) };

		const auto levelBeginRow = (0xfffffffe << (POWER-lod)) & ((1<<(POWER+1))-1);

		// flip Y and convert to mipmap coords
		auto levelX = ftoi(                        fract(uv.x+10.0F)  * levelDim);
		auto levelY = ftoi((_CONSTF(c_almostone) - fract(uv.y+10.0F)) * levelDim);

		auto bufferX = levelX;
		auto bufferY = levelBeginRow + levelY;

		mvec4i ofs;
		if (TILES) {
			const auto tileX = sar<2>(bufferX);
			const auto tileY = sar<2>(bufferY);
			const auto inTileX = bufferX & mvec4i{0x3};
			const auto inTileY = bufferY & mvec4i{0x3};

			ofs = shl<4>(shl<POWER-2>(tileY) + tileX)
			      + shl<2>(inTileY)
			      + inTileX; }
		else {
			ofs = shl<POWER>(bufferY) + bufferX; }

		ofs = shl<2>(ofs); // 4 interleaved channels

		load_interleaved_lut(reinterpret_cast<const float*>(buf_), ofs, out); }};


template <int POWER, bool TILES>
class TextureUnitRGBA8888_P2_MIPMAP_WRAP_NEAREST : public TextureUnit {
	const rmlv::qfloat baseDim_;
	const uint32_t* const buf_;

public:
	TextureUnitRGBA8888_P2_MIPMAP_WRAP_NEAREST(const uint32_t* buf) :
		TextureUnit(),
		baseDim_(float(1<<POWER)),
		buf_(buf) {}

	void sample(const rmlv::qfloat2& uv, rmlv::qfloat4& out) const override {
		using rmlv::mvec4f, rmlv::mvec4i, rmlv::ftoi, rmlv::shl, rmlv::sar, rmlv::qfloat4, rmlv::qfloat;
		SSE_CONST4(c_almostone, 0x3f7fffff);

		// const auto baseDim = qfloat{ float(1<<POWER) };
		const auto baseTexCoord = rmlv::qfloat2{ uv.s * baseDim_, uv.t * baseDim_ };

		const int lod = std::clamp(LevelOfDetail(baseTexCoord), 0, POWER);

		const mvec4f levelDim{ float(1 << (POWER-lod)) };

		const auto levelBeginRow = (0xfffffffe << (POWER-lod)) & ((1<<(POWER+1))-1);

		// flip Y and convert to mipmap coords
		auto levelX = ftoi(                        fract(uv.x+10.0F)  * levelDim);
		auto levelY = ftoi((_CONSTF(c_almostone) - fract(uv.y+10.0F)) * levelDim);

		auto bufferX = levelX;
		auto bufferY = levelBeginRow + levelY;

		mvec4i ofs;
		if (TILES) {
			const auto tileX = sar<2>(bufferX);
			const auto tileY = sar<2>(bufferY);
			const auto inTileX = bufferX & mvec4i{0x3};
			const auto inTileY = bufferY & mvec4i{0x3};

			ofs = shl<4>(shl<POWER-2>(tileY) + tileX)
			      + shl<2>(inTileY)
			      + inTileX; }
		else {
			ofs = shl<POWER>(bufferY) + bufferX; }

		mvec4i tmp;
		load_interleaved_lut(buf_, ofs, tmp); }};


/**
 * texture unit for FloatingPointPixel buffers
 *
 * will WRAP_X & WRAP_Y
 *
 * wants power-of-2 size with mipmaps, stacked top (NxN) to bottom (1x1)
 * supports nearest and bilinear filtering
 * equvalent to GL_NEAREST_MIPMAP_NEAREST and GL_NEAREST_MIPMAP_LINEAR
 *
 * non-power-of-2 (or stride != width) revert to a basic sampler without
 * mipmaps or filtering
 */
template <int POWER, bool TILES>
class TextureUnitRGBAF32_P2_MIPMAP_WRAP_LINEAR : public TextureUnit {
	const rmlv::qfloat baseDim_;
	const float* const buf_;

public:
	TextureUnitRGBAF32_P2_MIPMAP_WRAP_LINEAR(const float* buf) :
		TextureUnit(),
		baseDim_(float(1<<POWER)),
		buf_(buf) {}

	void sample(const rmlv::qfloat2& uv, rmlv::qfloat4& out) const override {
		using rmlv::mvec4f, rmlv::mvec4i, rmlv::ftoi, rmlv::shl, rmlv::sar, rmlv::qfloat4, rmlv::qfloat;

		auto baseTexCoord = rmlv::qfloat2{ uv.s * baseDim_, uv.t * baseDim_ };

		const int lod = std::clamp(LevelOfDetail(baseTexCoord), 0, POWER);

		const int levelDim = 1 << (POWER - lod);
		const mvec4i wrapMask{ levelDim - 1 };

		// const auto levelBeginRow = (0xfffffffe << (POWER-lod)) & ((1<<(POWER+1))-1);
		const auto levelLastRow =  ((0xffffffff << (POWER-lod)) & ((1<<(POWER+1))-1)) - 1;

		auto levelX = uv.s * float(levelDim);
		auto levelY = uv.t * float(levelDim);

		auto tx0 = ftoi(levelX - 0.5F);
		auto ty0 = ftoi(levelY - 0.5F);
		auto tx1 = tx0 + 1;
		auto ty1 = ty0 + 1;

		auto fx = levelX - itof(tx0) - 0.5F;
		auto fy = levelY - itof(ty0) - 0.5F;
		auto fx1 = 1.0F - fx;
		auto fy1 = 1.0F - fy;

		auto wx0y0 = fx1 * fy1;
		auto wx1y0 = fx  * fy1;
		auto wx0y1 = fx1 * fy;
		auto wx1y1 = fx  * fy;

		tx0 = tx0 & wrapMask;  ty0 = ty0 & wrapMask;
		tx1 = tx1 & wrapMask;  ty1 = ty1 & wrapMask;

		auto bufferX0 = tx0;
		auto bufferX1 = tx1;
		auto bufferY0 = levelLastRow - ty0;
		auto bufferY1 = levelLastRow - ty1;

		qfloat4 ax;
		qfloat4 px;
		mvec4i ofs;

		if (TILES) {
			const auto tileX = sar<2>(bufferX0);
			const auto tileY = sar<2>(bufferY0);
			const auto inTileX = bufferX0 & mvec4i{0x3};
			const auto inTileY = bufferY0 & mvec4i{0x3};

			ofs = shl<4>(shl<POWER-2>(tileY) + tileX)
				  + shl<2>(inTileY)
				  + inTileX;}
		else {
			ofs = shl<POWER>(bufferY0) + bufferX0;}
		ofs = shl<2>(ofs);
		load_interleaved_lut(reinterpret_cast<const float*>(buf_), ofs, px);
		ax = px * wx0y0;

		if (TILES) {
			const auto tileX = sar<2>(bufferX1);
			const auto tileY = sar<2>(bufferY0);
			const auto inTileX = bufferX1 & mvec4i{0x3};
			const auto inTileY = bufferY0 & mvec4i{0x3};

			ofs = shl<4>(shl<POWER-2>(tileY) + tileX)
				  + shl<2>(inTileY)
				  + inTileX;}
		else {
			ofs = shl<POWER>(bufferY0) + bufferX1; }
		ofs = shl<2>(ofs);
		load_interleaved_lut(reinterpret_cast<const float*>(buf_), ofs, px);
		ax += px * wx1y0;

		if (TILES) {
			const auto tileX = sar<2>(bufferX0);
			const auto tileY = sar<2>(bufferY1);
			const auto inTileX = bufferX0 & mvec4i{0x3};
			const auto inTileY = bufferY1 & mvec4i{0x3};

			ofs = shl<4>(shl<POWER-2>(tileY) + tileX)
				  + shl<2>(inTileY)
				  + inTileX;}
		else {
			ofs = shl<POWER>(bufferY1) + bufferX0; }
		ofs = shl<2>(ofs);
		load_interleaved_lut(reinterpret_cast<const float*>(buf_), ofs, px);
		ax += px * wx0y1;

		if (TILES) {
			const auto tileX = sar<2>(bufferX1);
			const auto tileY = sar<2>(bufferY1);
			const auto inTileX = bufferX1 & mvec4i{0x3};
			const auto inTileY = bufferY1 & mvec4i{0x3};

			ofs = shl<4>(shl<POWER-2>(tileY) + tileX)
				  + shl<2>(inTileY)
				  + inTileX;}
		else {
			ofs = shl<POWER>(bufferY1) + bufferX1; }
		ofs = shl<2>(ofs);
		load_interleaved_lut(reinterpret_cast<const float*>(buf_), ofs, px);
		ax += px * wx1y1;

		out = ax; }};


class TextureUnitRGBAF32_NM_ONEMAP_WRAP_NEAREST : public TextureUnit {
	const rmlv::qfloat width_;
	const rmlv::qfloat height_;
	const rmlv::mvec4i stride_;
	const float* const buf_;

public:
	TextureUnitRGBAF32_NM_ONEMAP_WRAP_NEAREST(const float* buf, int w, int h, int s) :
		TextureUnit(),
		width_(static_cast<float>(w)),
		height_(static_cast<float>(h)),
		stride_(s),
		buf_(buf) {}

	void sample(const rmlv::qfloat2& uv, rmlv::qfloat4& out) const override {
		using rmlv::mvec4f, rmlv::mvec4i, rmlv::ftoi, rmlv::shl, rmlv::sar, rmlv::qfloat4, rmlv::qfloat;
		SSE_CONST4(c_almostone, 0x3f7fffff);
		auto pxU = ftoi(                        fract(uv.s)  * width_);
		auto pxV = ftoi((_CONSTF(c_almostone) - fract(uv.t)) * height_);
		auto ofs = pxV*stride_ + pxU;
		ofs = shl<2>(ofs);  // 4 channels

		load_interleaved_lut(reinterpret_cast<const float*>(buf_), ofs, out); }};


auto MakeTextureUnit(const float* buf, int width, int height, int stride, bool filter,  void* mem) -> const TextureUnit* {

	const auto power = rmlg::ilog2(width);
	const auto isPowerOf2 = 1<<power == width && width==height && stride==width;

	if (!isPowerOf2) {
		return new(mem) TextureUnitRGBAF32_NM_ONEMAP_WRAP_NEAREST(buf, width, height, stride); }

/**
 * tiles are 4x4, so powers < 2 are not allowed
 * X(   1,  0) \
 * X(   2,  1) \
 * XXX MSVC has sort of a bug:
 * compiler warns about instantiation of shl<POWER-2>(x)
 * (which would rightly be an error) but
 * static control-flow in the template would
 * never result in that call
 */

#define DIMS \
	X(   4,  2) \
	X(   8,  3) \
	X(  16,  4) \
	X(  32,  5) \
	X(  64,  6) \
	X( 128,  7) \
	X( 256,  8) \
	X( 512,  9) \
	X(1024, 10)
	auto dim = width;

	if (!filter) {
		switch (dim) {
#define X(DIM, POWER) case DIM: return new(mem) TextureUnitRGBAF32_P2_MIPMAP_WRAP_NEAREST<POWER, false>(buf);
			DIMS
#undef X
		default:
			std::cerr << "can't make TextureUnit for pow2 size " << dim << "\n";
			std::exit(1); }}
	else {
		switch (dim) {
#define X(DIM, POWER) case DIM: return new(mem) TextureUnitRGBAF32_P2_MIPMAP_WRAP_LINEAR<POWER, false>(buf);
			DIMS
#undef X
		default:
			std::cerr << "can't make TextureInit for pow2 size " << dim << "\n";
			std::exit(1); }}

#undef DIMS
	}


}  // close package namespace
}  // close enterprise namespace
