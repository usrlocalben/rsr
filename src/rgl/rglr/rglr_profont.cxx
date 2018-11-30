#include "src/rgl/rglr/rglr_profont.hxx"
#include "src/rcl/rclr/rclr_algorithm.hxx"
#include "src/rgl/rglr/rglr_canvas.hxx"

#include <functional>
#include <sstream>
#include <vector>

#include <picopng.h>

namespace rqdq {
namespace rglr {

extern std::vector<uint8_t> profontbits;

const auto BITMAP_STRIDE = int(192);
const auto GLYPH_SIZE_IN_PX = rmlv::ivec2(6, 11);


std::vector<uint8_t> decode_png(std::vector<uint8_t>& png_bits) {

	std::vector<uint8_t> image;
	std::vector<uint8_t> bitmap;

	unsigned long w, h;
	int error = decodePNG(image, w, h, png_bits.empty() ? 0 : png_bits.data(), (int)png_bits.size());

	if (error != 0) {
		std::stringstream ss;
		ss << "png error: " << error;
		throw std::exception(ss.str().c_str()); }

	// convert black-on-white truecolor
	// to 1=foreground, 0=background
	for (int i = 0; i < image.size(); i += 4) {
		bitmap.push_back(image[i] == 0xff ? 0 : 1); }

	return bitmap; }


ProPrinter::ProPrinter()
	:bitmap(decode_png(profontbits))
{
	static const std::string row1(R"( !"#$%&'()*+,-./0123456789:;<=>?)");
	static const std::string row2(R"(@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_)");
	static const std::string row3(R"(`abcdefghijklmnopqrstuvwxyz{|}~ )");

	rclr::each(row1, [&](char ch, int x) { charmap[ch] = rmlv::ivec2(x, 0); });
	rclr::each(row2, [&](char ch, int x) { charmap[ch] = rmlv::ivec2(x, 1); });
	rclr::each(row3, [&](char ch, int x) { charmap[ch] = rmlv::ivec2(x, 2); }); }



int ProPrinter::charmap_to_offset(const rmlv::ivec2& cxy) const {
	rmlv::ivec2 gxy = cxy * GLYPH_SIZE_IN_PX;
	const int tex_width = 192;
	//const int tex_height = 77;
	return gxy.y * tex_width + gxy.x + 1; }


int ProPrinter::char_to_offset(const char ch) const {
	auto cxy = charmap.find(ch);
	if (cxy == charmap.end()) {
		// char-not-found mapped to Space
		return 0; }
	else {
		return charmap_to_offset(cxy->second); }}


void ProPrinter::draw_glyph(const char ch, int left, int top, TrueColorCanvas& canvas) const {

	left = left < 0 ? canvas.width() + left : left;
	top = top < 0 ? canvas.height() + top : top;

	const auto* src = &bitmap[char_to_offset(ch)];
	PixelToaster::TrueColorPixel* dst = &canvas.data()[top * canvas.stride()];
	for (int y = 0; y < GLYPH_SIZE_IN_PX.y; y++) {

		const int canvas_y = top + y;
		if (0 <= canvas_y && canvas_y < canvas.height()) {
			for (int x = 0; x < GLYPH_SIZE_IN_PX.x; x++) {

				const int canvas_x = left + x;
				//if (canvas_x >= 0 && canvas_x < canvas.width()) {
				if (0 <= canvas_x && canvas_x < canvas.width()) {

					dst[canvas_x].integer = src[x] ? 0xffffffff : 0;}}}

		dst += canvas.stride();
		src += BITMAP_STRIDE; }}



void ProPrinter::write(const std::string str, int left, int top, TrueColorCanvas& canvas) const {
	for (const char& ch : str) {
		draw_glyph(ch, left, top, canvas);
		left += GLYPH_SIZE_IN_PX.x; }}


std::vector<uint8_t> profontbits = {
	137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,192,0,0,0,77,8,3,
	0,0,0,199,44,155,51,0,0,0,25,116,69,88,116,83,111,102,116,119,97,114,
	101,0,65,100,111,98,101,32,73,109,97,103,101,82,101,97,100,121,113,201,
	101,60,0,0,3,34,105,84,88,116,88,77,76,58,99,111,109,46,97,100,111,98,
	101,46,120,109,112,0,0,0,0,0,60,63,120,112,97,99,107,101,116,32,98,101,
	103,105,110,61,34,239,187,191,34,32,105,100,61,34,87,53,77,48,77,112,
	67,101,104,105,72,122,114,101,83,122,78,84,99,122,107,99,57,100,34,63,
	62,32,60,120,58,120,109,112,109,101,116,97,32,120,109,108,110,115,58,
	120,61,34,97,100,111,98,101,58,110,115,58,109,101,116,97,47,34,32,120,
	58,120,109,112,116,107,61,34,65,100,111,98,101,32,88,77,80,32,67,111,
	114,101,32,53,46,51,45,99,48,49,49,32,54,54,46,49,52,53,54,54,49,44,32,
	50,48,49,50,47,48,50,47,48,54,45,49,52,58,53,54,58,50,55,32,32,32,32,
	32,32,32,32,34,62,32,60,114,100,102,58,82,68,70,32,120,109,108,110,115,
	58,114,100,102,61,34,104,116,116,112,58,47,47,119,119,119,46,119,51,46,
	111,114,103,47,49,57,57,57,47,48,50,47,50,50,45,114,100,102,45,115,121,
	110,116,97,120,45,110,115,35,34,62,32,60,114,100,102,58,68,101,115,99,
	114,105,112,116,105,111,110,32,114,100,102,58,97,98,111,117,116,61,34,
	34,32,120,109,108,110,115,58,120,109,112,61,34,104,116,116,112,58,47,
	47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,
	48,47,34,32,120,109,108,110,115,58,120,109,112,77,77,61,34,104,116,116,
	112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,
	47,49,46,48,47,109,109,47,34,32,120,109,108,110,115,58,115,116,82,101,
	102,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,
	111,109,47,120,97,112,47,49,46,48,47,115,84,121,112,101,47,82,101,115,
	111,117,114,99,101,82,101,102,35,34,32,120,109,112,58,67,114,101,97,116,
	111,114,84,111,111,108,61,34,65,100,111,98,101,32,80,104,111,116,111,
	115,104,111,112,32,67,83,54,32,40,87,105,110,100,111,119,115,41,34,32,
	120,109,112,77,77,58,73,110,115,116,97,110,99,101,73,68,61,34,120,109,
	112,46,105,105,100,58,66,65,49,48,56,67,65,69,54,56,51,51,49,49,69,52,
	57,56,51,69,57,51,56,51,55,51,50,65,67,68,54,56,34,32,120,109,112,77,
	77,58,68,111,99,117,109,101,110,116,73,68,61,34,120,109,112,46,100,105,
	100,58,66,65,49,48,56,67,65,70,54,56,51,51,49,49,69,52,57,56,51,69,57,
	51,56,51,55,51,50,65,67,68,54,56,34,62,32,60,120,109,112,77,77,58,68,
	101,114,105,118,101,100,70,114,111,109,32,115,116,82,101,102,58,105,110,
	115,116,97,110,99,101,73,68,61,34,120,109,112,46,105,105,100,58,66,65,
	49,48,56,67,65,67,54,56,51,51,49,49,69,52,57,56,51,69,57,51,56,51,55,
	51,50,65,67,68,54,56,34,32,115,116,82,101,102,58,100,111,99,117,109,101,
	110,116,73,68,61,34,120,109,112,46,100,105,100,58,66,65,49,48,56,67,65,
	68,54,56,51,51,49,49,69,52,57,56,51,69,57,51,56,51,55,51,50,65,67,68,
	54,56,34,47,62,32,60,47,114,100,102,58,68,101,115,99,114,105,112,116,
	105,111,110,62,32,60,47,114,100,102,58,82,68,70,62,32,60,47,120,58,120,
	109,112,109,101,116,97,62,32,60,63,120,112,97,99,107,101,116,32,101,110,
	100,61,34,114,34,63,62,234,254,200,219,0,0,3,0,80,76,84,69,255,255,255,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,11,35,183,
	225,0,0,5,79,73,68,65,84,120,218,236,155,137,118,226,48,12,69,245,255,
	63,125,103,41,216,90,158,236,64,66,161,115,166,156,241,208,16,28,203,
	214,242,244,164,154,253,91,63,252,126,185,95,234,199,227,63,142,78,248,
	141,107,31,2,220,70,110,35,127,70,183,156,58,46,103,253,154,231,54,63,
	115,230,251,135,220,239,241,239,199,61,122,59,110,223,15,247,76,1,190,
	38,138,47,115,139,112,107,27,18,47,214,255,53,45,140,163,27,239,168,219,
	64,186,167,59,78,170,34,144,20,105,76,69,163,89,110,159,54,2,64,250,218,
	144,126,28,48,104,1,24,195,120,96,120,164,20,32,30,101,217,140,114,2,
	204,231,135,247,115,199,73,79,188,237,125,90,180,223,20,33,0,82,229,251,
	19,24,91,52,213,168,158,192,198,18,188,162,144,190,60,69,37,40,4,211,
	56,228,108,193,164,118,14,41,219,64,21,32,109,171,59,197,176,169,150,
	141,136,104,33,85,0,161,66,107,15,41,76,198,29,52,20,9,196,253,82,0,239,
	97,130,114,90,245,57,213,83,5,1,138,22,240,164,15,231,216,22,112,173,
	139,255,158,64,242,146,199,126,99,52,252,255,211,28,129,183,40,147,198,
	58,157,148,55,59,101,124,52,78,144,224,249,39,136,168,115,6,179,46,193,
	43,140,16,221,153,114,33,248,184,40,39,50,181,56,239,209,93,84,250,43,
	173,139,222,253,232,255,245,247,212,24,66,6,157,220,55,54,8,96,141,0,
	38,5,240,15,9,209,163,29,173,21,192,175,68,133,203,130,130,194,195,136,
	0,139,128,85,110,113,217,31,235,64,75,89,0,163,206,73,80,161,24,119,11,
	52,68,68,122,26,103,55,181,124,127,2,90,1,144,247,180,182,33,128,131,
	19,128,138,124,55,2,216,70,23,247,2,24,123,1,88,216,192,12,247,8,72,97,
	69,233,138,23,66,28,49,201,87,152,242,66,229,138,240,66,229,187,2,56,
	24,122,131,138,186,190,53,170,113,46,14,120,131,251,193,57,123,82,29,
	126,220,194,185,242,100,191,121,253,236,37,88,0,98,144,249,140,79,22,
	121,153,77,236,120,16,246,2,176,103,139,236,125,2,76,103,234,35,101,101,
	105,226,61,30,38,38,56,136,41,0,135,187,158,92,112,0,33,13,152,11,88,
	8,149,227,63,2,188,172,137,215,119,158,162,67,151,107,44,132,198,66,37,
	30,11,111,218,6,127,60,37,213,8,64,17,96,13,227,14,96,33,58,28,218,169,
	80,128,95,101,111,119,39,224,55,235,8,14,77,4,138,192,66,126,222,163,
	54,16,160,68,38,205,72,160,67,237,34,68,1,148,13,152,57,202,7,191,109,
	150,150,14,11,27,184,14,14,112,46,6,61,244,205,243,2,148,200,125,196,
	245,63,41,128,96,158,251,56,86,113,246,36,179,170,250,254,148,210,134,
	4,126,154,15,69,208,225,52,252,153,190,14,157,211,220,127,154,2,68,226,
	11,18,87,44,125,238,61,60,85,238,178,20,77,154,235,57,45,125,228,211,
	116,171,115,163,108,202,13,31,131,10,37,31,33,147,192,234,94,229,104,
	171,209,212,232,161,71,55,182,40,65,196,96,207,65,216,158,225,57,62,118,
	145,245,8,143,212,11,64,100,37,142,164,249,187,88,27,230,220,132,36,78,
	61,139,99,68,8,161,94,179,25,151,8,230,148,0,242,137,13,155,153,32,195,
	208,224,3,66,60,97,33,242,89,194,126,186,13,187,212,147,240,230,112,246,
	15,21,35,62,131,179,40,62,120,79,30,185,122,2,33,225,180,55,176,74,174,
	154,201,42,153,9,31,151,124,194,149,90,17,73,114,201,126,162,201,54,104,
	138,2,50,163,195,109,4,16,115,66,112,168,62,18,155,74,83,10,255,94,161,
	21,205,18,5,154,42,117,22,137,169,168,37,243,10,230,60,143,96,78,96,98,
	185,59,68,105,242,142,204,251,103,61,193,74,153,40,144,193,164,50,152,
	107,56,209,9,20,162,235,165,20,59,102,246,63,189,56,190,63,162,117,56,
	129,92,153,175,184,32,106,62,141,228,22,176,152,226,38,2,38,119,81,80,
	139,71,204,158,15,79,141,200,32,146,79,224,14,133,245,210,253,9,68,34,
	133,38,205,242,196,1,169,131,35,11,48,167,134,88,9,195,88,37,167,152,
	173,108,192,164,218,71,51,93,9,16,90,129,132,95,114,220,27,74,185,56,
	22,36,176,236,133,68,151,196,216,159,177,152,169,121,253,9,96,25,152,
	44,232,92,94,31,100,57,249,249,103,85,44,86,193,245,92,101,231,24,121,
	204,194,184,207,145,12,47,199,101,129,186,229,93,199,200,57,1,114,187,
	18,139,141,101,213,175,22,217,233,165,114,244,174,228,177,226,166,224,
	53,88,42,134,104,3,20,4,28,7,118,172,233,96,180,156,13,61,39,0,122,119,
	219,3,160,118,209,233,211,24,176,47,151,195,107,234,77,66,103,8,84,171,
	242,87,28,93,62,124,60,139,212,3,213,242,17,213,50,23,57,50,198,38,64,
	231,252,101,20,204,182,202,65,36,216,193,195,158,167,118,57,134,93,67,
	10,80,133,51,93,72,210,2,168,19,120,208,19,202,22,144,210,166,32,5,144,
	160,52,95,152,92,87,192,164,85,128,88,32,124,202,167,22,32,141,178,87,
	87,51,252,192,48,126,77,40,124,27,87,123,241,147,155,84,176,118,80,119,
	215,173,189,255,245,59,1,61,184,109,90,240,196,245,134,221,175,179,17,
	91,204,101,46,65,234,150,91,101,38,206,76,9,8,190,80,182,217,153,70,53,
	192,18,133,88,99,109,40,148,40,246,93,202,37,255,144,35,36,37,165,174,
	131,34,107,251,240,7,182,108,232,228,160,37,62,215,228,13,53,136,240,
	4,219,188,108,112,189,194,87,132,35,75,217,91,219,58,25,255,18,100,39,
	128,109,75,30,87,248,25,212,250,73,21,118,158,58,129,23,11,32,92,75,32,
	32,250,78,210,11,84,232,133,2,164,142,222,244,126,57,70,2,213,182,5,188,
	135,35,43,28,88,255,39,19,0,63,178,247,18,249,203,47,1,6,0,6,98,18,137,
	114,34,4,253,0,0,0,0,73,69,78,68,174,66,96,130
};

}  // close package namespace
}  // close enterprise namespace
