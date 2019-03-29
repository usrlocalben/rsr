#pragma once

namespace rqdq {
namespace rglr {

struct DisplayMode {
	int width_in_pixels;
	int height_in_pixels;
	};


}  // namespace rglr

inline bool operator==(const rglr::DisplayMode& lhs, const rglr::DisplayMode& rhs) {
	return lhs.width_in_pixels == rhs.width_in_pixels &&
	       lhs.height_in_pixels == rhs.height_in_pixels; }


inline bool operator!=(const rglr::DisplayMode& lhs, const rglr::DisplayMode& rhs) {
	return !operator==(lhs, rhs); }


}  // namespace rqdq
