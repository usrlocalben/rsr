#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <xmmintrin.h>

#include "src/rgl/rglv/rglv_interpolate.hxx"
#include "src/rml/rmlg/rmlg_irect.hxx"
#include "src/rml/rmlv/rmlv_mvec4.hxx"
#include "src/rml/rmlv/rmlv_soa.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"

namespace rqdq {
namespace rglv {

constexpr int SF_TOP = 1;
constexpr int SF_BOTTOM = 2;
constexpr int SF_LEFT = 4;
constexpr int SF_RIGHT = 8;

inline 
auto ScissorFlags(const rmlg::irect& rect) -> int {
	int fTop    =  (rect.top.y    & 1) > 0;
	int fBottom = ((rect.bottom.y & 1) > 0) << 1;
	int fLeft   = ((rect.left.x   & 1) > 0) << 2;
	int fRight  = ((rect.right.x  & 1) > 0) << 3;
	return fTop | fBottom | fLeft | fRight; }

/*
struct BlahFp {
	auto Begin(int x [[maybe_unused]], int y [[maybe_unused]]) -> void {};
	auto CR() -> void {};
	auto Right2() -> void {};
	auto Render(rmlv::qfloat2 frag_coord, rmlv::mvec4i trimask, rglv::BaryCoord BS, bool frontfacing) -> void {}; };
*/

constexpr int FP_BITS = 8;
constexpr int FP_MASK = 0xff;
constexpr float FP_MUL = 256.0F;


template <bool SCISSOR_TEST, typename FRAGMENT_PROCESSOR>
class TriangleRasterizer {

	// DATA
	FRAGMENT_PROCESSOR& program_;
	const rmlg::irect rect_;
	const int targetHeightInPx_;
	const int scissorFlags_;

public:
	// CREATORS
	TriangleRasterizer(FRAGMENT_PROCESSOR& fp, rmlg::irect rect, int targetHeightInPx) :
		program_(fp),
		rect_(rect),
		targetHeightInPx_(targetHeightInPx),
		scissorFlags_(ScissorFlags(rect_)) {}

	// MANIPULATORS
	void Draw(rmlv::vec4 s1, rmlv::vec4 s2, rmlv::vec4 s3, const bool frontfacing) {
		int x1 = int(FP_MUL * (s1.x - 0.5f));
		int x2 = int(FP_MUL * (s2.x - 0.5f));
		int x3 = int(FP_MUL * (s3.x - 0.5f));
		int y1 = int(FP_MUL * (s1.y - 0.5f));
		int y2 = int(FP_MUL * (s2.y - 0.5f));
		int y3 = int(FP_MUL * (s3.y - 0.5f));
		Draw(x1, x2, x3, y1, y2, y3, frontfacing); }

	void Draw(int x1, int x2, int x3, int y1, int y2, int y3, const bool frontfacing) {
		using std::max, std::min;
		using rmlv::qfloat, rmlv::qfloat2, rmlv::qfloat3;
		using rmlv::mvec4f, rmlv::mvec4i;

		const int q = 2; // block size is 2x2

		int minx = max((rmlv::Min(x1, x2, x3) + FP_MASK) >> FP_BITS, rect_.left.x);
		int endx = min((rmlv::Max(x1, x2, x3) + FP_MASK) >> FP_BITS, rect_.right.x);
		int miny = max((rmlv::Min(y1, y2, y3) + FP_MASK) >> FP_BITS, rect_.top.y);
		int endy = min((rmlv::Max(y1, y2, y3) + FP_MASK) >> FP_BITS, rect_.bottom.y);
		minx &= ~(q - 1); // align to 2x2 block
		miny &= ~(q - 1);

		int64_t dx12 = x1 - x2, dy12 = y2 - y1;
		int64_t dx23 = x2 - x3, dy23 = y3 - y2;
		int64_t dx31 = x3 - x1, dy31 = y1 - y3;

		// setup edge values at the starting point
		int64_t c1 = dy12*((minx<<FP_BITS) - x1) + dx12*((miny<<FP_BITS)-y1);
		int64_t c2 = dy23*((minx<<FP_BITS) - x2) + dx23*((miny<<FP_BITS)-y2);
		int64_t c3 = dy31*((minx<<FP_BITS) - x3) + dx31*((miny<<FP_BITS)-y3);

		// correct for top-left fill
		if (dy12 > 0 || (dy12==0 && dx12>0)) c1++;  --c1;
		if (dy23 > 0 || (dy23==0 && dx23>0)) c2++;  --c2;
		if (dy31 > 0 || (dy31==0 && dx31>0)) c3++;  --c3;

		c1 >>= FP_BITS;
		c2 >>= FP_BITS;
		c3 >>= FP_BITS;

		auto cb1 = mvec4i(c1) + mvec4i(0,dy12,0,dy12) + mvec4i(0,0,dx12,dx12);
		auto cb2 = mvec4i(c2) + mvec4i(0,dy23,0,dy23) + mvec4i(0,0,dx23,dx23);
		auto cb3 = mvec4i(c3) + mvec4i(0,dy31,0,dy31) + mvec4i(0,0,dx31,dx31);
		auto cb1dydx = mvec4i(dy12*2); auto cb1dxdy = mvec4i(dx12*2);
		auto cb2dydx = mvec4i(dy23*2); auto cb2dxdy = mvec4i(dx23*2);
		auto cb3dydx = mvec4i(dy31*2); auto cb3dxdy = mvec4i(dx31*2);

		mvec4f scale{1.0f / (c1 + c2 + c3)};

		program_.Begin(minx, miny);
		for (int y=miny; y<endy; y+=2, cb1+=cb1dxdy, cb2+=cb2dxdy, cb3+=cb3dxdy, program_.CR()) {

			int sTop=0, sBot=0;
			if (SCISSOR_TEST) {
				sTop = 0;
				if ((y == (rect_.top.y&(~1))) && (scissorFlags_ & SF_TOP)) {
					sTop=-1; }
				sBot = 0;
				if ((y >= (rect_.bottom.y&(~1))) && (scissorFlags_ & SF_BOTTOM)) {
					sBot=-1; }}

			auto cx1{cb1}, cx2{cb2}, cx3{cb3};
			for (int x=minx; x<endx; x+=2, cx1+=cb1dydx, cx2+=cb2dydx, cx3+=cb3dydx, program_.Right2()) {
				mvec4i edges{cx1|cx2|cx3};
				if (movemask(bits2float(edges)) == 0xf) continue;
				mvec4i trimask(rmlv::sar<31>(edges));

				if (SCISSOR_TEST) {
					int sLeft=0, sRight=0;
					if ((x == (rect_.left.x&(~1))) && (scissorFlags_ & SF_LEFT)) {
						sLeft=-1; }
					if ((x >= (rect_.right.x&(~1))) && (scissorFlags_ & SF_RIGHT)) {
						sRight=-1; }
					trimask |= mvec4i{ sLeft|sTop, sRight|sTop, sLeft|sBot, sRight|sBot }; }

				// lower-left-origin opengl screen coords
				const qfloat2 frag_coord = { mvec4f(x+0.5f)+mvec4f{0,1,0,1}, mvec4f(targetHeightInPx_-y-0.5f)+mvec4f{1,1,0,0} };

				rglv::BaryCoord bary;
				bary.x = itof(cx2) * scale;
				bary.z = itof(cx1) * scale;
				bary.y = 1.0F - bary.x - bary.z;

				program_.Render(frag_coord, trimask, bary, frontfacing); }} } };


}  // namespace rglv
}  // namespace rqdq
