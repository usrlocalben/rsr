#include "rmlv_vec.hxx"

#include <ostream>

#include <fmt/format.h>

std::ostream& operator<<(std::ostream& os, rqdq::rmlv::vec2 v) {
	os << fmt::format("<vec2 {:.4f},{:.4f}>", v.x, v.y);
	return os; }


std::ostream& operator<<(std::ostream& os, rqdq::rmlv::vec3 v) {
	os << fmt::format("<vec3 {:.4f},{:.4f},{:.4f}>", v.x, v.y, v.z);
	return os; }


std::ostream& operator<<(std::ostream& os, rqdq::rmlv::vec4 v) {
	os << fmt::format("<vec4 {:.4f},{:.4f},{:.4f},{:.4f}>", v.x, v.y, v.z, v.w);
	return os; }


std::ostream& operator<<(std::ostream& os, rqdq::rmlv::ivec2 v) {
	os << "<ivec2 " << v.x << ", " << v.y << ">";
	return os; }
