#include "src/rgl/rglr/rglr_canvas_util.hxx"

#include <algorithm>

#include "src/rgl/rglr/rglr_canvas.hxx"
#include "src/rml/rmlg/rmlg_irect.hxx"

namespace rqdq {
namespace rglr {

rglr::TrueColorCanvas make_subcanvas(rglr::TrueColorCanvas& src, const rmlg::irect rect) {
	auto left = rect.left.x < 0 ? src.width() + rect.left.x : rect.left.x;
	auto top = rect.top.y < 0 ? src.height() + rect.top.y : rect.top.y;

	auto width = rect.width() < 0 ? src.width() + rect.width() : rect.width();
	auto height = rect.height() < 0 ? src.height() + rect.height() : rect.height();

	// clip to src bounds
	width = std::min(src.width() - left, width);
	height = std::min(src.height() - top, height);

	return rglr::TrueColorCanvas(src.data() + (top * src.stride()) + left, width, height, src.stride()); }


}  // namespace rglr
}  // namespace rqdq
