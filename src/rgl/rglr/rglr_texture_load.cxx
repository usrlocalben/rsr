#include "src/rgl/rglr/rglr_texture_load.hxx"

#include <fstream>
#include <string>
#include <vector>

#include "src/rcl/rcls/rcls_aligned_containers.hxx"
#include "src/rcl/rcls/rcls_file.hxx"
#include "src/rgl/rglr/rglr_texture.hxx"
#include "src/rml/rmlg/rmlg_pow2.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"

#include "3rdparty/fmt/include/fmt/format.h"
#include "3rdparty/fmt/include/fmt/printf.h"
#include "3rdparty/pixeltoaster/PixelToaster.h"
#include "3rdparty/picopng/picopng.h"
#include "3rdparty/ryg-srgb/ryg-srgb.h"

namespace rqdq {
namespace rglr {

std::vector<uint8_t> load_file(const std::string& filename);

Texture load_png(const std::string& filename, const std::string& name, const bool premultiply) {
	using ryg::srgb8_to_float;

	std::vector<unsigned char> image;

	auto data = load_file(filename);

	unsigned long w, h;
	int error = decodePNG(image, w, h, data.empty() ? nullptr : data.data(), static_cast<unsigned long>(data.size()));
	if (error != 0) {
		std::cout << "error(" << error << ") decoding png from [" << filename << "]" << std::endl;
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

	return { pc, int(w), int(h), int(w), name, -1, false }; }


Texture load_any(const std::string& prefix, const std::string& fn, const std::string& name, const bool premultiply) {
	//	string tmp = fn;
	//	transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);
	// std::cout << "texturestore: loading " << prefix << fn << std::endl;
	std::string ext = fn.substr(fn.length() - 4, 4);
	if (ext == ".png") {
		return load_png(prefix + fn, name, premultiply); }
//	else if (ext == ".jpg") {
//		return loadJpg(prefix + fn, name);
//	}
	std::cout << "unsupported texture extension \"" << ext << "\"" << std::endl;
	while (1) {}}


std::vector<uint8_t> load_file(const std::string& filename) {
	std::ifstream file(filename.c_str(), std::ios::in | std::ios::binary | std::ios::ate);

	// get filesize
	std::streamsize size = 0;
	if (file.seekg(0, std::ios::end).good()) {
		size = file.tellg(); }
	if (file.seekg(0, std::ios::beg).good()) {
		size -= file.tellg(); }

	std::vector<uint8_t> buffer;
	if (size > 0) {
		buffer.resize(size);
		file.read(reinterpret_cast<char*>(buffer.data()), size); }
	return buffer; }


}  // namespace rglr
}  // namespace rqdq
