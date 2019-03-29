#pragma once
#include "src/rml/rmlv/rmlv_soa.hxx"
#include "src/rml/rmlv/rmlv_mvec4.hxx"

namespace rqdq {
namespace rglr {

namespace BlendProgram {
#define BLEND_ARGS \
    const rmlv::mvec4i &mask, \
	rmlv::qfloat4& frag_color, \
	rmlv::qfloat4* const __restrict target


struct Set {
	static void blendFragment(BLEND_ARGS) {
		target->r = selectbits(target->r, frag_color.r, mask);
		target->g = selectbits(target->g, frag_color.g, mask);
		target->b = selectbits(target->b, frag_color.b, mask); } };


/*
struct UniformAlpha {
	UniformAlpha(float alpha)
		:alpha(alpha) {}

	void operator()(BLEND_ARGS) const {
		target->r = lerp_premul(target->r, frag_color.r, selectbits(rmlv::mvec4f{ 0 }, alpha, mask));
		target->g = lerp_premul(target->g, frag_color.g, selectbits(rmlv::mvec4f{ 0 }, alpha, mask));
		target->b = lerp_premul(target->b, frag_color.b, selectbits(rmlv::mvec4f{ 0 }, alpha, mask)); }

	rmlv::mvec4f alpha;
	};
*/

#undef BLEND_ARGS


}  // namespace BlendProgram


}  // namespace rglr
}  // namespace rqdq
