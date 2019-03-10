#pragma once
#include <string>
#include <vector>

namespace rqdq {
namespace rclt {

std::vector<std::string> Split(const std::string& str, char ch);

std::string Trim(const std::string& s);

bool ConsumePrefix(std::string& str, const std::string& prefix);

struct UTF8Codec {
	static std::wstring Decode(const std::string&);
	static std::string Encode(const std::wstring&); };


}  // close package namespace
}  // close enterprise namespace
