#pragma once
#include <array>
#include <cstdint>
#include <string>

#include "src/rcl/rcls/rcls_aligned_containers.hxx"
#include "src/rgl/rglv/rglv_mesh.hxx"
#include "src/rgl/rglv/rglv_vao.hxx"

namespace rqdq {
namespace rglv {

void MakeArray(const Mesh& m, const std::string& spec, VertexArray_F3F3& buffer, rcls::vector<uint16_t>& idx);
void MakeArray(const Mesh& m, const std::string& spec, VertexArray_F3F3F3& buffer, rcls::vector<uint16_t>& idx);


}  // close package namespace
}  // close enterprise namespace
