#pragma once
#include <string>
#include <tuple>

namespace rqdq {
namespace rglv {

struct Mesh;
class MaterialStore;

auto LoadOBJ(const std::string& prepend, const std::string& fn) -> Mesh;


}  // namespace rglv
}  // namespace rqdq
