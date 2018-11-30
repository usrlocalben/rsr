#include "src/viewer/rqv_node_value.hxx"

#include <memory>

namespace rqdq {
namespace rqv {

ValueType name_to_type(const std::string& name) {
	if (name == "integer") {
		return ValueType::Integer; }
	else if (name == "real") {
		return ValueType::Real; }
	else if (name == "vec2") {
		return ValueType::Vec2; }
	else if (name == "vec3") {
		return ValueType::Vec3; }
	else if (name == "vec4") {
		return ValueType::Vec4; }
	else {
		std::cout << "invalid value type \"" << name << "\", using real" << std::endl;
		return ValueType::Real; }}


}  // close package namespace
}  // close enterprise namespace
