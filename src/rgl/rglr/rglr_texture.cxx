#include "src/rgl/rglr/rglr_texture.hxx"

#include "src/rml/rmlg/rmlg_pow2.hxx"
#include "src/rcl/rcls/rcls_aligned_containers.hxx"
#include "src/rml/rmlv/rmlv_mvec4.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"

#include "3rdparty/pixeltoaster/PixelToaster.h"

namespace rqdq {
namespace rglr {

Texture ensurePowerOf2(Texture& src) {
	if (src.height == src.width && rmlg::is_pow2(src.width)) {
		return src; }

	int longEdge = std::max(src.width, src.height);

	const int dim = rmlg::pow2ceil(longEdge);

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

	if (!rmlg::is_pow2(width)) {
		pow = -1;
		mipmap = false;
		return; }

	pow = rmlg::ilog2(width);
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


}  // namespace rglr
}  // namespace rqdq
