#include "src/viewer/rqv_cpu_colors.hxx"

#include <array>

#include <PixelToaster.h>

namespace rqdq {
namespace rqv {

std::array<PixelToaster::TrueColorPixel, 8> cpu_colors{ {
	PixelToaster::TrueColorPixel(0x468966),
	PixelToaster::TrueColorPixel(0xfff0a5),
	PixelToaster::TrueColorPixel(0xffb03b),
	PixelToaster::TrueColorPixel(0xb64926),
	PixelToaster::TrueColorPixel(0x8e2800),
	PixelToaster::TrueColorPixel(0x288e00),
	PixelToaster::TrueColorPixel(0x00288e)
} };

}  // close package namespace
}  // close enterprise namespace
