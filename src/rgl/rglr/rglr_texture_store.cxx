#include "src/rgl/rglr/rglr_texture_store.hxx"

#include <string>
#include <vector>

#include "src/rcl/rcls/rcls_file.hxx"
#include "src/rgl/rglr/rglr_texture_load.hxx"

#include <fmt/format.h>
#include <fmt/printf.h>

namespace rqdq {
namespace rglr {

Texture checkerboard2x2() {
	rcls::vector<PixelToaster::FloatingPointPixel> db;
	db.push_back(PixelToaster::FloatingPointPixel(0, 0, 0, 0));
	db.push_back(PixelToaster::FloatingPointPixel(1, 0, 1, 0));
	db.push_back(PixelToaster::FloatingPointPixel(1, 0, 1, 0));
	db.push_back(PixelToaster::FloatingPointPixel(0, 0, 0, 0));
	return{ db, 2, 2, 2, "checkerboard", -1, false }; }


TextureStore::TextureStore() {
	this->append(checkerboard2x2()); }


void TextureStore::append(Texture t) {
	store.push_back(t); }


const Texture * TextureStore::find_by_name(const std::string& name) const {
	for (auto& item : store) {
		if (item.name == name) {
			return &item; }}
	return nullptr; }


void TextureStore::load_any(const std::string& prepend, const std::string& fname) {
	auto existing = this->find_by_name(fname);
	if (existing != nullptr) {
		return; }

	Texture newtex = rglr::load_any(prepend, fname, fname, true);
	newtex.maybe_make_mipmap();
	this->append(newtex); }


void TextureStore::load_dir(const std::string& prepend) {
	static const std::vector<std::string> extensions{ "*.png" }; // , "*.jpg" };

	for (auto& ext : extensions) {
		for (auto& fn : rcls::FindGlob(prepend + ext)) {
			//std::cout << "scanning [" << prepend << "][" << fn << "]" << std::endl;
			this->load_any(prepend, fn); }}}


void TextureStore::print() {
	int i = 0;
	for (const auto& item : store) {
		const void * const ptr = item.buf.data();
		fmt::printf("#% 3d \"%-20s\" % 4d x% 4d", i, item.name, item.width, item.height);
		fmt::printf("  data@ 0x%p\n", ptr);
		i++; }}


}  // namespace rglr
}  // namespace rqdq
