#include "i_value.hxx"

#include <iostream>
#include <stdexcept>
#include <string_view>

namespace rqdq {
namespace rqv {

ValueType ValueTypeSerializer::Deserialize(std::string_view data) {
    if (data == "integer") { return ValueType::Integer; }
	if (data == "real")    { return ValueType::Real; }
	if (data == "vec2")    { return ValueType::Vec2; }
	if (data == "vec3")    { return ValueType::Vec3; }
	if (data == "vec4")    { return ValueType::Vec4; }
	std::cerr << "invalid value type \"" << data << "\", using real" << std::endl;
	return ValueType::Real; }


std::string_view ValueTypeSerializer::Serialize(ValueType item) {
	switch (item) {
	case ValueType::Integer: return "integer";
	case ValueType::Real:    return "real";
	case ValueType::Vec2:    return "vec2";
	case ValueType::Vec3:    return "vec3";
	case ValueType::Vec4:    return "vec4";
	default: throw std::runtime_error("refusing to serialize invalid ValueType"); }}


}  // namespace rqv
}  // namespace rqdq
