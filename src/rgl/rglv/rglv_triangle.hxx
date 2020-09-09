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

#if 0
constexpr int FP_BITS = 8;
constexpr int FP_MASK = 0xff;
constexpr float FP_MUL = 256.0F;
constexpr int FP_HALF = 1<<(FP_BITS-1);
#else
constexpr int FP_BITS = 4;
constexpr int FP_MASK = 0xf;
constexpr float FP_MUL = 16.0F;
constexpr int FP_HALF = 1<<(FP_BITS-1);
#endif


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
	void Draw(rmlv::vec4 s1, rmlv::vec4 s2, rmlv::vec4 s3) {
		constexpr float co = 0.0F;  // pixel-center correction offset
		int x1 = int(FP_MUL * (s1.x - co));
		int x2 = int(FP_MUL * (s2.x - co));
		int x3 = int(FP_MUL * (s3.x - co));
		int y1 = int(FP_MUL * (s1.y - co));
		int y2 = int(FP_MUL * (s2.y - co));
		int y3 = int(FP_MUL * (s3.y - co));
		Draw(x1, x2, x3, y1, y2, y3); }

	void Draw(int x1, int x2, int x3, int y1, int y2, int y3) {
		using std::max, std::min;
		using rmlv::qfloat, rmlv::qfloat2, rmlv::qfloat3;
		using rmlv::mvec4f, rmlv::mvec4i;

		const int q = 2; // block size is 2x2

		auto Ceil = [](int x) -> int {
			return (x + FP_MASK) >> FP_BITS; };
		auto Floor = [](int x) -> int {
			return x >> FP_BITS; };

		int minx = max(Floor(rmlv::Min(x1, x2, x3)), rect_.left.x);
		int endx = min( Ceil(rmlv::Max(x1, x2, x3)), rect_.right.x);
		int miny = max(Floor(rmlv::Min(y1, y2, y3)), rect_.top.y);
		int endy = min( Ceil(rmlv::Max(y1, y2, y3)), rect_.bottom.y);
		minx &= ~(q - 1); // align to 2x2 block
		miny &= ~(q - 1);

		int64_t dx12 = x1-x2, dy12 = y2-y1;
		int64_t dx23 = x2-x3, dy23 = y3-y2;
		int64_t dx31 = x3-x1, dy31 = y1-y3;

		// setup edge values at the starting point
		// offset by FP_HALF to evaluate pixel centers
		int64_t sx = (minx<<FP_BITS) + FP_HALF;
		int64_t sy = (miny<<FP_BITS) + FP_HALF;
		int64_t c1 = dy12*(sx-x1) + dx12*(sy-y1);
		int64_t c2 = dy23*(sx-x2) + dx23*(sy-y2);
		int64_t c3 = dy31*(sx-x3) + dx31*(sy-y3);

		// correct for top-left fill
#if 1
		if (dy12 > 0 || (dy12==0 && dx12>0)) c1++;  --c1;
		if (dy23 > 0 || (dy23==0 && dx23>0)) c2++;  --c2;
		if (dy31 > 0 || (dy31==0 && dx31>0)) c3++;  --c3;
#endif
		c1 >>= FP_BITS;
		c2 >>= FP_BITS;
		c3 >>= FP_BITS;

		auto cb1 = mvec4i((int)c1,        (int)(c1+dy12),
		                  (int)(c1+dx12), (int)(c1+dx12+dy12));
		auto cb2 = mvec4i((int)c2,        (int)(c2+dy23),
		                  (int)(c2+dx23), (int)(c2+dx23+dy23));
		auto cb3 = mvec4i((int)c3,        (int)(c3+dy31),
		                  (int)(c3+dx31), (int)(c3+dx31+dy31));

		auto cb1dydx = mvec4i((int)dy12*2); auto cb1dxdy = mvec4i((int)dx12*2);
		auto cb2dydx = mvec4i((int)dy23*2); auto cb2dxdy = mvec4i((int)dx23*2);
		auto cb3dydx = mvec4i((int)dy31*2); auto cb3dxdy = mvec4i((int)dx31*2);

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

				program_.Render(frag_coord, trimask, bary); }} } };


