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


template <typename FRAGMENT_PROCESSOR>
class TriangleRasterizer {
public:
	TriangleRasterizer(FRAGMENT_PROCESSOR& fp, rmlg::irect rect, int targetHeightInPx) :
		program_(fp),
		rect_(rect),
		targetHeightInPx_(targetHeightInPx) {}

	void Draw(rmlv::vec4 s1, rmlv::vec4 s2, rmlv::vec4 s3, const bool frontfacing) {
		int x1 = int(16.0f * (s1.x - 0.5f));
		int x2 = int(16.0f * (s2.x - 0.5f));
		int x3 = int(16.0f * (s3.x - 0.5f));
		int y1 = int(16.0f * (s1.y - 0.5f));
		int y2 = int(16.0f * (s2.y - 0.5f));
		int y3 = int(16.0f * (s3.y - 0.5f));
		Draw(x1, x2, x3, y1, y2, y3, frontfacing); }

	void Draw(int x1, int x2, int x3, int y1, int y2, int y3, const bool frontfacing) {
		using std::max, std::min;
		using rmlv::qfloat, rmlv::qfloat2, rmlv::qfloat3;
		using rmlv::mvec4f, rmlv::mvec4i;

		const int q = 2; // block size is 2x2

		int minx = max((min(min(x1,x2),x3) + 0xf) >> 4, rect_.left.x);
		int endx = min((max(max(x1,x2),x3) + 0xf) >> 4, rect_.right.x);
		int miny = max((min(min(y1,y2),y3) + 0xf) >> 4, rect_.top.y);
		int endy = min((max(max(y1,y2),y3) + 0xf) >> 4, rect_.bottom.y);
		minx &= ~(q - 1); // align to 2x2 block
		miny &= ~(q - 1);

		int dx12 = x1 - x2, dy12 = y2 - y1;
		int dx23 = x2 - x3, dy23 = y3 - y2;
		int dx31 = x3 - x1, dy31 = y1 - y3;
		int c1 = dy12*((minx<<4) - x1) + dx12*((miny<<4)-y1);
		int c2 = dy23*((minx<<4) - x2) + dx23*((miny<<4)-y2);
		int c3 = dy31*((minx<<4) - x3) + dx31*((miny<<4)-y3);
		if (dy12 > 0 || (dy12==0 && dx12>0)) c1++;
		if (dy23 > 0 || (dy23==0 && dx23>0)) c2++;
		if (dy31 > 0 || (dy31==0 && dx31>0)) c3++;
		c1 = (c1 - 1) >> 4;
		c2 = (c2 - 1) >> 4;
		c3 = (c3 - 1) >> 4;

		auto cb1 = mvec4i{0,dy12,0,dy12} + mvec4i{0,0,dx12,dx12} + c1;
		auto cb2 = mvec4i{0,dy23,0,dy23} + mvec4i{0,0,dx23,dx23} + c2;
		auto cb3 = mvec4i{0,dy31,0,dy31} + mvec4i{0,0,dx31,dx31} + c3;
		auto cb1dydx = mvec4i{dy12*2}; auto cb1dxdy = mvec4i{dx12*2};
		auto cb2dydx = mvec4i{dy23*2}; auto cb2dxdy = mvec4i{dx23*2};
		auto cb3dydx = mvec4i{dy31*2}; auto cb3dxdy = mvec4i{dx31*2};

		mvec4f scale{1.0f / (c1 + c2 + c3)};

		const int last_row = targetHeightInPx_ - 1;

		program_.Begin(minx, miny);
		for (int y=miny; y<endy; y+=2, cb1+=cb1dxdy, cb2+=cb2dxdy, cb3+=cb3dxdy, program_.CR()) {
			auto cx1{cb1}, cx2{cb2}, cx3{cb3};
			for (int x=minx; x<endx; x+=2, cx1+=cb1dydx, cx2+=cb2dydx, cx3+=cb3dydx, program_.Right2()) {
				mvec4i edges{cx1|cx2|cx3};
				if (movemask(bits2float(edges)) == 0xf) continue;
				const mvec4i trimask(rmlv::sar<31>(edges));

				// lower-left-origin opengl screen coords
				const qfloat2 frag_coord = { mvec4f(x+0.5f)+rglv::FQX, mvec4f(last_row-y+0.5f)+rglv::FQYR };

				rglv::BaryCoord bary;
				bary.x = itof(cx2) * scale;
				bary.z = itof(cx1) * scale;
				bary.y = mvec4f{1.0F} - bary.x - bary.z;

				program_.render(frag_coord, trimask, bary, frontfacing); }}}

private:
	FRAGMENT_PROCESSOR& program_;
	const rmlg::irect rect_;
	const int targetHeightInPx_; };


}  // namespace rglv
}  // namespace rqdq
