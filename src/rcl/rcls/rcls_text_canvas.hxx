#pragma once
#include <algorithm>
#include <string>
#include <string_view>
#include <vector>

namespace rqdq {
namespace rcls {

/**
 * ASCII Character & Color attribute pair as used by VGA text-
 * modes, but 16-bits wide for ABI-compatibility with Windows
 * CHAR_INFO struct.
 */
struct ColorChar {
	union {
		wchar_t unicode;
		char ch; };
	uint16_t attr; };

enum Color {
	Black,
	Blue,
	Green,
	Cyan,
	Red,
	Magenta,
	Brown,
	White,

	StrongBlack,
	StrongBlue,
	StrongGreen,
	StrongCyan,
	StrongRed,
	StrongMagenta,
	StrongBrown,
	StrongWhite, };

// XXX const char Bright = 0x8;

inline uint8_t MakeAttribute(Color bg, Color fg) {
	return (bg<<4)|fg; }


class TextCanvas {
public:
	TextCanvas() :d_width(0), d_height(0) {}
	TextCanvas(int width, int height) : d_width(width), d_height(height), d_buf(d_width*d_height) {}

public:
	void Resize(int width, int height) {
		d_width = width;
		d_height = height;
		d_buf.resize(width*height);}

	void Clear() {
		std::fill(begin(d_buf), end(d_buf), ColorChar{});}

	ColorChar* GetDataPtr() { return d_buf.data(); }
	int d_width;
	int d_height;
	std::vector<ColorChar> d_buf; };


inline void WriteXY(TextCanvas& dst, int x, int y, const TextCanvas& src) {
	for (int ry=0; ry<dst.d_height; ry++) {
		for (int rx=0; rx<dst.d_width; rx++) {
			if ((ry>=y && ry<y+src.d_height) &&
			    (rx>=x && rx<x+src.d_width)) {
				int sx = rx-x;
				int sy = ry-y;
				auto& srcCell = src.d_buf[sy*src.d_width+sx];
				auto& dstCell = dst.d_buf[ry*dst.d_width+rx];
				dstCell = srcCell; }}}}


inline TextCanvas Flatten(const TextCanvas& a, const TextCanvas& b, int x, int y) {
	TextCanvas out(a.d_width, a.d_height);
	for (int ry=0; ry<a.d_height; ry++) {
		for (int rx=0; rx<a.d_width; rx++) {
			ColorChar ch;
			if ((ry>=y && ry<y+b.d_height) && (rx>=x && rx<x+b.d_width)) {
				int sx = rx-x;
				int sy = ry-y;
				ch = b.d_buf[sy*b.d_width+sx]; }
			else {
				ch = a.d_buf[ry*a.d_width+rx]; }
			out.d_buf[ry*a.d_width+rx] = ch; }}
	return out; }


inline void WriteXY(TextCanvas& canvas, int x, int y, std::string_view text) {
	if (0 <= y && y < canvas.d_height) {
		for (int xx=0; xx<canvas.d_width; xx++) {
			int ti = xx - x;
			if (0 <= ti && ti < text.length()) {
				canvas.d_buf[y*canvas.d_width + xx].ch = text[ti]; }}}}


inline void WriteXY(TextCanvas& canvas, int x, int y, std::string_view text, const uint8_t attr) {
	if (0 <= y && y < canvas.d_height) {
		for (int xx=0; xx<canvas.d_width; xx++) {
			int ti = xx - x;
			if (0 <= ti && ti < text.length()) {
				canvas.d_buf[y*canvas.d_width + xx].ch = text[ti];
				canvas.d_buf[y*canvas.d_width + xx].attr = attr; }}}}


inline void Fill(TextCanvas& canvas, uint8_t attr) {
	for (auto& cell : canvas.d_buf) {
		cell.attr = attr; }}


inline void DrawPercentageBar(TextCanvas& canvas, int x, int y, int width, float pct) {
	//char fillChar[2];
	//fillChar[0] = (char)0xb1; fillChar[1] = (char)0;
	const std::string fillChar("="); //
	const auto fillLen = int( pct * width );
	int cx;
	for ( cx=x; cx<fillLen+x; cx++) {
		WriteXY(canvas, cx, y, fillChar);}
	for (     ; cx<  width+x; cx++) {
		WriteXY(canvas, cx, y, " "     );}}


}  // namespace rcls
}  // namespace rqdq
