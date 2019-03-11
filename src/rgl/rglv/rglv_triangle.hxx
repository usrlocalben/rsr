#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <xmmintrin.h>

#include "src/rml/rmlg/rmlg_irect.hxx"
#include "src/rml/rmlv/rmlv_mvec4.hxx"
#include "src/rml/rmlv/rmlv_soa.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"


namespace rqdq {
namespace {

/*
inline int iround(const float x) {
	int t;
	__asm {
		fld x
		fistp t
	}
	return t; }
*/

inline int iround(const float x) {
	return int(x); }


}  // namespace

namespace rglv {

/*
 * SoA helper for triangle vertex interpolants, qfloat
 *
 * the ctor taking 3x-qfloat is for receiving the
 * barycentric coordinates, which are used for the
 * blending-weights.
 *
 * they should be the only non-uniform values used
 * for tri_* initialization.
 */
struct tri_qfloat {
	rmlv::qfloat v0, v1, v2; };
struct tri_qfloat2 {
	rmlv::qfloat2 v0, v1, v2; };
struct tri_qfloat3 {
	rmlv::qfloat3 v0, v1, v2; };


inline auto interpolate(const tri_qfloat& bary, const tri_qfloat& val) {
	return rmlv::qfloat{
		bary.v0*val.v0 + bary.v1*val.v1 + bary.v2*val.v2
		}; }

inline auto interpolate(const tri_qfloat& bary, const tri_qfloat2& val) {
	return rmlv::qfloat2{
		bary.v0*val.v0.x + bary.v1*val.v1.x + bary.v2*val.v2.x,
		bary.v0*val.v0.y + bary.v1*val.v1.y + bary.v2*val.v2.y
		}; }

inline auto interpolate(const tri_qfloat& bary, const tri_qfloat3& val) {
	return rmlv::qfloat3{
		bary.v0*val.v0.x + bary.v1*val.v1.x + bary.v2*val.v2.x,
		bary.v0*val.v0.y + bary.v1*val.v1.y + bary.v2*val.v2.y,
		bary.v0*val.v0.z + bary.v1*val.v1.z + bary.v2*val.v2.z
		}; }


/*
inline rmlv::qfloat3 fwidth(const tri_qfloat& a) {
	return rmlv::qfloat3{
		fwidth(a.v0),
		fwidth(a.v1),
		fwidth(a.v2)
		}; }


inline rmlv::qfloat3 smoothstep(const float a, const rmlv::qfloat3& b, const tri_qfloat& x) {
	const rmlv::mvec4f _a{ a };
	return rmlv::qfloat3{
		smoothstep(_a, b.x, x.v0),
		smoothstep(_a, b.y, x.v1),
		smoothstep(_a, b.z, x.v2)
		}; }
*/


struct Edge {
	int c;
	rmlv::mvec4i b, block_left_start;
	rmlv::mvec4i bdx, bdy;

	Edge(const int x1, const int y1, const int x2, const int y2, const int startx, const int starty) {
		const rmlv::mvec4i IQX{_mm_setr_epi32(0,1,0,1)};
		const rmlv::mvec4i IQY{_mm_setr_epi32(0,0,1,1)};

		const int dx = x1 - x2;
		const int dy = y2 - y1;

		c = dy*((startx << 4) - x1) + dx*((starty << 4) - y1);

		// correct for top/left fill convention
		if (dy > 0 || (dy == 0 && dx > 0)) {
			c++; }

		c = (c - 1) >> 4;

		block_left_start = rmlv::mvec4i(c) + IQX*dy + IQY*dx;
		b = block_left_start;
		bdx = rmlv::mvec4i(dy * 2);
		bdy = rmlv::mvec4i(dx * 2); }

	inline void inc_y() {
		block_left_start += bdy;
		b = block_left_start; }

	inline void inc_x() {
		b += bdx; }

