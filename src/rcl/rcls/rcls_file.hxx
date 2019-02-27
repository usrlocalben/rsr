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

}  // close package namespace
}  // close enterprise namespace
