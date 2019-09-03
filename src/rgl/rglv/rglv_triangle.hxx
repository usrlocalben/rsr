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
	static constexpr int q = 8;  // level 0 block is 8x8

public:
	TriangleRasterizer(int targetHeightInPx, FRAGMENT_PROCESSOR& fp, rmlg::irect rect) :
		targetHeightInPx_(targetHeightInPx),
		program_(fp),
		rect_(rect) {}

	/*
	void Draw(rmlv::vec4 s1, rmlv::vec4 s2, rmlv::vec4 s3, const bool frontfacing) {
		int x1 = int(16.0f * (s1.x - 0.5f));
		int x2 = int(16.0f * (s2.x - 0.5f));
		int x3 = int(16.0f * (s3.x - 0.5f));
		int y1 = int(16.0f * (s1.y - 0.5f));
		int y2 = int(16.0f * (s2.y - 0.5f));
		int y3 = int(16.0f * (s3.y - 0.5f));
		Draw(x1, x2, x3, y1, y2, y3, frontfacing); }*/

	void Draw(int x1, int x2, int x3,
	          int y1, int y2, int y3,
	          float z1, float z2, float z3) {
		using std::max, std::min;
		using rmlv::qfloat, rmlv::qfloat2, rmlv::qfloat3;
		using rmlv::mvec4f, rmlv::mvec4i;

			minX_= max((min(min(x1,x2),x3) + 0xf) >> 4, rect_.left.x);
			endX_= min((max(max(x1,x2),x3) + 0xf) >> 4, rect_.right.x);
		int minY = max((min(min(y1,y2),y3) + 0xf) >> 4, rect_.top.y);
		int endY = min((max(max(y1,y2),y3) + 0xf) >> 4, rect_.bottom.y);

		minX_ &= ~(q - 1); // align to block
		minY &= ~(q - 1);

		dx12_ = x1 - x2;  dy12_ = y2 - y1;
		dx23_ = x2 - x3;  dy23_ = y3 - y2;
		dx31_ = x3 - x1;  dy31_ = y1 - y3;

		int c1 = dy12_*((minX_<<4) - x1) + dx12_*((minY<<4)-y1);
		int c2 = dy23_*((minX_<<4) - x2) + dx23_*((minY<<4)-y2);
		int c3 = dy31_*((minX_<<4) - x3) + dx31_*((minY<<4)-y3);
		if (dy12_ > 0 || (dy12_==0 && dx12_>0)) c1++;
		if (dy23_ > 0 || (dy23_==0 && dx23_>0)) c2++;
		if (dy31_ > 0 || (dy31_==0 && dx31_>0)) c3++;
		c1 = (c1 - 1) >> 4;
		c2 = (c2 - 1) >> 4;
		c3 = (c3 - 1) >> 4;

		int dz12 = z1 - z2, dz31 = z3 - z1;
		float invDet = 1.0F / (dx12_*dy31_ - dx31_*dy12_);
		float fdzdx = invDet * (dz12*dy31_ - dz31*dy12_);
		float fdzdy = invDet * (dz12*dx31_ - dz31*dx12_);
		int cz = z1 + ((minX_<<4)-x1)*fdzdx + ((minY<<4)-y1)*fdzdy;
		dzdx_ = fdzdx * 16.0F;
		dzdy_ = fdzdy * 16.0F;

		// block limits
		const int qm1 = q - 1;
		nmin1_ = 0; nmin2_ = 0; nmin3_ = 0; nminz_ = 0;
		nmax1_ = 0; nmax2_ = 0; nmax3_ = 0; nmaxz_ = 0;
		if (dx12_ >= 0) nmax1_ -= qm1*dx12_; else nmin1_ -= qm1*dx12_;
		if (dy12_ >= 0) nmax1_ -= qm1*dy12_; else nmin1_ -= qm1*dy12_;
		if (dx23_ >= 0) nmax2_ -= qm1*dx23_; else nmin2_ -= qm1*dx23_;
		if (dy23_ >= 0) nmax2_ -= qm1*dy23_; else nmin2_ -= qm1*dy23_;
		if (dx31_ >= 0) nmax3_ -= qm1*dx31_; else nmin3_ -= qm1*dx31_;
		if (dy31_ >= 0) nmax3_ -= qm1*dy31_; else nmin3_ -= qm1*dy31_;
		if (dzdx_ >= 0) nmaxz_ += qm1*dzdx_; else nmin1_ += qm1*dzdx_;
		if (dzdy_ >= 0) nmaxz_ += qm1*dzdy_; else nmin1_ += qm1*dzdy_;

		cb1_ = c1; cb2_ = c2; cb3_ = c3; cbz_ = cz;
		qb1_ = mvec4i{0,dy12_,0,dy12_} + mvec4i{0,0,dx12_,dx12_} + c1;
		qb2_ = mvec4i{0,dy23_,0,dy23_} + mvec4i{0,0,dx23_,dx23_} + c2;
		qb3_ = mvec4i{0,dy31_,0,dy31_} + mvec4i{0,0,dx31_,dx31_} + c3;
		qbz_ = mvec4i{0,dzdx_,0,dzdx_} + mvec4i{0,0,dzdy_,dzdy_} + cz;
		qdx12_ = mvec4i{dx12_*2};  qdx23_ = mvec4i{dx23_*2};  qdx31_ = mvec4i{dx31_*2};  qdzdy_ = mvec4i{dzdy_*2};
		qdy12_ = mvec4i{dy12_*2};  qdy23_ = mvec4i{dy23_*2};  qdy31_ = mvec4i{dy31_*2};  qdzdx_ = mvec4i{dzdx_*2};

		qstep_ = -q;
		e1x_ = qstep_ * dy12_;  e2x_ = qstep_ * dy23_;  e3x_ = qstep_ * dy31_;  ezx_ = qstep_ * dzdx_;
		qe1x_= e1x_;            qe2x_= e2x_;            qe3x_= e3x_;            qezx_ = ezx_;
		scale_ = 1.0f / (c1 + c2 + c3);

		x0_ = minX_;
		for (y0_=minY; y0_<endY; y0_+=q) {
			AdvanceBlockInXUntilDead();
			FlipBlockXDirection();
			RenderBlocksUntilDead();
			NextRow(); }}

