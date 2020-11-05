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
	class TrueColorCanvas& canvas_;

public:
	ProPrinter(class TrueColorCanvas& canvas) :
		canvas_(canvas) {}

	void Write(rmlv::ivec2, char ch);
	void Write(rmlv::ivec2, std::string_view);
	void Write(rmlv::ivec2, const char*);
	void Write(rmlv::ivec2, const fmt::memory_buffer&); };

inline
void ProPrinter::Write(rmlv::ivec2 coord, const fmt::memory_buffer& buf) {
	return Write(coord, std::string_view(buf.data(), buf.size())); }


inline
void ProPrinter::Write(rmlv::ivec2 coord, const char* str) {
	return Write(coord, std::string_view(str)); }


}  // namespace rglr
}  // namespace rqdq
