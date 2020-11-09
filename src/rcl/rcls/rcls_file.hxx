#pragma once
#include <string>
#include <vector>

namespace rqdq {
namespace rcls {

std::vector<std::string> FindGlob(const std::string& pathpat);

int64_t GetMTime(const std::string& path);

std::vector<char> LoadBytes(const std::string& path);
void LoadBytes(const std::string& path, std::vector<char>& buf);

std::vector<std::string> LoadLines(const std::string& path);

auto JoinPath(std::string a, const std::string& b) -> std::string;
auto JoinPath(std::string a, const std::string& b, const std::string& c) -> std::string;

void EnsureOpenable(const std::wstring& path);

void EnsureDirectoryExists(const std::string& path);

auto DirName(std::string fn) -> std::string;

}  // namespace rcls
}  // namespace rqdq
