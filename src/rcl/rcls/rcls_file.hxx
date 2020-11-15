#pragma once
#include <memory_resource>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace rqdq {
namespace rcls {

auto ListDir(std::string_view dir, std::pmr::memory_resource* mem) -> std::pmr::vector<std::pmr::string>;

auto GetMTime(const std::string& path) -> int64_t;

void LoadBytes(const std::string& path, std::vector<char>& buf);
auto LoadBytes(const std::string& path) -> std::vector<char>;
void LoadLines(const std::string& path, std::vector<std::string>& buf);
auto LoadLines(const std::string& path) -> std::vector<std::string>;
void LoadLines(const char* path, std::pmr::vector<std::pmr::string>& buf);

auto JoinPath(std::string a, const std::string& b) -> std::string;
auto JoinPath(std::string_view a, std::string_view b, std::pmr::memory_resource*) -> std::pmr::string;
void JoinPath(std::string_view a, std::string_view b, std::pmr::string& out);

void EnsureOpenable(const std::wstring& path);
void EnsureOpenable(const wchar_t* path);

void EnsureDirectoryExists(const std::string& path);

auto DirName(std::string fn) -> std::string;
auto DirNameView(std::string_view fn) -> std::string_view;

auto SplitDrive(std::string p) -> std::pair<std::string, std::string>;
auto SplitDrive(std::pmr::string p) -> std::pair<std::pmr::string, std::pmr::string>;
auto SplitPath(std::string p) -> std::pair<std::string, std::string>;
auto SplitPath(std::pmr::string p) -> std::pair<std::pmr::string, std::pmr::string>;


}  // close package namespace
}  // close enterprise namespace
