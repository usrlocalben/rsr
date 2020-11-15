#include "src/rgl/rglr/rglr_texture_load.hxx"

#include "src/rcl/rcls/rcls_aligned_containers.hxx"
#include "src/rcl/rcls/rcls_file.hxx"
#include "src/rgl/rglr/rglr_texture.hxx"
#include "src/rml/rmlg/rmlg_pow2.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"

#include <fstream>
#include <string>
#include <vector>

#include <fmt/format.h>
#include <fmt/printf.h>
#include "3rdparty/pixeltoaster/PixelToaster.h"
#include "3rdparty/picopng/picopng.h"
#include "3rdparty/ryg-srgb/ryg-srgb.h"

namespace rqdq {
namespace {

auto LoadFile(std::ifstream& fd) -> std::vector<char> {
	if (!fd.is_open()) {
		std::cerr << "file is not open!\n";
		std::exit(1); }
	// get filesize
	std::streamsize size = 0;
	if (fd.seekg(0, std::ios::end).good()) {
		size = fd.tellg(); }
	if (fd.seekg(0, std::ios::beg).good()) {
		size -= fd.tellg(); }

	std::vector<char> buffer(size);
	fd.read(buffer.data(), size);
	return buffer; }


auto PNGToTexture(const std::vector<char>& data, std::string name, const bool premultiply) -> rglr::Texture {
	using ryg::srgb8_to_float;

	std::vector<unsigned char> image;

	unsigned long w, h;
	int error = decodePNG(image, w, h, data.empty() ? nullptr : reinterpret_cast<const std::uint8_t*>(data.data()), static_cast<unsigned long>(data.size()));
	if (error != 0) {
		std::cout << "error(" << error << ") decoding png from [" << name << "]" << std::endl;
		while (1) {}}

	rcls::vector<PixelToaster::FloatingPointPixel> pc;

	pc.resize(w * h);

	auto* ic = image.data();
	for (unsigned i = 0; i < w*h; i++) {
		auto& dst = pc[i];
		dst.r = srgb8_to_float(*(ic++));
		dst.g = srgb8_to_float(*(ic++));
		dst.b = srgb8_to_float(*(ic++));
		dst.a = *(ic++) / 255.0F; // XXX is alpha srgb?
		if (premultiply) {
			dst.r *= dst.a;
			dst.g *= dst.a;
			dst.b *= dst.a; }}

	return { pc, int(w), int(h), int(w), move(name), -1, false }; }

}  // close unnamed namespace
namespace rglr {

auto LoadPNG(const std::pmr::string& filename, std::string_view name, const bool premultiply) -> Texture {
	std::ifstream fd(filename.c_str(), std::ios::in | std::ios::binary);
	return PNGToTexture(LoadFile(fd), std::string(name), premultiply); }


auto LoadPNG(const char* filename, std::string name, const bool premultiply) -> Texture {
	std::ifstream fd(filename, std::ios::in | std::ios::binary);
	return PNGToTexture(LoadFile(fd), name, premultiply); }


auto LoadPNG(const std::pmr::string& filename, std::string name, const bool premultiply) -> Texture {
	std::ifstream fd(filename.c_str(), std::ios::in | std::ios::binary);
	return PNGToTexture(LoadFile(fd), name, premultiply); }


}  // namespace rglr
}  // namespace rqdq
