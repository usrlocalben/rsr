#pragma once
#include "src/rgl/rglv/rglv_mesh.hxx"

#include <string>

namespace rqdq {
namespace rglv {

auto LoadOBJ(const std::pmr::string&) -> Mesh;


}  // close package namespace
}  // clsoe enterprise namespace
