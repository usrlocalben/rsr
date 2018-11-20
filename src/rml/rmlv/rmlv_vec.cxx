#include <rmlv_vec.hxx>

#include <ostream>
#include <fmt/format.h>


std::ostream& operator<<(std::ostream& os, const rqdq::rmlv::vec4& v) {
	os << fmt::format("<vec4 {:.4f},{:.4f},{:.4f},{:.4f}>", v.x, v.y, v.z, v.w);
	return os; }


std::ostream& operator<<(std::ostream& os, const rqdq::rmlv::vec3& v) {
	auto str = fmt::format("<vec3 {:.4f},{:.4f},{:.4f}>", v.x, v.y, v.z);
	os << str;
	return os; }


std::ostream& operator<<(std::ostream& os, const rqdq::rmlv::ivec2& v) {
	os << "<ivec2 " << v.x << ", " << v.y << ">";
	return os; }
