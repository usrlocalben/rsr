#pragma once
#include <rcls_aligned_containers.hxx>

#include <string>

#include <PixelToaster.h>

namespace rqdq {
namespace rglr {

struct Texture {
	void maybe_make_mipmap();

	rcls::vector<PixelToaster::FloatingPointPixel> buf;
	int width;
	int height;
	int stride;
	std::string name;
	int pow;
	bool mipmap;

	void resize(int w, int h) {
		buf.resize(w * h);
		mipmap = false;
		width = w;
		stride = w;
		height = h; }};


}  // close package namespace
}  // close enterprise namespace
