#pragma once
#include <rglr_texture.hxx>

#include <string>
#include <vector>

namespace rqdq {
namespace rglr {

class TextureStore {
public:
	TextureStore();
	//      const Texture& get(string const key);
	void append(Texture t);
	const Texture* const find_by_name(const std::string& name) const;
	void load_dir(const std::string& prepend);
	void load_any(const std::string& prepend, const std::string& fname);
	void print();

private:
	std::vector<Texture> store; };

}  // close package namespace
}  // close enterprise namespace