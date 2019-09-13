#include "src/rgl/rglv/rglv_view_frustum.hxx"

#include <string>

namespace rqdq {
namespace rglv {

std::string plane_name(const Plane plane) {
	switch (plane) {
	case Plane::Left:   return "left";
	case Plane::Right:  return "right";
	case Plane::Bottom: return "bottom";
	case Plane::Top:    return "top";
	case Plane::Near:   return "near";
	case Plane::Far:    return "far";
	default:
		assert(false);
		return "invalid"; }}


}  // namespace rglv
}  // namespace rqdq
