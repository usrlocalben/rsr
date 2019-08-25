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

	/*
	void Draw(rmlv::vec4 s1, rmlv::vec4 s2, rmlv::vec4 s3, const bool frontfacing) {
		int x1 = int(16.0f * (s1.x - 0.5f));
		int x2 = int(16.0f * (s2.x - 0.5f));
		int x3 = int(16.0f * (s3.x - 0.5f));
		int y1 = int(16.0f * (s1.y - 0.5f));
		int y2 = int(16.0f * (s2.y - 0.5f));
		int y3 = int(16.0f * (s3.y - 0.5f));
		Draw(x1, x2, x3, y1, y2, y3, frontfacing); }*/

	template <typename DATA>
	void Draw(rmlv::qfloat4 dc1, rmlv::qfloat4 dc2, rmlv::qfloat4 dc3,
	          DATA data1, DATA data2, DATA data3,
			  const bool backfacing[4], int laneCnt) {
		using std::max, std::min;
		using rmlv::qfloat, rmlv::qfloat2, rmlv::qfloat3;
		using rmlv::mvec4f, rmlv::mvec4i, rmlv::shr, rmlv::shl, rmlv::vmin, rmlv::vmax;

		const int last_row = targetHeightInPx_ - 1;
		const int q = 2; // block size is 2x2

		auto x1 = ftoi_round(dc1.x * 16.0F);
		auto x2 = ftoi_round(dc2.x * 16.0F);
		auto x3 = ftoi_round(dc3.x * 16.0F);
		auto y1 = ftoi_round(dc1.y * 16.0F);
		auto y2 = ftoi_round(dc2.y * 16.0F);
		auto y3 = ftoi_round(dc3.y * 16.0F);

		mvec4i Vminx = vmax(rmlv::shr<4>(vmin(vmin(x1,x2),x3) + 0xf), rect_.left.x) & mvec4i{~q-1};
		mvec4i Vendx = vmin(rmlv::shr<4>(vmax(vmax(x1,x2),x3) + 0xf), rect_.right.x);
		mvec4i Vminy = vmax(rmlv::shr<4>(vmin(vmin(y1,y2),y3) + 0xf), rect_.top.y) & mvec4i{~q-1};
		mvec4i Vendy = vmin(rmlv::shr<4>(vmax(vmax(y1,y2),y3) + 0xf), rect_.bottom.y);

		auto Vdx12 = x1 - x2, Vdy12 = y2 - y1;
		auto Vdx23 = x2 - x3, Vdy23 = y3 - y2;
		auto Vdx31 = x3 - x1, Vdy31 = y1 - y3;
		auto Vc1 = Vdy12*(shl<4>(Vminx) - x1) + Vdx12*(shl<4>(Vminy)-y1);
		auto Vc2 = Vdy23*(shl<4>(Vminx) - x2) + Vdx23*(shl<4>(Vminy)-y2);
		auto Vc3 = Vdy31*(shl<4>(Vminx) - x3) + Vdx31*(shl<4>(Vminy)-y3);

		/*if (dy12 > 0 || (dy12==0 && dx12>0)) c1++;
		if (dy23 > 0 || (dy23==0 && dx23>0)) c2++;
		if (dy31 > 0 || (dy31==0 && dx31>0)) c3++;*/
		Vc1 += mvec4i{1} & (cmpgt(Vdy12, 0) | (cmpeq(Vdy12,0) & cmpgt(Vdx12, 0)));
		Vc2 += mvec4i{1} & (cmpgt(Vdy23, 0) | (cmpeq(Vdy23,0) & cmpgt(Vdx23, 0)));
		Vc3 += mvec4i{1} & (cmpgt(Vdy31, 0) | (cmpeq(Vdy31,0) & cmpgt(Vdx31, 0)));
		Vc1 = shr<4>(Vc1 - 1);
		Vc2 = shr<4>(Vc2 - 1);
		Vc3 = shr<4>(Vc3 - 1);

		mvec4f Vscale{mvec4f{1.0f} / (itof(Vc1) + itof(Vc2) + itof(Vc3))};

		for (int li=0; li<laneCnt; li++) {
			auto dx12 = Vdx12.si[li];
			auto dx23 = Vdx23.si[li];
			auto dx31 = Vdx31.si[li];
			auto dy12 = Vdy12.si[li];
			auto dy23 = Vdy23.si[li];
			auto dy31 = Vdy31.si[li];
			auto c1 = Vc1.si[li];
			auto c2 = Vc2.si[li];
			auto c3 = Vc3.si[li];
			auto miny = Vminy.si[li];
			auto endy = Vendy.si[li];
			auto minx = Vminx.si[li];
			auto endx = Vendx.si[li];
			mvec4f scale{Vscale.lane[li]};
			bool frontfacing = !backfacing[li];

			auto cb1 = mvec4i{0,dy12,0,dy12} + mvec4i{0,0,dx12,dx12} + c1;
			auto cb2 = mvec4i{0,dy23,0,dy23} + mvec4i{0,0,dx23,dx23} + c2;
			auto cb3 = mvec4i{0,dy31,0,dy31} + mvec4i{0,0,dx31,dx31} + c3;

			auto cb1dydx = mvec4i{dy12*2}; auto cb1dxdy = mvec4i{dx12*2};
			auto cb2dydx = mvec4i{dy23*2}; auto cb2dxdy = mvec4i{dx23*2};
			auto cb3dydx = mvec4i{dy31*2}; auto cb3dxdy = mvec4i{dx31*2};

			program_.Begin(minx, miny, dc1, dc2, dc3, data1, data2, data3, li);
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
					bary.y = 1.0F - bary.x - bary.z;

					program_.Render(frag_coord, trimask, bary, frontfacing); }}}}

private:
	FRAGMENT_PROCESSOR& program_;
	const rmlg::irect rect_;
	const int targetHeightInPx_; };


}  // namespace rglv
}  // namespace rqdq
