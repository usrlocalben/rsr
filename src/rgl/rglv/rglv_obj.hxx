#pragma once
#include <string>
#include <tuple>

namespace rqdq {
namespace rglv {

struct Mesh;
struct MaterialStore;

std::tuple<Mesh, MaterialStore> loadOBJ(const std::string& prepend, const std::string& fn);


}  // namespace rglv
}  // namespace rqdq
