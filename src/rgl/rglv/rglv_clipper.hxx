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


std::string plane_name(const Plane plane);


class Clipper {
public:
	Clipper() {}

	inline unsigned clip_point(const rmlv::vec4& p) const {
		const int left = (p.w + p.x < 0) << 0;
		const int bottom = (p.w + p.y < 0) << 1;
		const int near = (p.w + p.z < 0) << 2;

		const int right = (p.w - p.x < 0) << 3;
		const int top = (p.w - p.y < 0) << 4;
		//auto far = (p.w - p.z < 0) << 5;
		const unsigned flags = left | bottom | near | right | top;  //| far
		return flags; }

	inline rmlv::mvec4i clip_point(const rmlv::qfloat4 p) const {
		const auto zero = rmlv::mvec4f::zero();
		const auto w = p.w * factor;

		auto left   =              rmlv::shr<31>(rmlv::float2bits(cmple(w + p.x, zero)));
		auto bottom = rmlv::shl<1>(rmlv::shr<31>(rmlv::float2bits(cmple(w + p.y, zero))));
		auto near   = rmlv::shl<2>(rmlv::shr<31>(rmlv::float2bits(cmple(w + p.z, zero))));

		auto right  = rmlv::shl<3>(rmlv::shr<31>(rmlv::float2bits(cmple(w - p.x, zero))));
		auto top    = rmlv::shl<4>(rmlv::shr<31>(rmlv::float2bits(cmple(w - p.y, zero))));
		//auto far    = shl<5>(shr<31>(float2bits(cmple(w - p.z, zero))));

		auto flags = left | bottom | near | right | top;  // | far
		return flags; }

	inline bool is_inside(const Plane plane, const rmlv::vec4& p) const {
		const float wscaled = p.w * factor;
		switch (plane) {
		case Plane::Left:   return wscaled + p.x >= 0;
		case Plane::Right:  return wscaled - p.x >= 0;
		case Plane::Bottom: return wscaled + p.y >= 0;
		case Plane::Top:    return wscaled - p.y >= 0;
		case Plane::Near:   return p.w + p.z >= 0;
		case Plane::Far:    return p.w - p.z >= 0;
		default: assert(false); return true; } }

	inline float clip_line(const Plane plane, const rmlv::vec4& a, const rmlv::vec4& b) const {
		const float aws = a.w * factor;
		const float bws = b.w * factor;
		switch (plane) {
		case Plane::Left:   return (aws + a.x) / ((aws + a.x) - (bws + b.x));
		case Plane::Right:  return (aws - a.x) / ((aws - a.x) - (bws - b.x));
		case Plane::Bottom: return (aws + a.y) / ((aws + a.y) - (bws + b.y));
		case Plane::Top:    return (aws - a.y) / ((aws - a.y) - (bws - b.y));
		case Plane::Near:   return (a.w + a.z) / ((a.w + a.z) - (b.w + b.z));
		case Plane::Far:    return (a.w - a.z) / ((a.w - a.z) - (b.w - b.z));
		default: assert(false); return 0.0f; } }

	/*
	inline unsigned clip_point(const rmlv::mvec4f& p) {
		const auto zero = rmlv::mvec4f::zero();
		auto www = GUARDBAND_WWWW * p.wwww();  // guardband vector is (gbf,gbf,1,0)
		auto lbn_mask = movemask(cmple(www+p, zero)) & 0x7;  // left, bottom, near
		auto rtf_mask = movemask(cmple(www-p, zero)) & 0x3;  // right, top, far
		return lbn_mask | (rtf_mask << 3); }
	*/

private:
	const float factor{ 4.0f };
	// const rmlv::mvec4f GUARDBAND_WWWW{GUARDBAND_FACTOR, GUARDBAND_FACTOR, 1.0f, 0.0f};
	};


}  // namespace rglv
}  // namespace rqdq
