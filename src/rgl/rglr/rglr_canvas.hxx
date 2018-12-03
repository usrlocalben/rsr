#pragma once
#include "src/rcl/rcls/rcls_aligned_containers.hxx"
#include "src/rml/rmlg/rmlg_irect.hxx"
#include "src/rml/rmlv/rmlv_soa.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"

#include "3rdparty/pixeltoaster/PixelToaster.h"


namespace rqdq {
namespace rglr {

inline PixelToaster::TrueColorPixel to_tc_from_fpp(PixelToaster::FloatingPointPixel& fpp) {
	return {
		PixelToaster::integer8(fpp.r * 255.0),
		PixelToaster::integer8(fpp.g * 255.0),
		PixelToaster::integer8(fpp.b * 255.0)
		}; }


struct QFloatCanvas {
	QFloatCanvas() :_width(2), _height(2), _stride2(1) {
		buffer.reserve(1);
		_ptr = buffer.data(); }

	QFloatCanvas(const int width, const int height)
		:_width(width), _height(height), _stride2(width / 2) {
		buffer.reserve(width * height / 4);
		_ptr = buffer.data(); }

	auto data() { return _ptr; }
	auto cdata() const { return _ptr; }
	const auto width() const { return _width; }
	const auto height() const { return _height; }
	const auto stride2() const { return _stride2; }
	const rmlg::irect rect() const {
		return rmlg::irect{ rmlv::ivec2{0,0}, rmlv::ivec2{_width, _height} }; }
	const float aspect() const {
		if (_width == 0 || _height == 0) {
			return 1.0f; }
		return float(_width) / float(_height); }

	void resize(const int width, const int height) {
		buffer.reserve(width * height / 4);
		_ptr = buffer.data();
		_width = width;
		_height = height;
		_stride2 = width / 2; }

private:
	rcls::vector<rmlv::qfloat> buffer;
	rmlv::qfloat* _ptr;
	int _width;
	int _height;
	int _stride2;
	};


struct QFloat4Canvas {
	QFloat4Canvas() :_width(0), _height(0), _stride2(0), _ptr(nullptr) {}

	QFloat4Canvas(const int width, const int height)
		:_width(width), _height(height), _stride2(width / 2) {
		buffer.reserve(width * height / 4);
		_ptr = buffer.data();
	}
	auto data() { return _ptr; }
	auto cdata() const { return _ptr; }

	const auto width() const { return _width; }
	const auto height() const { return _height; }
	const auto stride2() const { return _stride2; }
	const rmlg::irect rect() const {
		return rmlg::irect{ rmlv::ivec2{0,0}, rmlv::ivec2{_width, _height} }; }
	const float aspect() const {
		if (_width == 0 || _height == 0) {
			return 1.0f; }
		return float(_width) / float(_height); }

	void resize(const int width, const int height) {
		buffer.reserve(width * height / 4);
		_ptr = buffer.data();
		_width = width;
		_height = height;
		_stride2 = width / 2; }

	rmlv::vec4 get_pixel(int x, int y) {
		const float* data = reinterpret_cast<const float*>(_ptr);
		const int channels = 4;
		const int pixelsPerQuad = 4;
		const int elemsPerQuad = channels * pixelsPerQuad;
		const int widthInQuads = _width / 2;
		const int heightInQuads = _height / 2;

		const int Yquad = y / 2;
		const int Xquad = x / 2;

		const int quadAddr = (Yquad * widthInQuads + Xquad) * elemsPerQuad;

		//int cellidx = y / 2 * _stride2 + x / 2;
		int sy = y % 2;
		int sx = x % 2;
		float r = data[ quadAddr +  0 + sy*2 + sx ];
		float g = data[ quadAddr +  4 + sy*2 + sx ];
		float b = data[ quadAddr +  8 + sy*2 + sx ];
		float a = data[ quadAddr + 12 + sy*2 + sx ];
		return rmlv::vec4{ r, g, b, a }; }

private:
	rcls::vector<rmlv::qfloat4> buffer;
	rmlv::qfloat4* _ptr;
	int _width;
	int _height;
	int _stride2;
	};


struct TrueColorCanvas {
	TrueColorCanvas(
		PixelToaster::TrueColorPixel * ptr,
	    const int width,
	    const int height,
	    const int stride = 0
	) :
		_ptr(ptr),
		_width(width),
		_height(height),
		_stride(stride == 0 ? width : stride),
		_aspect(float(width) / float(height)) {}

	auto data() { return _ptr; }
	auto cdata() const { return _ptr; }
	const auto width() const { return _width; }
	const auto height() const { return _height; }
	const auto stride() const { return _stride; }
	const auto aspect() const { return _aspect; }
	const rmlg::irect rect() const {
		return rmlg::irect{ rmlv::ivec2{0,0}, rmlv::ivec2{_width, _height} }; }
private:
	PixelToaster::TrueColorPixel * _ptr;
	const int _width;
	const int _height;
	const int _stride;
	const float _aspect;
	};


struct FloatingPointCanvas {
	FloatingPointCanvas(
		PixelToaster::FloatingPointPixel* ptr,
		const int width,
		const int height,
		const int stride = 0
	) :_ptr(ptr),
		_width(width),
		_height(height),
		_stride(stride == 0 ? width : stride),
		_aspect(float(width) / float(height)) {}

	FloatingPointCanvas() :_ptr(nullptr), _width(0), _height(0), _stride(0), _aspect(1.0f) {}

	auto data() { return _ptr; }
	const auto cdata() const { return _ptr; }
	const auto width() const { return _width; }
	const auto height() const { return _height; }
	const auto stride() const { return _stride; }
	const auto aspect() const { return _aspect; }
	const rmlg::irect rect() const {
		return rmlg::irect{ rmlv::ivec2{0,0}, rmlv::ivec2{_width, _height} }; }
private:
	PixelToaster::FloatingPointPixel * _ptr = nullptr;
	int _width = 0;
	int _height = 0;
	int _stride = 0;
	float _aspect = 1.0f; };

}  // close package namespace
}  // close enterprise namespace
