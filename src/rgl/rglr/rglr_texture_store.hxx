#pragma once
#include <string>
#include <string_view>
#include <vector>

#include "src/rgl/rglr/rglr_texture.hxx"

namespace rqdq {
namespace rglr {

class TextureStore {
	std::vector<Texture> store;

public:
	TextureStore();
	//      const Texture& get(string const key);
	void Append(Texture t);
	auto Find(std::string_view name) const -> const Texture*;
	void LoadDir(std::string_view dir);
	void LoadPNG(const std::pmr::string& path, std::string_view name);
	void Print(); };


}  // close package namespace
}  // close enterprise namespace
