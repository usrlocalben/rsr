#pragma once
#include "src/rgl/rglr/rglr_canvas.hxx"

namespace rqdq {
namespace rglr {

namespace Blur {

struct Kawase {
	void blur_copy(const int y0,
	               const int y1,
	               const int dist,
	               const FloatingPointCanvas& src,
	               FloatingPointCanvas& dst);
	};


}  // namespace Blur


}  // namespace rglr
}  // namespace rqdq
