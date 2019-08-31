#pragma once
#include <cassert>

#include "src/rml/rmlg/rmlg_pow2.hxx"
#include "src/rml/rmlv/rmlv_mvec4.hxx"
#include "src/rml/rmlv/rmlv_soa.hxx"

#include "3rdparty/pixeltoaster/PixelToaster.h"

namespace rqdq {
namespace rglr {

template <int dim>
inline int levelOfDetail(const rmlv::qfloat uCoord, const rmlv::qfloat vCoord) {
	const rmlv::qfloat dimension{ float(dim) };
	const auto pixelUCoord = uCoord * dimension;
	const auto pixelVCoord = vCoord * dimension;

	auto duxduy = pixelUCoord.yzxw() - pixelUCoord.xxxx();
	duxduy = duxduy * duxduy;
	const auto dot_u = duxduy.xxxx() + duxduy.yyyy();

	auto dvxdvy = pixelVCoord.yzxw() - pixelVCoord.xxxx();
	dvxdvy = dvxdvy * dvxdvy;
	const auto dot_v = dvxdvy.xxxx() + dvxdvy.yyyy();

	float delta_max_sqr = vmax(dot_u, dot_v).get_x();
	int level;

	// http://hugi.scene.org/online/coding/hugi%2014%20-%20comipmap.htm
	level = ((*reinterpret_cast<int*>(&delta_max_sqr)) - (127 << 23)) >> 24;

	//level = int(0.5f * mFast_Log2(delta_max_sqr));

	level = std::max(level, 0);
	return level; }


struct ts_pow2_mipmap {
	const PixelToaster::FloatingPointPixel* const d_bitmap;

	rmlv::mvec4i d_stride;
	rmlv::mvec4f d_height;
	rmlv::mvec4f d_width;
	rmlv::mvec4i d_uLastRow;

	rqdq::rmlv::qfloat4(rqdq::rglr::ts_pow2_mipmap::*d_func)(const rqdq::rmlv::qfloat2& texcoord) const;

	// only for mipmaps
	rmlv::qfloat d_baseDimf;
	const PixelToaster::FloatingPointPixel* levelData_[16];

	ts_pow2_mipmap(
		const PixelToaster::FloatingPointPixel* ptr,
		int width, int height, int row_stride, int mode
	) :d_bitmap(ptr) {
		d_stride = row_stride;
		if (rmlg::is_pow2(width) && width == height && row_stride == width) {
			// power-of-2 texture
			int power = rmlg::ilog2(width);
			d_baseDimf = float(width);
			assert(2 <= power && power <= 12);

			// rowlut[level#] is the row# for the top of level# in the mipmap
			// e.g. for power=8 (256x256)
			// 0=0, 1=256, 2=384, ... 8=510, 9=511
			int x = 0;
			for (int p=power; p>=0; p--) {
				int level = power - p;
				int levelDim = 1 << p;
				levelData_[level] = &d_bitmap[x * width];
				x += levelDim; }

			if (mode == 0) {
				switch (power) {
				case 12: d_func = &ts_pow2_mipmap::sample_nearest_nearest<12>; break;
				case 11: d_func = &ts_pow2_mipmap::sample_nearest_nearest<11>; break;
				case 10: d_func = &ts_pow2_mipmap::sample_nearest_nearest<10>; break;
				case  9: d_func = &ts_pow2_mipmap::sample_nearest_nearest< 9>; break;
				case  8: d_func = &ts_pow2_mipmap::sample_nearest_nearest< 8>; break;
				case  7: d_func = &ts_pow2_mipmap::sample_nearest_nearest< 7>; break;
				case  6: d_func = &ts_pow2_mipmap::sample_nearest_nearest< 6>; break;
				case  5: d_func = &ts_pow2_mipmap::sample_nearest_nearest< 5>; break;
				case  4: d_func = &ts_pow2_mipmap::sample_nearest_nearest< 4>; break;
				case  3: d_func = &ts_pow2_mipmap::sample_nearest_nearest< 3>; break;
				case  2: d_func = &ts_pow2_mipmap::sample_nearest_nearest< 2>; break;
				default: assert(false); }}
			else if (mode == 1) {
				switch (power) {
				case 12: d_func = &ts_pow2_mipmap::sample_nearest_linear <12>; break;
				case 11: d_func = &ts_pow2_mipmap::sample_nearest_linear <11>; break;
				case 10: d_func = &ts_pow2_mipmap::sample_nearest_linear <10>; break;
				case  9: d_func = &ts_pow2_mipmap::sample_nearest_linear < 9>; break;
				case  8: d_func = &ts_pow2_mipmap::sample_nearest_linear < 8>; break;
				case  7: d_func = &ts_pow2_mipmap::sample_nearest_linear < 7>; break;
				case  6: d_func = &ts_pow2_mipmap::sample_nearest_linear < 6>; break;
				case  5: d_func = &ts_pow2_mipmap::sample_nearest_linear < 5>; break;
				case  4: d_func = &ts_pow2_mipmap::sample_nearest_linear < 4>; break;
				case  3: d_func = &ts_pow2_mipmap::sample_nearest_linear < 3>; break;
				case  2: d_func = &ts_pow2_mipmap::sample_nearest_linear < 2>; break;
				default: assert(false); }}}
		else {
			// non power-of-2 texture
			d_height = height;
			d_uLastRow = rmlv::mvec4i{height - 1};
			d_width = width;
			d_func = &ts_pow2_mipmap::sample_zero_nearest_nonpow2; }}

