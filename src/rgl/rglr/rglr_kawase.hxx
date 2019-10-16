#pragma once
#include "src/rgl/rglr/rglr_canvas.hxx"

namespace rqdq {
namespace rglr {

void KawaseBlurFilter(const FloatingPointCanvas& src,
                      FloatingPointCanvas& dst,
                      int dist, int yBegin, int yEnd);


}  // namespace rglr
}  // namespace rqdq