template <bool SCISSOR_TEST, typename FRAGMENT_PROCESSOR>
class VTriangleRasterizer {

	// DATA
	FRAGMENT_PROCESSOR& program_;
	const rmlv::mvec4i rectTopLeftX_;
	const rmlv::mvec4i rectTopLeftY_;
	const rmlv::mvec4i rectBottomRightX_;
	const rmlv::mvec4i rectBottomRightY_;
	const float topRowYCoord_;
	const int scissorFlags_;

public:
	// CREATORS
	VTriangleRasterizer(FRAGMENT_PROCESSOR& fp, rmlg::irect rect, int targetHeightInPx) :
		program_(fp),
		rectTopLeftX_(rect.top_left.x),
		rectTopLeftY_(rect.top_left.y),
		rectBottomRightX_(rect.bottom_right.x),
		rectBottomRightY_(rect.bottom_right.y),
		topRowYCoord_(float(targetHeightInPx) - 0.5F),
		scissorFlags_(ScissorFlags(rect)) {}

	void Draw(rmlv::mvec4i x1, rmlv::mvec4i x2, rmlv::mvec4i x3, rmlv::mvec4i y1, rmlv::mvec4i y2, rmlv::mvec4i y3, int lanes) {
		using std::max, std::min;
		using rmlv::qfloat, rmlv::qfloat2, rmlv::qfloat3;
		using rmlv::mvec4f, rmlv::mvec4i;
		using rmlv::sar, rmlv::shr, rmlv::shl;

		const mvec4i q( 2 );
		const mvec4i blockMask( ~1 );
		const mvec4i fpMask( FP_MASK );
		const mvec4i fpHalf( FP_HALF );
		const mvec4i zero( 0 );
		const mvec4i one( 1 );

		auto Ceil  = [=](mvec4i x) -> mvec4i { return sar<FP_BITS>(x + fpMask); };
		auto Floor = [](mvec4i x) -> mvec4i { return sar<FP_BITS>(x); };

		auto vminx = vmax(Floor(vmin(vmin(x1, x2), x3)), rectTopLeftX_);
		auto vmaxx = vmin( Ceil(vmax(vmax(x1, x2), x3)), rectBottomRightX_);
		auto vminy = vmax(Floor(vmin(vmin(y1, y2), y3)), rectTopLeftY_);
		auto vmaxy = vmin( Ceil(vmax(vmax(y1, y2), y3)), rectBottomRightY_);
		vminx = vminx & blockMask;
		vminy = vminy & blockMask;

		auto vdx12 = x1-x2, vdy12 = y2-y1;
		auto vdx23 = x2-x3, vdy23 = y3-y2;
		auto vdx31 = x3-x1, vdy31 = y1-y3;

		// setup edge values at the starting point
		// offset by FP_HALF to evaluate pixel centers
		auto vsx = shl<FP_BITS>(vminx) + fpHalf;
		auto vsy = shl<FP_BITS>(vminy) + fpHalf;
		auto vc1 = vdy12*(vsx-x1) + vdx12*(vsy-y1);
		auto vc2 = vdy23*(vsx-x2) + vdx23*(vsy-y2);
		auto vc3 = vdy31*(vsx-x3) + vdx31*(vsy-y3);

		// correct for top-left fill
		auto tl12 = cmpgt(vdy12, zero) | (cmpeq(vdy12,zero) & cmpgt(vdx12,zero));
		auto tl23 = cmpgt(vdy23, zero) | (cmpeq(vdy23,zero) & cmpgt(vdx23,zero));
		auto tl31 = cmpgt(vdy31, zero) | (cmpeq(vdy31,zero) & cmpgt(vdx31,zero));
		vc1 = vc1 + shr<31>(tl12) - one;
		vc2 = vc2 + shr<31>(tl23) - one;
		vc3 = vc3 + shr<31>(tl31) - one;

		vc1 = sar<FP_BITS>(vc1);
		vc2 = sar<FP_BITS>(vc2);
		vc3 = sar<FP_BITS>(vc3);

		auto vscale = mvec4f{ 1.0F } / itof(vc1 + vc2 + vc3);

		for (int li=0; li<lanes; ++li) {
			auto c1 = vc1.si[li];
			auto c2 = vc2.si[li];
			auto c3 = vc3.si[li];
			auto dx12 = vdx12.si[li], dy12 = vdy12.si[li];
			auto dx23 = vdx23.si[li], dy23 = vdy23.si[li];
			auto dx31 = vdx31.si[li], dy31 = vdy31.si[li];
			auto minx = vminx.si[li], maxx = vmaxx.si[li];
			auto miny = vminy.si[li], maxy = vmaxy.si[li];
			auto scale = vscale.lane[li];

			auto cb1 = mvec4i((int)c1,        (int)(c1+dy12),
							  (int)(c1+dx12), (int)(c1+dx12+dy12));
			auto cb2 = mvec4i((int)c2,        (int)(c2+dy23),
							  (int)(c2+dx23), (int)(c2+dx23+dy23));
			auto cb3 = mvec4i((int)c3,        (int)(c3+dy31),
							  (int)(c3+dx31), (int)(c3+dx31+dy31));

			auto cb1dydx = mvec4i((int)dy12*2); auto cb1dxdy = mvec4i((int)dx12*2);
			auto cb2dydx = mvec4i((int)dy23*2); auto cb2dxdy = mvec4i((int)dx23*2);
			auto cb3dydx = mvec4i((int)dy31*2); auto cb3dxdy = mvec4i((int)dx31*2);

			program_.Lane(li);

			program_.Begin(minx, miny);
			for (int y=miny; y<maxy; y+=2, cb1+=cb1dxdy, cb2+=cb2dxdy, cb3+=cb3dxdy, program_.CR()) {

				int sTop=0, sBot=0;
				if (SCISSOR_TEST) {
					sTop = 0;
					int rectTop = rectTopLeftY_.si[0];
					int rectBot = rectBottomRightY_.si[0];
					if ((y == (rectTop&(~1))) && (scissorFlags_ & SF_TOP)) {
						sTop=-1; }
					sBot = 0;
					if ((y >= (rectBot&(~1))) && (scissorFlags_ & SF_BOTTOM)) {
						sBot=-1; }}

				auto cx1{cb1}, cx2{cb2}, cx3{cb3};
				for (int x=minx; x<maxx; x+=2, cx1+=cb1dydx, cx2+=cb2dydx, cx3+=cb3dydx, program_.Right2()) {
					mvec4i edges{cx1|cx2|cx3};
					if (movemask(bits2float(edges)) == 0xf) continue;
					mvec4i trimask(rmlv::sar<31>(edges));

					if (SCISSOR_TEST) {
						int sLeft=0, sRight=0;
						int rectLeft = rectTopLeftX_.si[0];
						int rectRight = rectBottomRightX_.si[0];
						if ((x == (rectLeft&(~1))) && (scissorFlags_ & SF_LEFT)) {
							sLeft=-1; }
						if ((x >= (rectRight&(~1))) && (scissorFlags_ & SF_RIGHT)) {
							sRight=-1; }
						trimask |= mvec4i{ sLeft|sTop, sRight|sTop, sLeft|sBot, sRight|sBot }; }

					// lower-left-origin opengl screen coords
					const qfloat2 frag_coord = { mvec4f(x+0.5F)+mvec4f(0,1,0,1), mvec4f(topRowYCoord_-y)-mvec4f(0,0,1,1) };

					rglv::BaryCoord bary;
					bary.x = itof(cx2) * scale;
					bary.z = itof(cx1) * scale;
					bary.y = 1.0F - bary.x - bary.z;

					program_.Render(frag_coord, trimask, bary); }} }} };


}  // namespace rglv
}  // namespace rqdq
