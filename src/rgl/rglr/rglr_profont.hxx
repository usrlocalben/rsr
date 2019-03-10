#pragma once
#include <unordered_map>
#include <vector>

#include "src/rml/rmlv/rmlv_vec.hxx"

#include "3rdparty/pixeltoaster/PixelToaster.h"

namespace rqdq {
namespace rglr {

/*
SPACE,!,",#,$,%,&,',(,),*,+,COMMA,-,.,/,0,1,2,3,4,5,6,7,8,:,;,<,=,>,?
@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_
`abcdefghijklmnopqrstuvwxyz{|}~MISSING

other interesting chars..

"degrees" y=4, x=1
"notequal" y=4, x=13
bullet y=4, x=5
plusminus, y=4, x=17
*/

class ProPrinter {
public:
	ProPrinter();
	int charmap_to_offset(const rmlv::ivec2& cxy) const;
	int char_to_offset(const char ch) const;
	void draw_glyph(const char ch, int left, int top, struct TrueColorCanvas& canvas) const;
	void write(const std::string str, int left, int top, struct TrueColorCanvas& canvas) const;

private:
	// const ivec2 glyph_dimensions;
	std::unordered_map<char, rmlv::ivec2> charmap;
	std::vector<uint8_t> bitmap; };


}  // namespace rglr
}  // namespace rqdq
