#pragma once
#include "src/rgl/rglr/rglr_canvas.hxx"

namespace rqdq {
namespace rglr {

namespace Blur {

struct Kawase {
	void blur_copy(int y0,
	               int y1,
	               int dist,
	               const FloatingPointCanvas& src,
	               FloatingPointCanvas& dst);
	};


}  // namespace Blur


}  // namespace rglr
}  // namespace rqdq
