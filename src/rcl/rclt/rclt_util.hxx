#pragma once
#include <string>
#include <string_view>
#include <vector>

namespace rqdq {
namespace rclt {

std::vector<std::string> Split(const std::string& str, char ch);
void Split(const std::string& str, char ch, std::vector<std::string>& out);

std::string Trim(const std::string& s);

bool ConsumePrefix(std::string& str, const std::string& prefix);

struct UTF8Codec {
	static std::wstring Decode(std::string_view /*str_*/);
	static std::wstring Decode(const std::string& /*str*/);
	static std::string Encode(const std::wstring& /*str*/); };


}  // namespace rclt
}  // namespace rqdq
