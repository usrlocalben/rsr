#pragma once
#include <string>
#include <vector>

namespace rqdq {
namespace rclt {

std::vector<std::string> explode(const std::string& str, char ch);

std::string trim(const std::string& s);

bool consumePrefix(std::string& str, const std::string& prefix);

struct UTF8Codec {
	static std::wstring decode(const std::string&);
	static std::string encode(const std::wstring&); };

}  // close package namespace
}  // close enterprise namespace
