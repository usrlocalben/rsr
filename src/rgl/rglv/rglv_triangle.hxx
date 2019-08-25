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
		using rmlv::mvec4f, rmlv::mvec4i, rmlv::sar, rmlv::shr, rmlv::shl, rmlv::vmin, rmlv::vmax;

		const int q = 2; // block size is 2x2

		//const mvec4f fixScale{ 16.0F };
		auto x1 = ftoi_round(dc1.x);
		auto x2 = ftoi_round(dc2.x);
		auto x3 = ftoi_round(dc3.x);
		auto y1 = ftoi_round(dc1.y);
		auto y2 = ftoi_round(dc2.y);
		auto y3 = ftoi_round(dc3.y);

		mvec4i Vminx = vmax(rmlv::sar<4>(vmin(vmin(x1,x2),x3) + 0xf), rect_.left.x) & mvec4i{~q-1};
		mvec4i Vendx = vmin(rmlv::sar<4>(vmax(vmax(x1,x2),x3) + 0xf), rect_.right.x);
		mvec4i Vminy = vmax(rmlv::sar<4>(vmin(vmin(y1,y2),y3) + 0xf), rect_.top.y)  & mvec4i{~q-1};
		mvec4i Vendy = vmin(rmlv::sar<4>(vmax(vmax(y1,y2),y3) + 0xf), rect_.bottom.y);

		auto Vdx12 = x1 - x2, Vdy12 = y2 - y1;
		auto Vdx23 = x2 - x3, Vdy23 = y3 - y2;
		auto Vdx31 = x3 - x1, Vdy31 = y1 - y3;
		auto Vc1 = Vdy12*(shl<4>(Vminx) - x1) + Vdx12*(shl<4>(Vminy)-y1);
		auto Vc2 = Vdy23*(shl<4>(Vminx) - x2) + Vdx23*(shl<4>(Vminy)-y2);
		auto Vc3 = Vdy31*(shl<4>(Vminx) - x3) + Vdx31*(shl<4>(Vminy)-y3);
		const auto zero = _mm_setzero_si128();
		const auto one = _mm_set1_epi32(1);
		Vc1 += shr<31>(cmpgt(Vdy12, zero) | (cmpeq(Vdy12,zero) & cmpgt(Vdx12, zero)));
		Vc2 += shr<31>(cmpgt(Vdy23, zero) | (cmpeq(Vdy23,zero) & cmpgt(Vdx23, zero)));
		Vc3 += shr<31>(cmpgt(Vdy31, zero) | (cmpeq(Vdy31,zero) & cmpgt(Vdx31, zero)));
		Vc1 = sar<4>(Vc1 - one);
		Vc2 = sar<4>(Vc2 - one);
		Vc3 = sar<4>(Vc3 - one);

		const auto Vscale = rmlv::oneover(itof(Vc1) + itof(Vc2) + itof(Vc3));

		for (int li=0; li<laneCnt; li++) {
			int dx12 = Vdx12.si[li];
			int dx23 = Vdx23.si[li];
			int dx31 = Vdx31.si[li];
			int dy12 = Vdy12.si[li];
			int dy23 = Vdy23.si[li];
			int dy31 = Vdy31.si[li];
			int c1 = Vc1.si[li];
			int c2 = Vc2.si[li];
			int c3 = Vc3.si[li];
			int miny = Vminy.si[li];
			int endy = Vendy.si[li];
			int minx = Vminx.si[li];
			int endx = Vendx.si[li];
			mvec4f scale{Vscale.lane[li]};
			bool frontfacing = !backfacing[li];

			auto cb1 = mvec4i{0,dy12,0,dy12} + mvec4i{0,0,dx12,dx12} + c1;
			auto cb2 = mvec4i{0,dy23,0,dy23} + mvec4i{0,0,dx23,dx23} + c2;
			auto cb3 = mvec4i{0,dy31,0,dy31} + mvec4i{0,0,dx31,dx31} + c3;

			mvec4i cb1dydx{dy12*2}; mvec4i cb1dxdy{dx12*2};
			mvec4i cb2dydx{dy23*2}; mvec4i cb2dxdy{dx23*2};
			mvec4i cb3dydx{dy31*2}; mvec4i cb3dxdy{dx31*2};

			program_.Begin(minx, miny, dc1, dc2, dc3, data1, data2, data3, li);
			for (int y=miny; y<endy; y+=2, cb1+=cb1dxdy, cb2+=cb2dxdy, cb3+=cb3dxdy, program_.CR()) {
				auto cx1{cb1}, cx2{cb2}, cx3{cb3};
				for (int x=minx; x<endx; x+=2, cx1+=cb1dydx, cx2+=cb2dydx, cx3+=cb3dydx, program_.Right2()) {
					mvec4i edges{cx1|cx2|cx3};
					if (movemask(bits2float(edges)) == 0xf) continue;
					const mvec4i trimask(sar<31>(edges));

					// lower-left-origin opengl screen coords
					const qfloat2 frag_coord = { mvec4f(x+0.5F)+rglv::FQX, mvec4f(targetHeightInPx_-y-0.5F)+rglv::FQYR };

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