	inline rmlv::qfloat4 sample(const rmlv::qfloat2& texcoord) const {
		return ((*this).*d_func)(texcoord); }

	template <int POWER>
	rmlv::qfloat4 sample_nearest_nearest(const rmlv::qfloat2& texcoord) const {
		using rmlv::mvec4f, rmlv::mvec4i, rmlv::ftoi, rmlv::shl, rmlv::qfloat4;

		const int level = 3; //std::min(levelOfDetail<1 << POWER>(texcoord.s, texcoord.t), POWER);
		auto levelData = levelData_[level];

		const int levelDim = 1 << (POWER - level);
		const mvec4f levelDim_vf{ float(levelDim) };
		const mvec4i wrapMask{ levelDim - 1 };

		auto tu = ftoi(texcoord.s * levelDim_vf) & wrapMask;
		auto tv = ftoi(texcoord.t * levelDim_vf) & wrapMask;

		// v*width + u,  then FPP* to float*
		auto ofs = shl<2>(shl<POWER>(tv) + tu);

		qfloat4 color;
		load_interleaved_lut(reinterpret_cast<const float*>(levelData), ofs, color);
		return color; }

	rmlv::qfloat4 sample_zero_nearest_nonpow2(const rmlv::qfloat2& texcoord) const {
		using rmlv::mvec4f, rmlv::mvec4i, rmlv::ftoi, rmlv::shl, rmlv::qfloat4;

		auto pxU = ftoi(fract(texcoord.s) * d_width); // XXX floor?
		auto pxV = ftoi(fract(texcoord.t) * d_height);

		auto ofs = (d_uLastRow - pxV)*d_stride + pxU;
		ofs = shl<2>(ofs);  // 4 channels

		qfloat4 color;
		load_interleaved_lut(reinterpret_cast<const float*>(d_bitmap), ofs, color);
		return color; }

	template <int POWER>
	rmlv::qfloat4 sample_nearest_linear(const rmlv::qfloat2& texcoord) const {
		using rmlv::mvec4f, rmlv::mvec4i, rmlv::ftoi, rmlv::shl, rmlv::qfloat4;

		const int level = std::min(levelOfDetail<1 << POWER>(texcoord.s, texcoord.t), POWER);
		auto levelData = levelData_[level];

		const int levelDim = 1 << (POWER - level);
		const mvec4f levelDim_vf{ float(levelDim) };
		const mvec4i wrapMask{ levelDim - 1 };

		auto up = texcoord.s * levelDim_vf;
		auto vp = texcoord.t * levelDim_vf;

		auto tx0 = ftoi(up);
		auto ty0 = ftoi(vp);
		auto tx1 = tx0 + mvec4i{1};
		auto ty1 = ty0 + mvec4i{1};

		auto fx = up - itof(tx0);
		auto fy = vp - itof(ty0);
		auto fx1 = mvec4f{1.0F} - fx;
		auto fy1 = mvec4f{1.0F} - fy;

		auto w1 = fx1 * fy1;
		auto w2 = fx  * fy1;
		auto w3 = fx1 * fy;
		auto w4 = fx  * fy;

		qfloat4 out;
		qfloat4 px;

		tx0 = tx0 & wrapMask;  ty0 = ty0 & wrapMask;
		tx1 = tx1 & wrapMask;  ty1 = ty1 & wrapMask;
		{
			auto ofs = shl<2>(shl<POWER>(ty0) + tx0);
			load_interleaved_lut(reinterpret_cast<const float*>(levelData), ofs, px);
			out = px * w1; }
		{
			auto ofs = shl<2>(shl<POWER>(ty0) + tx1);
			load_interleaved_lut(reinterpret_cast<const float*>(levelData), ofs, px);
			out += px * w2; }
		{
			auto ofs = shl<2>(shl<POWER>(ty1) + tx0);
			load_interleaved_lut(reinterpret_cast<const float*>(levelData), ofs, px);
			out += px * w3; }
		{
			auto ofs = shl<2>(shl<POWER>(ty1) + tx1);
			load_interleaved_lut(reinterpret_cast<const float*>(levelData), ofs, px);
			out += px * w4; }

		return out; } };


}  // namespace rglr
}  // namespace rqdq
