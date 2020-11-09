#pragma once
#include <optional>
#include <string>
#include <tuple>

namespace rqdq {
namespace rglv {

struct Mesh;
class MaterialStore;

auto LoadOBJ(const std::string& fn, std::optional<const std::string> dir=std::nullopt) -> Mesh;


}  // namespace rglv
}  // namespace rqdq
