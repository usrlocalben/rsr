#pragma once
#include <rcls_aligned_containers.hxx>
#include <rmlv_soa.hxx>
#include <rmlv_mvec4.hxx>

#include <cmath>
#include <iostream>
#include <string>
#include <vector>
#include <PixelToaster.h>
#include <fmt/printf.h>

namespace rqdq {
namespace rglr {

struct Texture {
	void maybe_make_mipmap();
	void save_tga(const std::string& fn) const;

	rcls::vector<PixelToaster::FloatingPointPixel> buf;
	int width;
	int height;
	int stride;
	std::string name;
	int pow;
	bool mipmap;

	void resize(int w, int h) {
		buf.resize(w * h);
		mipmap = false;
		width = w;
		stride = w;
		height = h; }
	};


class TextureStore {
public:
	TextureStore();
	//      const Texture& get(string const key);
	void append(Texture t);
	const Texture* const find_by_name(const std::string& name) const;
	void load_dir(const std::string& prepend);
	void load_any(const std::string& prepend, const std::string& fname);
	void print();

private:
	std::vector<Texture> store;
	};


Texture load_png(const std::string filename, const std::string name, const bool premultiply);

/*
inline float mFast_Log2(float val) {
	union { float val; int32_t x; } u = { val };
	register float log_2 = (float)(((u.x >> 23) & 255) - 128);
	u.x &= ~(255 << 23);
	u.x += 127 << 23;
	log_2 += ((-0.3358287811f) * u.val + 2.0f) * u.val - 0.65871759316667f;
	return (log_2); }
*/


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
	const rmlv::qfloat d_baseDimf;
	const rmlv::mvec4i d_stride;
	const int d_power;
	int rowlut[16];
	rqdq::rmlv::qfloat4(rqdq::rglr::ts_pow2_mipmap::*d_func)(const rqdq::rmlv::qfloat2& texcoord) const;

	ts_pow2_mipmap(
		const PixelToaster::FloatingPointPixel* ptr,
		int power,
		int mode
	) :d_bitmap(ptr), d_baseDimf(float(1L << power)), d_stride(1 << power), d_power(power) {
		assert(power >= 2 && power <= 12);
		int x = 0;
		for (int p = power; p >= 0; p--) {
			int siz = 1 << p;
			rowlut[power - p] = x + siz - 1;
			x += siz; }

		if (mode == 0) {
			switch (power) {
			case 12: d_func = &ts_pow2_mipmap::sample_nearest<12>; break;
			case 11: d_func = &ts_pow2_mipmap::sample_nearest<11>; break;
			case 10: d_func = &ts_pow2_mipmap::sample_nearest<10>; break;
			case 9: d_func = &ts_pow2_mipmap::sample_nearest<9>; break;
			case 8: d_func = &ts_pow2_mipmap::sample_nearest<8>; break;
			case 7: d_func = &ts_pow2_mipmap::sample_nearest<7>; break;
			case 6: d_func = &ts_pow2_mipmap::sample_nearest<6>; break;
			case 5: d_func = &ts_pow2_mipmap::sample_nearest<5>; break;
			case 4: d_func = &ts_pow2_mipmap::sample_nearest<4>; break;
			case 3: d_func = &ts_pow2_mipmap::sample_nearest<3>; break;
			case 2: d_func = &ts_pow2_mipmap::sample_nearest<2>; break;
			default: assert(false); }}
		else if (mode == 1) {
			switch (power) {
			case 12: d_func = &ts_pow2_mipmap::sample_bilinear<12>; break;
			case 11: d_func = &ts_pow2_mipmap::sample_bilinear<11>; break;
			case 10: d_func = &ts_pow2_mipmap::sample_bilinear<10>; break;
			case 9: d_func = &ts_pow2_mipmap::sample_bilinear<9>; break;
			case 8: d_func = &ts_pow2_mipmap::sample_bilinear<8>; break;
			case 7: d_func = &ts_pow2_mipmap::sample_bilinear<7>; break;
			case 6: d_func = &ts_pow2_mipmap::sample_bilinear<6>; break;
			case 5: d_func = &ts_pow2_mipmap::sample_bilinear<5>; break;
			case 4: d_func = &ts_pow2_mipmap::sample_bilinear<4>; break;
			case 3: d_func = &ts_pow2_mipmap::sample_bilinear<3>; break;
			case 2: d_func = &ts_pow2_mipmap::sample_bilinear<2>; break;
			default: assert(false); }}}

