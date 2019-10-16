#include "src/rgl/rglr/rglr_kawase.hxx"

#include <algorithm>
#include <array>

#include "src/rgl/rglr/rglr_canvas.hxx"
#include "src/rml/rmlv/rmlv_mvec4.hxx"

namespace rqdq {
namespace {

struct ofs {
	int x, y; };

inline int clamp_int(int a, int l, int h) {
	return std::min(std::max(a, l), h); }


}  // namespace
namespace rglr {

void KawaseBlurFilter(const rglr::FloatingPointCanvas& src,
                      rglr::FloatingPointCanvas& dst,
                      int d, int yBegin, int yEnd) {
	using rmlv::mvec4f;
	const std::array<ofs, 4> offsets = {{
		{-d-1, -d-1}, {d, -d-1}, {-d-1, d}, {d,d} }};

	auto in = reinterpret_cast<const mvec4f*>(src.cdata());
	auto out = reinterpret_cast<mvec4f*>(dst.data() + yBegin*dst.stride());
	const mvec4f oneOver16{ 1.0F / 16.0F };

	const int srcStride = src.stride();

	auto sample_clamp = [&](int x, int y) {
		const int _x = clamp_int(x, 0, src.width() - 1);
		const int _y = clamp_int(y, 0, src.height() - 1);
		return in[_y*srcStride + _x]; };

	auto sample_safe = [&](int x, int y) {
		return (sample_clamp(x+0, y+0) + sample_clamp(x+1, y+0) +
		        sample_clamp(x+0, y+1) + sample_clamp(x+1, y+1)); };

	auto sample_fast = [&](int x, int y) {
		auto row1 = in + y*srcStride + x;
		auto row2 = row1 + srcStride;
		return (row1[0] + row1[1] +
		        row2[0] + row2[1]); };

	const int margin = d + 2;
	const int ySafeBegin = margin;
	const int ySafeEnd = dst.height() - margin;
	const int xEnd = dst.width();
	const int xSafeBegin = margin;
	const int xSafeEnd = xEnd - margin;

	for (int y=yBegin; y<yEnd; ++y) {
		if (y < ySafeBegin || ySafeEnd < y) {
			for (int x=0; x<xEnd; ++x) {
				mvec4f ax = mvec4f::zero();
				for (auto ofs : offsets) {
					ax += sample_safe(x + ofs.x, y + ofs.y); }
				out[x] = ax * oneOver16; }}
		else {
			int x{0};
			for (; x<xSafeBegin; ++x) {
				mvec4f ax = mvec4f::zero();
				for (auto ofs : offsets) {
					ax += sample_safe(x+ofs.x, y+ofs.y); }
				out[x] = ax * oneOver16; }
			for (; x<xSafeEnd; ++x) {
				mvec4f ax = mvec4f::zero();
				for (auto ofs : offsets) {
					ax += sample_fast(x+ofs.x, y+ofs.y); }
				out[x] = ax * oneOver16; }
			for (; x <xEnd; ++x) {
				mvec4f ax = mvec4f::zero();
				for (auto ofs : offsets) {
					ax += sample_safe(x+ofs.x, y+ofs.y); }
				out[x] = ax * oneOver16; }}
		out += dst.stride(); }}


}  // namespace rglr
}  // namespace rqdq
