#pragma once
#include "src/rml/rmlv/rmlv_vec.hxx"

#include "3rdparty/gason/gason.h"

#include <optional>
#include <variant>
#include <string>


namespace rqdq {
namespace rclx {

std::optional<JsonValue> jv_find(const JsonValue& data, const std::string& key, int tag = -1);
std::optional<rmlv::vec3> jv_decode_vec3(const JsonValue& data);
std::variant<std::monostate, std::string, rmlv::vec3> jv_decode_ref_or_vec3(const JsonValue& data);

}  // close package namespace
}  // close enterprise namespace
