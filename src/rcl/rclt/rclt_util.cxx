#include "src/rcl/rclt/rclt_util.hxx"

#include <algorithm>
#include <cctype>
#include <codecvt>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <Windows.h>

namespace rqdq {
namespace rclt {

std::wstring UTF8Codec::Decode(const std::string& str) {
	if (str.empty()) {
		return std::wstring{}; }
	int needed = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, str.c_str(), str.size(), nullptr, 0);
	if (needed == 0) {
		throw std::runtime_error("error decoding UTF8"); }
	std::wstring out(needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.size(), out.data(), out.size());
	return out; }


std::string UTF8Codec::Encode(const std::wstring& str) {
	if (str.empty()) {
		return std::string{}; }
	int needed = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, str.c_str(), str.size(), nullptr, 0, nullptr, nullptr);
	if (needed == 0) {
		throw std::runtime_error("error decoding wchars"); }
	std::string out(needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, str.c_str(), str.size(), out.data(), needed, nullptr, nullptr);
	return out; }


std::vector<std::string> Split(const std::string& str, char ch) {
	std::vector<std::string> items;
	std::string src(str);
	auto nextmatch = src.find(ch);
	while (true) {
		auto item = src.substr(0, nextmatch);
		items.push_back(item);
		if (nextmatch == std::string::npos) { break; }
		src = src.substr(nextmatch + 1);
		nextmatch = src.find(ch); }

	return items; }


bool ConsumePrefix(std::string& str, const std::string& prefix) {
	if (str.compare(0, prefix.length(), prefix) == 0) {
		str.erase(0, prefix.length());
		return true; }
	return false; }


std::string Trim(const std::string &s) {
	auto it = cbegin(s);
	while (it != cend(s) && (isspace(*it) != 0)) {
		it++; }

	auto rit = crbegin(s);
	while (rit.base() != it && (isspace(*rit) != 0)) {
		rit++; }

	return std::string(it, rit.base()); }


}  // namespace rclt
}  // namespace rqdq
