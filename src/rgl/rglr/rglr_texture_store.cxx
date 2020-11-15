#include "src/rgl/rglr/rglr_texture_store.hxx"

#include "src/rcl/rclt/rclt_util.hxx"
#include "src/rcl/rcls/rcls_file.hxx"
#include "src/rgl/rglr/rglr_texture_load.hxx"

#include <algorithm>
#include <memory_resource>
#include <string>
#include <vector>

#include <fmt/format.h>
#include <fmt/printf.h>

namespace rqdq {
namespace {

template <typename T1, typename T2>
auto EndsWithI(const T1& suffix, const T2& text) -> bool {
	return equal(rbegin(suffix), rend(suffix), rbegin(text),
	             [](char a, char b) { return tolower(a) == tolower(b); }); }


}  // close unnamed namespace
namespace rglr {

Texture checkerboard2x2() {
	rcls::vector<PixelToaster::FloatingPointPixel> db;
	db.push_back(PixelToaster::FloatingPointPixel(0, 0, 0, 0));
	db.push_back(PixelToaster::FloatingPointPixel(1, 0, 1, 0));
	db.push_back(PixelToaster::FloatingPointPixel(1, 0, 1, 0));
	db.push_back(PixelToaster::FloatingPointPixel(0, 0, 0, 0));
	return{ db, 2, 2, 2, "checkerboard", -1, false }; }


TextureStore::TextureStore() {
	Append(checkerboard2x2()); }


void TextureStore::Append(Texture t) {
	store.push_back(t); }


auto TextureStore::Find(std::string_view name) const -> const Texture* {
	for (auto& item : store) {
		if (item.name == name) {
			return &item; }}
	return nullptr; }


void TextureStore::LoadPNG(const std::pmr::string& path, std::string_view name) {
	auto existing = Find(name);
	if (existing != nullptr) {
		return; }

	Texture newtex = rglr::LoadPNG(path, name, true);
	newtex.maybe_make_mipmap();
	Append(newtex); }


void TextureStore::LoadDir(std::string_view dir) {
	char buf[4096];
	std::pmr::monotonic_buffer_resource pool(buf, sizeof(buf));

	auto lst = rcls::ListDir(dir, &pool);

	std::pmr::string path(&pool);
	path.reserve(dir.size() + 16);
	for (const auto& item : lst) {
		rcls::JoinPath(dir, item, path);
		if (EndsWithI(std::string_view{".png"}, item)) {
			fmt::print("loading texture \"{}\"\n", item);
			LoadPNG(path, item); }}}


void TextureStore::Print() {
	int i = 0;
	for (const auto& item : store) {
		const void * const ptr = item.buf.data();
		fmt::printf("#% 3d \"%-20s\" % 4d x% 4d", i, item.name, item.width, item.height);
		fmt::printf("  data@ 0x%p\n", ptr);
		++i; }}


}  // close package namespace
}  // close enterprise namespace
