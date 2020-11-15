#pragma once
#include <algorithm>
#include <cctype>
#include <string>
#include <string_view>
#include <memory_resource>
#include <vector>

namespace rqdq {
namespace rclt {

// ----------------
// struct UTF8Codec
// ----------------

struct UTF8Codec {
	// CLASS METHODS
	static auto Decode(std::string_view text, std::pmr::memory_resource* mem) -> std::pmr::wstring;
	static auto Decode(std::string_view) -> std::wstring;

	static auto Encode(std::wstring_view text) -> std::string;
	static auto Encode(std::wstring_view text, std::pmr::memory_resource* mem) -> std::pmr::string; };

// FREE FUNCTIONS
template <typename T>
void Split(std::string_view, char delim, T& out);

auto Split(std::string_view, char delim) -> std::vector<std::string>;
auto Split(std::string_view, char delim, std::pmr::memory_resource* mem) -> std::pmr::vector<std::pmr::string>;

auto Trim(std::string_view s) -> std::string;
auto Trim(std::string_view s, std::pmr::memory_resource *mem) -> std::pmr::string;
auto TrimView(std::string_view s) -> std::string_view;

auto Split1View(std::string_view s) -> std::pair<std::string_view, std::string_view>;

auto ConsumePrefix(std::string& text, std::string_view prefix) -> bool;
auto ConsumePrefix(std::pmr::string& text, std::string_view prefix) -> bool;

void ToLower(std::string& text);
auto ToLower_copy(std::string text) -> std::string;
void ToLower_copy(std::string_view text, std::pmr::string& out);

//=============================================================================
//								INLINE DEFINITIONS
//=============================================================================

							// ----------------
							// struct UTF8Codec
							// ----------------



// FREE FUNCTIONS
template <typename T>
inline
void Split(std::string_view src, char delim, T& out) {
	auto nextmatch = src.find(delim);
	while (true) {
		auto item = src.substr(0, nextmatch);
		out.emplace_back(item);
		if (nextmatch == std::string_view::npos) { break; }
		src = src.substr(nextmatch+1);
		nextmatch = src.find(delim); }}


inline
auto Split(std::string_view src, char delim) -> std::vector<std::string> {
	std::vector<std::string> tmp;
	Split(src, delim, tmp);
	return tmp; }


inline
auto Split(std::string_view src, char delim, std::pmr::memory_resource* mem) -> std::pmr::vector<std::pmr::string> {
	std::pmr::vector<std::pmr::string> tmp(mem);
	Split(src, delim, tmp);
	return tmp; }


}  // close package namespace
}  // close enterprise namespace
