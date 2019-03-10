/**
 * raii wrapper for a pixeltoaster frame
 */
#pragma once
#include "src/rgl/rglr/rglr_canvas.hxx"

#include "3rdparty/pixeltoaster/PixelToaster.h"

namespace rqdq {
namespace rglr {

class WrongFormat : public std::exception {
	const char* what() const throw() override {
		return "Framebuffer is not RGBA8888"; } };


class WindowClosed : public std::exception {
	const char* what() const throw() override {
		return "Window was closed"; } };


class Frame {
public:
	Frame(PixelToaster::Display &display) : display(display) {
		ptfb = display.update_begin();
		if (ptfb == nullptr) {
			throw WindowClosed(); }
		if (ptfb->format != PixelToaster::Format::XRGB8888) {
			throw WrongFormat(); }}

	~Frame() {
		display.update_end(); }

	auto tc_ptr() {
		return reinterpret_cast<PixelToaster::TrueColorPixel *>(ptfb->pixels); }

	auto canvas() {
		return TrueColorCanvas(tc_ptr(), display.width(), display.height()); }

private:
	PixelToaster::Framebuffer *ptfb;
	PixelToaster::Display &display; };


}  // namespace rglr
}  // namespace rqdq
