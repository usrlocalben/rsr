#pragma once
#include <algorithm>
#include <cctype>
#include <string>
#include <string_view>
#include <vector>

namespace rqdq {
namespace rclt {

// ----------------
// struct UTF8Codec
// ----------------

struct UTF8Codec {
	// CLASS METHODS
	static auto Decode(std::string_view) -> std::wstring;
	static auto Decode(const std::string&) -> std::wstring;
	static auto Encode(const std::wstring&) -> std::string; };

// FREE FUNCTIONS
auto Split(const std::string& text, char delim) -> std::vector<std::string>;

auto Split(const std::string& text, char delim, std::vector<std::string>& out) -> void;

auto Trim(const std::string& text) -> std::string;

auto ConsumePrefix(std::string& text, const std::string& prefix) -> bool;

void ToLower(std::string& text);
auto ToLower_copy(std::string text) -> std::string;


}  // close package namespace
}  // close enterprise namespace
