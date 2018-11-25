#include <rglr_texture_load.hxx>
#include <rcls_aligned_containers.hxx>
#include <rcls_file.hxx>
#include <rglr_texture.hxx>
#include <rmlg_pow2.hxx>
#include <rmlv_mvec4.hxx>
#include <rmlv_vec.hxx>

#include <fstream>
#include <string>
#include <vector>

#include <fmt/format.h>
#include <fmt/printf.h>
#include <PixelToaster.h>
#include <picopng.h>
#include <ryg-srgb.h>


namespace rqdq {
namespace rglr {

std::vector<uint8_t> load_file(const std::string& filename);

Texture load_png(const std::string& filename, const std::string& name, const bool premultiply) {
	using ryg::srgb8_to_float;

	std::vector<unsigned char> image;

	auto data = load_file(filename);

	unsigned long w, h;
	int error = decodePNG(image, w, h, data.empty() ? 0 : data.data(), (unsigned long)data.size());
	if (error != 0) {
		std::cout << "error(" << error << ") decoding png from [" << filename << "]" << std::endl;
		while (1); }

	rcls::vector<PixelToaster::FloatingPointPixel> pc;

	pc.resize(w * h);

	auto* ic = image.data();
	for (unsigned i = 0; i < w*h; i++) {
		auto& dst = pc[i];
		dst.r = srgb8_to_float(*(ic++));
		dst.g = srgb8_to_float(*(ic++));
		dst.b = srgb8_to_float(*(ic++));
		dst.a = *(ic++) / 255.0f; // XXX is alpha srgb?
		if (premultiply) {
			dst.r *= dst.a;
			dst.g *= dst.a;
			dst.b *= dst.a; }}

	return{ pc, int(w), int(h), int(w), name }; }


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
	else {
		std::cout << "unsupported texture extension \"" << ext << "\"" << std::endl;
		while (1); }}


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

}  // close package namespace
}  // close enterprise namespace
