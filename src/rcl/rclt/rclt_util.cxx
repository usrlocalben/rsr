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
namespace {

template <typename T>
void UTF8Codec_Decode(std::string_view text, T& out) {
	const auto inputLengthInBytes = static_cast<int>(text.size());
	out.clear();
	if (inputLengthInBytes == 0) {
		return; }
	const int needed = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text.data(), inputLengthInBytes, nullptr, 0);
	if (needed == 0) {
		throw std::runtime_error("error decoding UTF8"); }
	out.resize(needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, text.data(), inputLengthInBytes, out.data(), needed); }


template <typename T>
void UTF8Codec_Encode(std::wstring_view text, T& out) {
	const auto inputLengthInWChars = static_cast<int>(text.size());
	out.clear();
	if (text.empty()) {
		return; }
	const int needed = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, text.data(), inputLengthInWChars, nullptr, 0, nullptr, nullptr);
	if (needed == 0) {
		throw std::runtime_error("error decoding wchars"); }
	out.resize(needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, text.data(), inputLengthInWChars, out.data(), needed, nullptr, nullptr); }


}
namespace rclt {

auto UTF8Codec::Decode(std::string_view text) -> std::wstring {
	std::wstring out;
	UTF8Codec_Decode(text, out);
	return out; }

auto UTF8Codec::Decode(std::string_view text, std::pmr::memory_resource* mem) -> std::pmr::wstring {
	std::pmr::wstring out(mem);
	UTF8Codec_Decode(text, out);
	return out; }

auto UTF8Codec::Encode(std::wstring_view text) -> std::string {
	std::string out;
	UTF8Codec_Encode(text, out);
	return out; }

auto UTF8Codec::Encode(std::wstring_view text, std::pmr::memory_resource* mem) -> std::pmr::string {
	std::pmr::string out(mem);
	UTF8Codec_Encode(text, out);
	return out; }

// FREE FUNCTIONS
/*
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
*/

auto Trim(std::string_view s) -> std::string {
	auto lit = cbegin(s);
	auto rit = crbegin(s);
	while (lit != cend(s)    && (isspace(*lit) != 0)) ++lit;
	while (rit.base() != lit && (isspace(*rit) != 0)) ++rit;
	return { lit, rit.base() }; }


auto TrimView(std::string_view s) -> std::string_view {
	if (s.length() == 0) {
		return s; }
	auto lit = cbegin(s);
	auto rit = crbegin(s);
	while (lit != cend(s) && (isspace(*lit) != 0)) ++lit;
	while (rit.base() != lit && (isspace(*rit) != 0)) ++rit;
	auto contentBegin = &(*lit);
	auto contentEnd = &(*rit);
	//assert(contentEnd >= contentBegin);
	return { contentBegin, size_t(contentEnd - contentBegin+1) }; }


auto Trim(std::string_view s, std::pmr::memory_resource *mem) -> std::pmr::string {
	auto lit = cbegin(s);
	auto rit = crbegin(s);
	while (lit != cend(s)    && (isspace(*lit) != 0)) ++lit;
	while (rit.base() != lit && (isspace(*rit) != 0)) ++rit;
	return { lit, rit.base(), mem }; }


auto Split1View(std::string_view s) -> std::pair<std::string_view, std::string_view> {
	auto it = cbegin(s);
	while (it != cend(s) && (isspace(*it) == 0)) ++it;
	auto len = distance(cbegin(s), it);
	auto left = s.substr(0, len);
	while (it != cend(s) && (isspace(*it) != 0)) ++it;
	auto right = s.substr(distance(cbegin(s), it));
	return { left, right }; }


auto ConsumePrefix(std::string& str, std::string_view prefix) -> bool {
	if (str.compare(0, prefix.length(), prefix) == 0) {
		str.erase(0, prefix.length());
		return true; }
	return false; }


auto ConsumePrefix(std::pmr::string& str, std::string_view prefix) -> bool {
	if (str.compare(0, prefix.length(), prefix) == 0) {
		str.erase(0, prefix.length());
		return true; }
	return false; }


void ToLower(std::string& text) {
	std::transform(begin(text), end(text), begin(text), 
				   [](unsigned char ch) -> unsigned char { return (unsigned char)std::tolower(ch); }); }

void ToLower(std::pmr::string& text) {
	std::transform(begin(text), end(text), begin(text), 
				   [](unsigned char ch) -> unsigned char { return (unsigned char)std::tolower(ch); }); }

auto ToLower_copy(std::string text) -> std::string {
	std::transform(begin(text), end(text), begin(text),
				   [](unsigned char ch) -> unsigned char { return (unsigned char)std::tolower(ch); });
	return text; }

void ToLower_copy(std::string_view text, std::pmr::string& out) {
	std::transform(begin(text), end(text), back_inserter(out),
	               [](unsigned char ch) -> unsigned char { return (unsigned char)std::tolower(ch); }); }


}  // close package namespace
}  // close enterprise namespace