	inline rmlv::qfloat4 sample(const rmlv::qfloat2& texcoord) const {
		return ((*this).*d_func)(texcoord); }

	template <int POWER>
	rmlv::qfloat4 sample_nearest(const rmlv::qfloat2& texcoord) const {
		using rmlv::mvec4f, rmlv::mvec4i, rmlv::ftoi, rmlv::shl, rmlv::qfloat4;

		const int level = std::min(levelOfDetail<1 << POWER>(texcoord.s, texcoord.t), POWER);

		const int levelDim = 1 << (POWER - level);
		const mvec4f levelDim_vf{ float(levelDim) };
		const mvec4i wrapMask{ levelDim - 1 };

		mvec4i lastRow{ rowlut[level] };

		auto mmu = ftoi(texcoord.s * levelDim_vf) & wrapMask;
		auto mmv = ftoi(texcoord.t * levelDim_vf) & wrapMask;

		auto ofs = (lastRow - mmv);
		ofs = shl<POWER>(ofs);
		ofs += mmu;
		ofs = shl<2>(ofs);  // 4 channels

		qfloat4 color;
		load_interleaved_lut(reinterpret_cast<const float*>(d_bitmap), ofs, color);
		return color; }

	template <int POWER>
	rmlv::qfloat4 sample_bilinear(const rmlv::qfloat2& texcoord) const {
		using rmlv::mvec4f, rmlv::mvec4i, rmlv::ftoi, rmlv::shl, rmlv::qfloat4;

		const int level = std::min(levelOfDetail<1 << POWER>(texcoord.s, texcoord.t), POWER);

		const int levelDim = 1 << (POWER - level);
		const mvec4f levelDim_vf{ float(levelDim) };
		const mvec4i wrapMask{ levelDim - 1 };

		mvec4i lastRow{ rowlut[level] };

		auto up = texcoord.s * levelDim_vf;
		auto vp = texcoord.t * levelDim_vf;

		auto tx0 = ftoi(up);
		auto ty0 = ftoi(vp);
		auto tx1 = tx0 + mvec4i{1};
		auto ty1 = ty0 + mvec4i{1};

		auto fx = up - itof(tx0);
		auto fy = vp - itof(ty0);
		auto fx1 = mvec4f{1.0f} - fx;
		auto fy1 = mvec4f{1.0f} - fy;

		auto w1 = fx1 * fy1;
		auto w2 = fx  * fy1;
		auto w3 = fx1 * fy;
		auto w4 = fx  * fy;

		qfloat4 out;
		qfloat4 px;

		tx0 = tx0 & wrapMask;  ty0 = ty0 & wrapMask;
		tx1 = tx1 & wrapMask;  ty1 = ty1 & wrapMask;
		{
			auto ofs = lastRow - ty0;
			ofs = shl<POWER>(ofs);
			ofs += tx0;
			ofs = shl<2>(ofs);
			load_interleaved_lut(reinterpret_cast<const float*>(d_bitmap), ofs, px);
			out = px * w1; }
		{
			auto ofs = lastRow - ty0;
			ofs = shl<POWER>(ofs);
			ofs += tx1;
			ofs = shl<2>(ofs);
			load_interleaved_lut(reinterpret_cast<const float*>(d_bitmap), ofs, px);
			out += px * w2; }
		{
			auto ofs = lastRow - ty1;
			ofs = shl<POWER>(ofs);
			ofs += tx0;
			ofs = shl<2>(ofs);
			load_interleaved_lut(reinterpret_cast<const float*>(d_bitmap), ofs, px);
			out += px * w3; }
		{
			auto ofs = lastRow - ty1;
			ofs = shl<POWER>(ofs);
			ofs += tx1;
			ofs = shl<2>(ofs);
			load_interleaved_lut(reinterpret_cast<const float*>(d_bitmap), ofs, px);
			out += px * w4; }

		return out; } };


}  // close package namespace
}  // close enterprise namespace