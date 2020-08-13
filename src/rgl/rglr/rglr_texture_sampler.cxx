#include "src/rgl/rglr/rglr_texture_sampler.hxx"

// #define TILES

namespace rqdq {
namespace {

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

FloatingPointPixelUnit::FloatingPointPixelUnit(
	const PixelToaster::FloatingPointPixel* ptr,
	int width, int height, int stride, int mode) :
	buf_(ptr),
	stride_(stride),
	height_(static_cast<float>(height)),
	width_(static_cast<float>(width)),
	power_(rmlg::ilog2(width)),
	isPowerOf2_(1<<power_ == width && width == height && stride == width),
	baseDim_(static_cast<float>(width)),
	sampleFunc_(nullptr) {

	if (!isPowerOf2_) {
		sampleFunc_ = &FloatingPointPixelUnit::sample_nearest_nonpow2;
		return; }

	// power-of-2 texture
	assert(power_ >= 0 && power_ <= 12);

	if (mode == 0) {
		switch (power_) {
		case 12: sampleFunc_ = &FloatingPointPixelUnit::sample_nearest<12>; break;
		case 11: sampleFunc_ = &FloatingPointPixelUnit::sample_nearest<11>; break;
		case 10: sampleFunc_ = &FloatingPointPixelUnit::sample_nearest<10>; break;
		case  9: sampleFunc_ = &FloatingPointPixelUnit::sample_nearest< 9>; break;
		case  8: sampleFunc_ = &FloatingPointPixelUnit::sample_nearest< 8>; break;
		case  7: sampleFunc_ = &FloatingPointPixelUnit::sample_nearest< 7>; break;
		case  6: sampleFunc_ = &FloatingPointPixelUnit::sample_nearest< 6>; break;
		case  5: sampleFunc_ = &FloatingPointPixelUnit::sample_nearest< 5>; break;
		case  4: sampleFunc_ = &FloatingPointPixelUnit::sample_nearest< 4>; break;
		case  3: sampleFunc_ = &FloatingPointPixelUnit::sample_nearest< 3>; break;
		case  2: sampleFunc_ = &FloatingPointPixelUnit::sample_nearest< 2>; break;
		case  1: sampleFunc_ = &FloatingPointPixelUnit::sample_nearest< 1>; break;
		case  0: sampleFunc_ = &FloatingPointPixelUnit::sample_nearest< 0>; break;
		default: assert(false); }}
	else if (mode == 1) {
		switch (power_) {
		case 12: sampleFunc_ = &FloatingPointPixelUnit::sample_linear<12>; break;
		case 11: sampleFunc_ = &FloatingPointPixelUnit::sample_linear<11>; break;
		case 10: sampleFunc_ = &FloatingPointPixelUnit::sample_linear<10>; break;
		case  9: sampleFunc_ = &FloatingPointPixelUnit::sample_linear< 9>; break;
		case  8: sampleFunc_ = &FloatingPointPixelUnit::sample_linear< 8>; break;
		case  7: sampleFunc_ = &FloatingPointPixelUnit::sample_linear< 7>; break;
		case  6: sampleFunc_ = &FloatingPointPixelUnit::sample_linear< 6>; break;
		case  5: sampleFunc_ = &FloatingPointPixelUnit::sample_linear< 5>; break;
		case  4: sampleFunc_ = &FloatingPointPixelUnit::sample_linear< 4>; break;
		case  3: sampleFunc_ = &FloatingPointPixelUnit::sample_linear< 3>; break;
		case  2: sampleFunc_ = &FloatingPointPixelUnit::sample_linear< 2>; break;
		case  1: sampleFunc_ = &FloatingPointPixelUnit::sample_linear< 1>; break;
		case  0: sampleFunc_ = &FloatingPointPixelUnit::sample_linear< 0>; break;
		default: assert(false); }}}


auto FloatingPointPixelUnit::sample_nearest_nonpow2(const rmlv::qfloat2& texcoord) const -> rmlv::qfloat4 {
		using rmlv::mvec4f, rmlv::mvec4i, rmlv::ftoi, rmlv::shl, rmlv::qfloat4;

		auto pxU = ftoi(        fract(texcoord.s)  * width_);
		auto pxV = ftoi((1.0F - fract(texcoord.t)) * height_);
		auto ofs = pxV*stride_ + pxU;
		ofs = shl<2>(ofs);  // 4 channels

		qfloat4 color;
		load_interleaved_lut(reinterpret_cast<const float*>(buf_), ofs, color);
		return color; }


template <int POWER>
auto FloatingPointPixelUnit::sample_nearest(const rmlv::qfloat2& uv) const -> rmlv::qfloat4 {
	using rmlv::mvec4f, rmlv::mvec4i, rmlv::ftoi, rmlv::shl, rmlv::sar, rmlv::qfloat4, rmlv::qfloat;

	// const auto baseDim = qfloat{ float(1<<POWER) };
	const auto baseTexCoord = rmlv::qfloat2{ uv.s * baseDim_, uv.t * baseDim_ };

	const int lod = std::clamp(LevelOfDetail(baseTexCoord), 0, POWER);

	const mvec4f levelDim{ float(1 << (POWER-lod)) };

	const auto levelBeginRow = (0xfffffffe << (POWER-lod)) & ((1<<(POWER+1))-1);

	// flip Y and convert to mipmap coords
	auto levelX = ftoi(        fract(uv.x+10.0F)  * levelDim);
	auto levelY = ftoi((1.0F - fract(uv.y+10.0F)) * levelDim);

	auto bufferX = levelX;
	auto bufferY = levelBeginRow + levelY;

#ifdef TILES
	const auto tileX = sar<2>(bufferX);
	const auto tileY = sar<2>(bufferY);
	const auto inTileX = bufferX & mvec4i{0x3};
	const auto inTileY = bufferY & mvec4i{0x3};

	auto ofs = shl<4>(shl<POWER-2>(tileY) + tileX)
			   + shl<2>(inTileY)
			   + inTileX;
#else
	auto ofs = shl<POWER>(bufferY) + bufferX;
#endif

	ofs = shl<2>(ofs); // 4 interleaved channels

	qfloat4 color;
	load_interleaved_lut(reinterpret_cast<const float*>(buf_), ofs, color);
	return color; }


template <int POWER>
auto FloatingPointPixelUnit::sample_linear(const rmlv::qfloat2& uv) const -> rmlv::qfloat4 {
	using rmlv::mvec4f, rmlv::mvec4i, rmlv::ftoi, rmlv::shl, rmlv::sar, rmlv::qfloat4, rmlv::qfloat;

	// const auto baseDim = qfloat{ float(1<<POWER) };
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

#ifdef TILES
	{const auto tileX = sar<2>(bufferX0);
	const auto tileY = sar<2>(bufferY0);
	const auto inTileX = bufferX0 & mvec4i{0x3};
	const auto inTileY = bufferY0 & mvec4i{0x3};

	ofs = shl<4>(shl<POWER-2>(tileY) + tileX)
		  + shl<2>(inTileY)
		  + inTileX;}
#else
	ofs = shl<POWER>(bufferY0) + bufferX0;
#endif
	ofs = shl<2>(ofs);
	load_interleaved_lut(reinterpret_cast<const float*>(buf_), ofs, px);
	ax = px * wx0y0;

#ifdef TILES
	{const auto tileX = sar<2>(bufferX1);
	const auto tileY = sar<2>(bufferY0);
	const auto inTileX = bufferX1 & mvec4i{0x3};
	const auto inTileY = bufferY0 & mvec4i{0x3};

	ofs = shl<4>(shl<POWER-2>(tileY) + tileX)
		  + shl<2>(inTileY)
		  + inTileX;}
#else
	ofs = shl<POWER>(bufferY0) + bufferX1;
#endif
	ofs = shl<2>(ofs);
	load_interleaved_lut(reinterpret_cast<const float*>(buf_), ofs, px);
	ax += px * wx1y0;

#ifdef TILES
	{const auto tileX = sar<2>(bufferX0);
	const auto tileY = sar<2>(bufferY1);
	const auto inTileX = bufferX0 & mvec4i{0x3};
	const auto inTileY = bufferY1 & mvec4i{0x3};

	ofs = shl<4>(shl<POWER-2>(tileY) + tileX)
		  + shl<2>(inTileY)
		  + inTileX;}
#else
	ofs = shl<POWER>(bufferY1) + bufferX0;
#endif
	ofs = shl<2>(ofs);
	load_interleaved_lut(reinterpret_cast<const float*>(buf_), ofs, px);
	ax += px * wx0y1;

#ifdef TILES
	{const auto tileX = sar<2>(bufferX1);
	const auto tileY = sar<2>(bufferY1);
	const auto inTileX = bufferX1 & mvec4i{0x3};
	const auto inTileY = bufferY1 & mvec4i{0x3};

	ofs = shl<4>(shl<POWER-2>(tileY) + tileX)
		  + shl<2>(inTileY)
		  + inTileX;}
#else
	ofs = shl<POWER>(bufferY1) + bufferX1;
#endif
	ofs = shl<2>(ofs);
	load_interleaved_lut(reinterpret_cast<const float*>(buf_), ofs, px);
	ax += px * wx1y1;

	return ax; }

}  // close package namespace
}  // close enterprise namespace
