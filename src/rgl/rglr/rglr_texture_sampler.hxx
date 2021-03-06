#pragma once
#include <cassert>

#include "src/rml/rmlg/rmlg_pow2.hxx"
#include "src/rml/rmlv/rmlv_mvec4.hxx"
#include "src/rml/rmlv/rmlv_soa.hxx"

#include "3rdparty/pixeltoaster/PixelToaster.h"

namespace rqdq {
namespace rglr {

class TextureUnit {
public:
	TextureUnit() = default;
	virtual ~TextureUnit() = default;
	virtual void sample(const rmlv::qfloat2&, rmlv::qfloat4&) const = 0; };


auto MakeTextureUnit(const float*, int, int, int, bool, void*) -> const TextureUnit*;


/**
 * texture unit for e.g. shadow buffers:
 *
 * float buffer
 * power-of-2 size
 * FILTERING = NEAREST
 * WRAP_S = CLAMP_TO_BORDER
 * WRAP_T = CLAMP_TO_BORDER
 * static border color
 */
class DepthTextureUnit {
	const float* const buf_;
	const rmlv::mvec4i dim_;
	const rmlv::mvec4f dimf_;
	const rmlv::mvec4i mask_;

public:
	DepthTextureUnit(const float* buf, int dim);

	void sample(rmlv::qfloat2 coord, rmlv::qfloat& out) const; };


//=============================================================================
//							INLINE DEFINITIONS
//=============================================================================

						// ----------------------
						// class DepthTextureUnit
						// ----------------------

inline
DepthTextureUnit::DepthTextureUnit(const float* buf, int dim) :
		buf_(buf),
		dim_(dim),
		dimf_(static_cast<float>(dim)),
		mask_(dim*dim-1) {}

inline
void DepthTextureUnit::sample(rmlv::qfloat2 coord, rmlv::qfloat& out) const {
	using rmlv::mvec4f, rmlv::mvec4i, rmlv::ftoi, rmlv::shl, rmlv::qfloat4;

	const auto _0_ = _mm_setzero_ps();
	const auto _1_ = rmlv::mvec4f(1.0F);
	const auto borderColor = rmlv::mvec4f(-1.0F);

	auto xHit = cmpge(coord.x, _0_) & cmplt(coord.x, _1_);
	auto yHit = cmpge(coord.y, _0_) & cmplt(coord.y, _1_);
	auto hit = xHit & yHit;

	auto px = ftoi(     coord.x  * dimf_);
	auto py = ftoi((_1_-coord.y) * dimf_);

	auto ofs = py * dim_ + px;
	ofs = ofs & mask_;

	auto color = load_lut(buf_, ofs);
	out = rmlv::SelectFloat(borderColor, color, hit); }


}  // namespace rglr
}  // namespace rqdq
