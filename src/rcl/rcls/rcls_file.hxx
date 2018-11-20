#pragma once
#include <string>
#include <vector>


namespace rqdq {
namespace rcls {

std::vector<std::string> fileglob(const std::string& pathpat);

long long getmtime(const std::string& fn);

std::vector<char> file_get_contents(const std::string& fn);
void file_get_contents(const std::string& fn, std::vector<char>& buf);

std::vector<std::string> loadFileAsLines(const std::string& filename);

}  // close package namespace
}  // close enterprise namespace
