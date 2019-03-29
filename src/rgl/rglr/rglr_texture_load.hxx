#pragma once
#include <string>

#include "src/rgl/rglr/rglr_texture.hxx"

namespace rqdq {
namespace rglr {

Texture load_png(const std::string& filename, const std::string& name, bool premultiply);
Texture load_any(const std::string& prefix, const std::string& fn, const std::string& name, bool premultiply);


}  // namespace rglr
}  // namespace rqdq
