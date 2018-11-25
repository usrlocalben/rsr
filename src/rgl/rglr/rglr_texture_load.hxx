#pragma once
#include <rglr_texture.hxx>

#include <string>

namespace rqdq {
namespace rglr {

Texture load_png(const std::string& filename, const std::string& name, bool premultiply);
Texture load_any(const std::string& prefix, const std::string& fn, const std::string& name, bool premultiply);

}  // close package namespace
}  // close enterprise namespace
