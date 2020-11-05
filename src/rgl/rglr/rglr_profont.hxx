#pragma once
#include "src/rml/rmlv/rmlv_vec.hxx"

#include <string_view>
#include <unordered_map>
#include <vector>

#include <fmt/format.h>
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
	std::unordered_map<char, rmlv::ivec2> charmap;
	std::vector<uint8_t> bitmap;

public:
	ProPrinter();
	int charmap_to_offset(const rmlv::ivec2& cxy) const;
	int char_to_offset(char ch) const;
	void draw_glyph(char ch, int left, int top, class TrueColorCanvas& canvas) const;

	void write(std::string_view str, int left, int top, class TrueColorCanvas& canvas) const;
	void write(const char *str, int left, int top, class TrueColorCanvas& canvas) const;
	void write(const fmt::memory_buffer&, int left, int top, class TrueColorCanvas& canvas) const; };


inline
void ProPrinter::write(const fmt::memory_buffer& buf, int left, int top, class TrueColorCanvas& canvas) const {
	return write(std::string_view(buf.data(), buf.size()), left, top, canvas); }


inline
void ProPrinter::write(const char* str, int left, int top, class TrueColorCanvas& canvas) const {
	return write(std::string_view(str), left, top, canvas); }


}  // namespace rglr
}  // namespace rqdq
