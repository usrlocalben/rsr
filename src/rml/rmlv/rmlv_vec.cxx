#include "rmlv_vec.hxx"

#include <ostream>

#include <fmt/format.h>


auto operator<<(std::ostream& os, rqdq::rmlv::vec2 v) -> std::ostream& {
	return os << fmt::format("<vec2 {:.4f},{:.4f}>", v.x, v.y); }


auto operator<<(std::ostream& os, rqdq::rmlv::vec3 v) -> std::ostream& {
	return os << fmt::format("<vec3 {:.4f},{:.4f},{:.4f}>", v.x, v.y, v.z); }


auto operator<<(std::ostream& os, rqdq::rmlv::vec4 v) -> std::ostream& {
	return os << fmt::format("<vec4 {:.4f},{:.4f},{:.4f},{:.4f}>", v.x, v.y, v.z, v.w); }


auto operator<<(std::ostream& os, rqdq::rmlv::ivec2 v) -> std::ostream& {
	return os << "<ivec2 " << v.x << ", " << v.y << ">"; }
