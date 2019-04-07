#include "src/rgl/rglr/rglr_kawase.hxx"

#include <algorithm>
#include <array>

#include "src/rgl/rglr/rglr_canvas.hxx"
#include "src/rml/rmlv/rmlv_mvec4.hxx"

using std::array;
using std::min;
using std::max;

namespace rqdq {
namespace rglr {

struct ofs {
	char x, y; };


constexpr array<const array<ofs, 4>, 5> kawase_offsets = { {
	{{ {-1, -1}, {0, -1}, {-1, 0}, {0, 0} }},
	{{ {-2, -2}, {1, -2}, {-2, 1}, {1, 1} }},
	{{ {-3, -3}, {2, -3}, {-3, 2}, {2, 2} }},
	{{ {-4, -4}, {3, -4}, {-4, 3}, {3, 3} }},
	{{ {-5, -5}, {4, -5}, {-5, 4}, {4, 4} }}, } };

const rmlv::mvec4f ONE_OVER_16{ 1.0F / 16.0F };

constexpr int SAFE_ZONE = 5;


inline int clamp_int(const int a, const int l, const int h) {
	return min(max(a, l), h); }

namespace Blur {


void Kawase::blur_copy(const int y0,
                           const int y1,
                           const int dist,
                           const rglr::FloatingPointCanvas& src,
                           rglr::FloatingPointCanvas& dst) {
	auto& offsets = kawase_offsets[dist];
	auto in = reinterpret_cast<const rmlv::mvec4f*>(src.cdata());

	auto sample_clamp = [&](int x, int y) {
		const int _x = clamp_int(x, 0, src.width() - 1);
		const int _y = clamp_int(y, 0, src.height() - 1);
		return in[_y * src.stride() + _x]; };

	auto sample_safe = [&](int x, int y) {
		return (sample_clamp(x + 0, y + 0) + sample_clamp(x + 1, y + 0) +
		        sample_clamp(x + 0, y + 1) + sample_clamp(x + 1, y + 1)); };

	auto sample_fast = [&](int x, int y) {
		auto ptr = &in[y * src.stride() + x];
		return (ptr[0]          + ptr[1] +
		        ptr[src.stride()] + ptr[src.stride() + 1]); };

	auto out = reinterpret_cast<rmlv::mvec4f*>(dst.data() + y0 * dst.stride());

	for (int y = y0; y < y1; y++) {
		if (y < SAFE_ZONE || y > dst.height() - SAFE_ZONE) {
			for (int x = 0; x < dst.width(); x++) {
				rmlv::mvec4f ax{ 0 };
				for (const auto& ofs : offsets) {
					ax += sample_safe(x + ofs.x, y + ofs.y); }
				(ax * ONE_OVER_16).store(&out[x].v); }}
		else {
			int x;
			for (x = 0; x < SAFE_ZONE; x++) {
				rmlv::mvec4f ax{ 0 };
				for (const auto& ofs : offsets) {
					ax += sample_safe(x + ofs.x, y + ofs.y); }
				(ax * ONE_OVER_16).store(&out[x].v); }
			for (; x < dst.width() - SAFE_ZONE; x++) {
				rmlv::mvec4f ax{ 0 };
				for (const auto& ofs : offsets) {
					ax += sample_fast(x + ofs.x, y + ofs.y); }
				(ax * ONE_OVER_16).store(&out[x].v); }
			for (; x < dst.width(); x++) {
				rmlv::mvec4f ax{ 0 };
				for (const auto& ofs : offsets) {
					ax += sample_safe(x + ofs.x, y + ofs.y); }
				(ax * ONE_OVER_16).store(&out[x].v); }}
		out += dst.stride(); }}

}  // namespace Blur


}  // namespace rglr
}  // namespace rqdq
