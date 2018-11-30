#include "src/rcl/rclt/rclt_util.hxx"

#include <algorithm>
#include <cctype>
#include <codecvt>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#undef min
#undef max

namespace rqdq {
namespace rclt {


std::wstring UTF8Codec::decode(const std::string& str) {
	if (str.empty()) {
		return std::wstring{}; }
	int needed = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, str.c_str(), str.size(), NULL, 0);
	if (needed == 0) {
		throw std::runtime_error("error decoding UTF8"); }
	std::wstring out(needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.size(), out.data(), out.size());
	return out; }


std::string UTF8Codec::encode(const std::wstring& str) {
	if (str.empty()) {
		return std::string{}; }
	int needed = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, str.c_str(), str.size(), NULL, 0, NULL, NULL);
	if (needed == 0) {
		throw std::runtime_error("error decoding wchars"); }
	std::string out(needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, str.c_str(), str.size(), out.data(), needed, NULL, NULL);
	return out; }


std::vector<std::string> explode(const std::string& str, char ch) {
	std::vector<std::string> items;
	std::string src(str);
	auto nextmatch = src.find(ch);
	while (1) {
		auto item = src.substr(0, nextmatch);
		items.push_back(item);
		if (nextmatch == std::string::npos) { break; }
		src = src.substr(nextmatch + 1);
		nextmatch = src.find(ch); }

	return items; }


bool consumePrefix(std::string& str, const std::string& prefix) {
	if (str.compare(0, prefix.length(), prefix) == 0) {
		str.erase(0, prefix.length());
		return true; }
	return false; }


std::string trim(const std::string &s) {
	std::string::const_iterator it = s.begin();
	while (it != s.end() && isspace(*it))
		it++;

	std::string::const_reverse_iterator rit = s.rbegin();
	while (rit.base() != it && isspace(*rit))
		rit++;

	return std::string(it, rit.base()); }

}  // close package namespace
}  // close enterprise namespace
