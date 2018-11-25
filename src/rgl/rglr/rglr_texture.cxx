#include <rglr_texture.hxx>
#include <rcls_aligned_containers.hxx>
#include <rmlv_mvec4.hxx>
#include <rmlv_vec.hxx>
#include <rcls_file.hxx>

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


Texture load_png(const std::string filename, const std::string name, const bool premultiply) {
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


bool is_power_of_two(unsigned x) {
	while (((x & 1) == 0) && x > 1) {
		x >>= 1; }
	return x == 1; }


int ilog2(unsigned x) {
	int pow = 0;
	while (x) {
		x >>= 1;
		pow++; }
	return pow - 1; }


inline int powerOf2Successor(int a) {
	int dim = 1;
	while (dim < a) {
		dim <<= 1; }
	return dim; }


Texture ensurePowerOf2(Texture& src) {
	if (src.height == src.width && is_power_of_two(src.width)) {
		return src; }

	int longEdge = std::max(src.width, src.height);

	const int dim = powerOf2Successor(longEdge);

	rcls::vector<PixelToaster::FloatingPointPixel> img;
	img.resize(dim*dim);

	for (int iy = 0; iy < dim; iy++) {
		for (int ix = 0; ix < dim; ix++) {
			auto sy = double(iy) / double(dim-1) * double(src.height-1);
			auto sx = double(ix) / double(dim-1) * double(src.width-1);
			img[iy*dim + ix] = src.buf[sy*src.width + sx]; } }

	return Texture{ img, dim, dim, dim, src.name }; }


void Texture::maybe_make_mipmap() {

	if (width != height) {
		pow = -1;
		mipmap = false;
		return; }

	if (!is_power_of_two(width)) {
		pow = -1;
		mipmap = false;
		return; }

	pow = ilog2(width);
	if (pow > 12) {
		pow = -1;
		mipmap = false;
		return; }

	buf.resize(width * width * 2);

	int src_size = width;          // inital w/h of source
	int src = 0;                   // begin reading from the root image start
	int dst = src_size * src_size; // start output at the end of the root image
	for (int mip_level = pow - 1; mip_level >= 0; mip_level--) {
		//		std::cout << "making " << (sw >> 1) << std::endl;
		for (int src_y = 0; src_y < src_size; src_y += 2) {
			int dstrow = dst;
			for (int src_x = 0; src_x < src_size; src_x += 2) {

				const auto row1ofs = src + (src_y     *width) + src_x;
				const auto row2ofs = src + ((src_y + 1)*width) + src_x;

				const auto sum2x2 = (
					rmlv::mvec4f(buf[row1ofs].v) + rmlv::mvec4f(buf[row1ofs + 1].v) +
					rmlv::mvec4f(buf[row2ofs].v) + rmlv::mvec4f(buf[row2ofs + 1].v));

				const auto avg2x2 = sum2x2 / rmlv::mvec4f(4.0f);

				buf[dstrow++].v = avg2x2.v;
			}
			dst += stride;
		}
		src += src_size * stride; // advance read pos to mip-level we just drew
		src_size >>= 1;

	}
	this->mipmap = true;
	//	this->height *= 2;  // this is probably only useful for viewing the mipmap itself
}


Texture checkerboard2x2() {
	rcls::vector<PixelToaster::FloatingPointPixel> db;
	db.push_back(PixelToaster::FloatingPointPixel(0, 0, 0, 0));
	db.push_back(PixelToaster::FloatingPointPixel(1, 0, 1, 0));
	db.push_back(PixelToaster::FloatingPointPixel(1, 0, 1, 0));
	db.push_back(PixelToaster::FloatingPointPixel(0, 0, 0, 0));
	return{ db, 2, 2, 2, "checkerboard" }; }


TextureStore::TextureStore() {
	this->append(checkerboard2x2()); }


void TextureStore::append(Texture t) {
	store.push_back(t); }


const Texture * const TextureStore::find_by_name(const std::string& name) const {
	for (auto& item : store) {
		if (item.name == name) return &item; }
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
		for (auto& fn : rcls::fileglob(prepend + ext)) {
			//std::cout << "scanning [" << prepend << "][" << fn << "]" << std::endl;
			this->load_any(prepend, fn); }}}


void TextureStore::print() {
	int i = 0;
	for (const auto& item : store) {
		const void * const ptr = item.buf.data();
		fmt::printf("#% 3d \"%-20s\" % 4d x% 4d", i, item.name, item.width, item.height);
		fmt::printf("  data@ 0x%p\n", ptr);
		i++; }}


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
