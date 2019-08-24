#pragma once
#include <cassert>
#include <string>

#include "src/rcl/rcls/rcls_aligned_containers.hxx"
#include "src/rml/rmlv/rmlv_soa.hxx"
#include "src/rml/rmlv/rmlv_mvec4.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"

namespace rqdq {
namespace rglv {

enum class Plane {
	Left = 1 << 0,
	Bottom = 1 << 1,
	Near = 1 << 2,
	Right = 1 << 3,
	Top = 1 << 4,
	Far = 1 << 5
	};

constexpr std::array<Plane, 5> clipping_panes = {
	Plane::Left,
	Plane::Bottom,
	Plane::Near,
	Plane::Right,
	Plane::Top,
	};


std::string plane_name(Plane plane);


inline float CalcGuardBandFactor(rmlv::ivec2 dim) {
	auto longSide = std::max(dim.x, dim.y);
	auto half = longSide / 2;
	return (2048.0F - half) / half; }


class Clipper {
public:
	Clipper(rmlv::ivec2 targetDimensionsInPixels) :
		factor_(CalcGuardBandFactor(targetDimensionsInPixels)),
		vFactor_(factor_) {}

	inline unsigned Test(const rmlv::vec4& p) const {
		const auto w = p.w * factor_;
		const int left   = static_cast<int>(  w + p.x < 0) << 0;
		const int bottom = static_cast<int>(  w + p.y < 0) << 1;
		const int near   = static_cast<int>(p.w + p.z < 0) << 2;

		const int right  = static_cast<int>(  w - p.x < 0) << 3;
		const int top    = static_cast<int>(  w - p.y < 0) << 4;
		//auto far = (p.w - p.z < 0) << 5;
		const unsigned flags = left | bottom | near | right | top;  //| far
		return flags; }

	inline rmlv::mvec4i Test(const rmlv::qfloat4 p) const {
		const auto zero = rmlv::mvec4f::zero();
		const auto w = p.w * vFactor_;

		auto left   =              rmlv::shr<31>(rmlv::float2bits(cmple(w + p.x, zero)));
		auto bottom = rmlv::shl<1>(rmlv::shr<31>(rmlv::float2bits(cmple(w + p.y, zero))));
		auto near   = rmlv::shl<2>(rmlv::shr<31>(rmlv::float2bits(cmple(p.w + p.z, zero))));

		auto right  = rmlv::shl<3>(rmlv::shr<31>(rmlv::float2bits(cmple(w - p.x, zero))));
		auto top    = rmlv::shl<4>(rmlv::shr<31>(rmlv::float2bits(cmple(w - p.y, zero))));
		//auto far    = shl<5>(shr<31>(float2bits(cmple(w - p.z, zero))));

		auto flags = left | bottom | near | right | top;  // | far
		return flags; }

	inline bool IsInside(const Plane plane, const rmlv::vec4& p) const {
		// const float wscaled = p.w * factor;
		switch (plane) {
		case Plane::Left:   return p.w + p.x >= 0;
		case Plane::Right:  return p.w - p.x >= 0;
		case Plane::Bottom: return p.w + p.y >= 0;
		case Plane::Top:    return p.w - p.y >= 0;
		case Plane::Near:   return p.w + p.z >= 0;
		case Plane::Far:    return p.w - p.z >= 0;
		default: assert(false); return true; } }

	inline float Clip(const Plane plane, const rmlv::vec4& a, const rmlv::vec4& b) const {
		switch (plane) {
		case Plane::Left:   return (a.w + a.x) / ((a.w + a.x) - (b.w + b.x));
		case Plane::Right:  return (a.w - a.x) / ((a.w - a.x) - (b.w - b.x));
		case Plane::Bottom: return (a.w + a.y) / ((a.w + a.y) - (b.w + b.y));
		case Plane::Top:    return (a.w - a.y) / ((a.w - a.y) - (b.w - b.y));
		case Plane::Near:   return (a.w + a.z) / ((a.w + a.z) - (b.w + b.z));
		case Plane::Far:    return (a.w - a.z) / ((a.w - a.z) - (b.w - b.z));
		default: assert(false); return 0.0F; } }

	/*
	inline unsigned clip_point(const rmlv::mvec4f& p) {
		const auto zero = rmlv::mvec4f::zero();
		auto www = GUARDBAND_WWWW * p.wwww();  // guardband vector is (gbf,gbf,1,0)
		auto lbn_mask = movemask(cmple(www+p, zero)) & 0x7;  // left, bottom, near
		auto rtf_mask = movemask(cmple(www-p, zero)) & 0x3;  // right, top, far
		return lbn_mask | (rtf_mask << 3); }
	*/

private:
	const float factor_;
	const rmlv::mvec4f vFactor_;
	// const rmlv::mvec4f GUARDBAND_WWWW{GUARDBAND_FACTOR, GUARDBAND_FACTOR, 1.0f, 0.0f};
	};


}  // namespace rglv
}  // namespace rqdq