private:
	inline bool BlockIsTouchingTriangle() const {
		return cb1_>=nmax1_ && cb2_>=nmax2_ && cb3_>=nmax3_; }
	inline bool BlockIsInsideTriangle() const {
		return cb1_>=nmin1_ && cb2_>=nmin2_ && cb3_>=nmin3_; }
	inline bool BlockIsInBounds() const {
		return minX_ <= x0_ && x0_ <= endX_; }

	inline void AdvanceBlockInXUntilDead() {
		while (BlockIsInBounds() && BlockIsTouchingTriangle()) {
			AdvanceBlockInX(); }}

	inline void RenderBlocksUntilDead() {
		while (1) {
			AdvanceBlockInX();

			if (!BlockIsInBounds()) break;
			if (cb1_ < nmax1_) { if (e1x_ < 0) { break; } else { continue; }}
			if (cb2_ < nmax2_) { if (e2x_ < 0) { break; } else { continue; }}
			if (cb3_ < nmax3_) { if (e3x_ < 0) { break; } else { continue; }}

			// if (!program_.BeginBlock(x0_, y0_/*, cbz_+nminz_*/)) continue;
			program_.BeginBlock(x0_, y0_);
			if (BlockIsInsideTriangle()) {
				RenderCompleteBlock(); }
			else {
				RenderPartialBlock(); }
			program_.EndBlock(); }}

	inline void AdvanceBlockInX() {
		x0_ += qstep_;
		cb1_+=e1x_;   cb2_+=e2x_;   cb3_+=e3x_;   cbz_+=ezx_;
		qb1_+=qe1x_;  qb2_+=qe2x_;  qb3_+=qe3x_;  qbz_+=qezx_; }

	inline void NextRow() {
		using rmlv::mvec4i;
		cb1_+=q*dx12_;          cb2_+=q*dx23_;          cb3_+=q*dx31_;          cbz_+=q*dzdy_;
		qb1_+=mvec4i{dx12_*q};  qb2_+=mvec4i{dx23_*q};  qb3_+=mvec4i{dx31_*q};  qbz_+=mvec4i{dzdy_*q}; }

	inline void FlipBlockXDirection() {
		qstep_ = -qstep_;
		e1x_=-e1x_;   e2x_=-e2x_;    e3x_=-e3x_;    ezx_=-ezx_;
		qe1x_=0-qe1x_; qe2x_=0-qe2x_;  qe3x_=0-qe3x_;  qezx_=0-qezx_; }

	inline void RenderCompleteBlock() {
		using rmlv::qfloat, rmlv::qfloat2, rmlv::mvec4f, rmlv::mvec4i;
		// program_.update_max_z(cbz_ + nmaxz_);
		auto qcy1 = qb1_;
		auto qcy2 = qb2_;
		auto qcyz = qbz_;
		for (int iy=0; iy<q; iy+=2, qcy1+=qdx12_, qcy2+=qdx23_, qcyz+=qdzdy_, program_.CR2()) {
			auto qcx1 = qcy1;
			auto qcx2 = qcy2;
			auto qcxz = qcyz;
			for (int ix=0; ix<q; ix+=2, qcx1+=qdy12_, qcx2+=qdy23_, qcxz+=qdzdx_, program_.Right2()) {
				// XXX const qfloat2 frag_coord = { mvec4f(x+0.5f)+rglv::FQX, mvec4f(last_row-y+0.5f)+rglv::FQYR };
				qfloat b0 = itof(qcx2) * scale_;
				qfloat b2 = itof(qcx1) * scale_;
				qfloat b1 = mvec4f{1.0f} - b0 - b2;
				BaryCoord bary{ b0, b1, b2 };
				program_.Render(qfloat2{0,0}, bary/*, qcxz*/); }}}

	inline void RenderPartialBlock() {
		using rmlv::qfloat, rmlv::qfloat2, rmlv::mvec4f, rmlv::mvec4i, rmlv::sar;
		auto qcy1 = qb1_;
		auto qcy2 = qb2_;
		auto qcy3 = qb3_;
		auto qcyz = qbz_;
		for (int iy=0; iy<q; iy+=2, qcy1+=qdx12_, qcy2+=qdx23_, qcy3+=qdx31_, qcyz+=qdzdy_, program_.CR2()) {
			auto qcx1 = qcy1;
			auto qcx2 = qcy2;
			auto qcx3 = qcy3;
			auto qcxz = qcyz;
			for (int ix=0; ix<q; ix+=2, qcx1+=qdy12_, qcx2+=qdy23_, qcx3+=qdy31_, qcxz+=qdzdx_, program_.Right2()) {
				mvec4i edges{qcx1|qcx2|qcx3};
				if (movemask(bits2float(edges)) == 0xf) continue;
				mvec4i triMask{sar<31>(edges)};

				// XXX frag_coord
				//
				qfloat b0 = itof(qcx2) * scale_;
				qfloat b2 = itof(qcx1) * scale_;
				qfloat b1 = mvec4f{1.0f} - b0 - b2;
				BaryCoord bary{ b0, b1, b2 };
				//BaryCoord bary{ 0.333F, 0.333F, 0.333F };
				program_.Render(qfloat2{0, 0}, bary/*, qcxz*/, triMask); }}}

private:
	// global state
	const int targetHeightInPx_;

	// draw-call uniform state
	FRAGMENT_PROCESSOR& program_;

	// thread
	const rmlg::irect rect_;

	// fragment
	int minX_, endX_, qstep_, x0_, y0_;
	int    dx12_,  dx23_,  dx31_,  dzdy_;
	int    dy12_,  dy23_,  dy31_,  dzdx_;
	int    nmin1_, nmin2_, nmin3_, nminz_;
	int    nmax1_, nmax2_, nmax3_, nmaxz_;
	int    cb1_,   cb2_,   cb3_,   cbz_;
	int    e1x_,   e2x_,   e3x_,   ezx_;
	rmlv::mvec4i qb1_,   qb2_,   qb3_,   qbz_;
	rmlv::mvec4f scale_;
	rmlv::mvec4i qdx12_, qdx23_, qdx31_, qdzdy_;
	rmlv::mvec4i qdy12_, qdy23_, qdy31_, qdzdx_;
	rmlv::mvec4i qe1x_,  qe2x_,  qe3x_,  qezx_;
	};


}  // namespace rglv
}  // namespace rqdq
