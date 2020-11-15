#pragma once
#include <string>

#include "src/rgl/rglr/rglr_texture.hxx"

namespace rqdq {
namespace rglr {

auto LoadPNG(const std::pmr::string& filename, std::string_view name, const bool premultiply) -> Texture;
auto LoadPNG(const std::pmr::string& filename, std::string name, bool premultiply) -> Texture;
auto LoadPNG(const char* filename, std::string name, bool premultiply) -> Texture;
// auto LoadAny(const std::string& prefix, const std::string& fn, const std::string& name, bool premultiply) -> Texture;


}  // namespace rglr
}  // namespace rqdq
