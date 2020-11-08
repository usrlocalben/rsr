#include "rclt_util.hxx"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <codecvt>
#include <fstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include <Windows.h>

namespace rqdq {
namespace rclt {

// ----------------
// struct UTF8Codec
// ----------------

auto UTF8Codec::Decode(std::string_view text) -> std::wstring {
	thread_local std::string str;
	str.assign(text);
	return Decode(str); }

auto UTF8Codec::Decode(const std::string& str) -> std::wstring {
	assert(str.size() <= std::numeric_limits<int>::max());
	const auto inputLengthInBytes = static_cast<int>(str.size());
	std::wstring out;
	if (str.empty()) {
		return out; }
	const int needed = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, str.data(), inputLengthInBytes, nullptr, 0);
	if (needed == 0) {
		throw std::runtime_error("error decoding UTF8"); }
	out.resize(needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, str.data(), inputLengthInBytes, out.data(), needed);
	return out; }

auto UTF8Codec::Encode(const std::wstring& str) -> std::string {
	assert(str.size() <= std::numeric_limits<int>::max());
	const auto inputLengthInWChars = static_cast<int>(str.size());
	std::string out;
	if (str.empty()) {
		return out; }
	const int needed = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, str.data(), inputLengthInWChars, nullptr, 0, nullptr, nullptr);
	if (needed == 0) {
		throw std::runtime_error("error decoding wchars"); }
	out.resize(needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, str.data(), inputLengthInWChars, out.data(), needed, nullptr, nullptr);
	return out; }

// FREE FUNCTIONS
auto Split(const std::string& str, char delim) -> std::vector<std::string> {
	std::vector<std::string> items;
	std::string src(str);
	auto nextmatch = src.find(delim);
	while (true) {
		auto item = src.substr(0, nextmatch);
		items.emplace_back(item);
		if (nextmatch == std::string::npos) { break; }
		src = src.substr(nextmatch + 1);
		nextmatch = src.find(delim); }
	return items; }

void Split(const std::string& str, char delim, std::vector<std::string>& out) {
	std::vector<std::string> items;
	std::string src(str);
	auto nextmatch = src.find(delim);
	std::size_t cnt{0};
	while (true) {
		auto item = src.substr(0, nextmatch);
		if (cnt < out.size()) {
			out[cnt].assign(src.substr(0, nextmatch)); }
		else {
			out.emplace_back().assign(src.substr(0, nextmatch)); }
		++cnt;
		if (nextmatch == std::string::npos) { break; }
		src = src.substr(nextmatch + 1);
		nextmatch = src.find(delim); }
	out.resize(cnt); }

auto Trim(const std::string& s) -> std::string {
	auto lit = cbegin(s);
	auto rit = crbegin(s);
	while (lit != cend(s)    && (isspace(*lit) != 0)) ++lit;
	while (rit.base() != lit && (isspace(*rit) != 0)) ++rit;
	return std::string(lit, rit.base()); }

auto ConsumePrefix(std::string& str, const std::string& prefix) -> bool {
	if (str.compare(0, prefix.length(), prefix) == 0) {
		str.erase(0, prefix.length());
		return true; }
	return false; }

void ToLower(std::string& text) {
	std::transform(begin(text), end(text), begin(text), 
				   [](unsigned char ch) -> unsigned char { return (unsigned char)std::tolower(ch); }); }


}  // close package namespace
}  // close enterprise namespace