	inline const rmlv::mvec4i value() {
		return b; }
	};


template <typename FRAGMENT_PROCESSOR>
void draw_triangle(const int target_height, const rmlg::irect& r, const rmlv::vec4& s1, const rmlv::vec4& s2, const rmlv::vec4& s3, const bool frontfacing, FRAGMENT_PROCESSOR& fp) {

	using std::max;
	using std::min;
	using std::array;
	using rmlv::qfloat2;
	using rmlv::qfloat;
	using rmlv::mvec4f;
	using rmlv::mvec4i;

	const int x1 = iround(16.0f * (s1.x - 0.5f));
	const int x2 = iround(16.0f * (s2.x - 0.5f));
	const int x3 = iround(16.0f * (s3.x - 0.5f));

	const int y1 = iround(16.0f * (s1.y - 0.5f));
	const int y2 = iround(16.0f * (s2.y - 0.5f));
	const int y3 = iround(16.0f * (s3.y - 0.5f));

	int minx = max((min(min(x1,x2),x3) + 0xf) >> 4, r.left.x);
	int maxx = min((max(max(x1,x2),x3) + 0xf) >> 4, r.right.x);
	int miny = max((min(min(y1,y2),y3) + 0xf) >> 4, r.top.y);
	int maxy = min((max(max(y1,y2),y3) + 0xf) >> 4, r.bottom.y);

	const int q = 2; // block size is 2x2
	minx &= ~(q - 1); // align to 2x2 block
	miny &= ~(q - 1);

	array<Edge, 3> e{ {
		Edge{x1, y1, x2, y2, minx, miny},
		Edge{x2, y2, x3, y3, minx, miny},
		Edge{x3, y3, x1, y1, minx, miny},
	} };

	const mvec4f scale(1.0f / (e[0].c + e[1].c + e[2].c));

	const int last_row = target_height - 1;

	fp.goto_xy(minx, miny);
	for (int y = miny; y < maxy; y += 2, e[0].inc_y(), e[1].inc_y(), e[2].inc_y(), fp.inc_y()) {
		for (int x = minx; x < maxx; x += 2, e[0].inc_x(), e[1].inc_x(), e[2].inc_x(), fp.inc_x()) {

			const mvec4i edges(e[0].value() | e[1].value() | e[2].value());
			if (movemask(bits2float(edges)) == 0xf) continue;
			const mvec4i trimask(rmlv::sar<31>(edges));

			// lower-left-origin opengl screen coords
			const qfloat2 frag_coord = { mvec4f(x+0.5f)+rglv::FQX, mvec4f(last_row-y+0.5f)+rglv::FQYR };

			const qfloat b0 = itof(e[1].value()) * scale;
			const qfloat b2 = itof(e[0].value()) * scale;
			const qfloat b1 = mvec4f(1.0f) - (b0 + b2);
			const tri_qfloat bary{ b0, b1, b2 };

			fp.render(frag_coord, trimask, bary, frontfacing); }}}


template <typename FRAGMENT_PROCESSOR>
void draw_triangle(const int target_height, const rmlg::irect& r, int x1, int x2, int x3, int y1, int y2, int y3, const bool frontfacing, FRAGMENT_PROCESSOR& fp) {

	using std::max;
	using std::min;
	using std::array;
	using rmlv::qfloat2;
	using rmlv::qfloat;
	using rmlv::mvec4f;
	using rmlv::mvec4i;

	int minx = max((min(min(x1,x2),x3) + 0xf) >> 4, r.left.x);
	int maxx = min((max(max(x1,x2),x3) + 0xf) >> 4, r.right.x);
	int miny = max((min(min(y1,y2),y3) + 0xf) >> 4, r.top.y);
	int maxy = min((max(max(y1,y2),y3) + 0xf) >> 4, r.bottom.y);

	const int q = 2; // block size is 2x2
	minx &= ~(q - 1); // align to 2x2 block
	miny &= ~(q - 1);

	array<Edge, 3> e{ {
		Edge{x1, y1, x2, y2, minx, miny},
		Edge{x2, y2, x3, y3, minx, miny},
		Edge{x3, y3, x1, y1, minx, miny},
	} };

	const mvec4f scale{1.0f / (e[0].c + e[1].c + e[2].c)};

	const int last_row = target_height - 1;

	fp.goto_xy(minx, miny);
	for (int y = miny; y < maxy; y += 2, e[0].inc_y(), e[1].inc_y(), e[2].inc_y(), fp.inc_y()) {
		for (int x = minx; x < maxx; x += 2, e[0].inc_x(), e[1].inc_x(), e[2].inc_x(), fp.inc_x()) {

			const mvec4i edges(e[0].value() | e[1].value() | e[2].value());
			if (movemask(bits2float(edges)) == 0xf) continue;
			const mvec4i trimask(rmlv::sar<31>(edges));

			// lower-left-origin opengl screen coords
			const qfloat2 frag_coord = { mvec4f(x+0.5f)+rglv::FQX, mvec4f(last_row-y+0.5f)+rglv::FQYR };

			tri_qfloat bary;
			bary.v0 = itof(e[1].value()) * scale;
			bary.v2 = itof(e[0].value()) * scale;
			bary.v1 = mvec4f::one() - (bary.v0 + bary.v2);

			fp.render(frag_coord, trimask, bary, frontfacing); }}}


}  // namespace rglv
}  // namespace rqdq
