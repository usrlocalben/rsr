#pragma once
#include "src/rml/rmlv/rmlv_vec.hxx"

#include <optional>
#include <variant>
#include <string>
#include <string_view>

#include "3rdparty/gason/gason.h"

namespace rqdq {
namespace rclx {

auto jv_find(const JsonValue& data, std::string_view key, int tag = -1) -> std::optional<JsonValue>;
auto jv_decode_vec3(const JsonValue& data) -> std::optional<rmlv::vec3>;
// std::variant<std::monostate, std::string, rmlv::vec3> jv_decode_ref_or_vec3(const JsonValue& data);


}  // namespace rclx
}  // namespace rqdq
