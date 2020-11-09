#include "src/rcl/rcls/rcls_file.hxx"

#include <algorithm>
#include <cctype>
#include <codecvt>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "src/rcl/rclt/rclt_util.hxx"

#include <Windows.h>

namespace rqdq {
namespace rcls {

auto FindGlob(const std::string& pathpat) -> std::vector<std::string> {
	std::vector<std::string> lst;
	WIN32_FIND_DATA ffd;

	HANDLE hFind = FindFirstFileW(rclt::UTF8Codec::Decode(pathpat).c_str(), &ffd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if ((ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0U) {
				continue; }
			lst.emplace_back(rclt::UTF8Codec::Encode(std::wstring{ffd.cFileName}));
		} while (FindNextFile(hFind, &ffd) != 0); }
	return lst; }


/*
* example from
* http://nickperrysays.wordpress.com/2011/05/24/monitoring-a-file-last-modified-date-with-visual-c/
*/
auto GetMTime(const std::string& path) -> int64_t {
	int64_t mtime = -1;
	HANDLE hFile = CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
	if (hFile != INVALID_HANDLE_VALUE) {
		FILETIME ftCreate, ftAccess, ftWrite;
		if (GetFileTime(hFile, &ftCreate, &ftAccess, &ftWrite) == 0) {
			mtime = 0; }
		else {
			mtime = static_cast<int64_t>(ftWrite.dwHighDateTime) << 32 | ftWrite.dwLowDateTime; }
		CloseHandle(hFile); }
	return mtime; }


auto LoadBytes(const std::string& path) -> std::vector<char> {
	std::vector<char> buf;
	std::ifstream f(path);
	f.exceptions(std::ifstream::badbit | std::ifstream::failbit | std::ifstream::eofbit);
	f.seekg(0, std::ios::end);
	std::streampos length(f.tellg());
	if (length != 0) {
		std::cout << "reading " << length << " bytes from " << path << std::endl;
		f.seekg(0, std::ios::beg);
		buf.resize(static_cast<size_t>(length));
		f.read(buf.data(), static_cast<std::size_t>(length)); }
	else {
		std::cout << "nothing to read from " << path << std::endl; }
	return buf; }


void LoadBytes(const std::string& path, std::vector<char>& buf) {
	std::ifstream fd(path, std::ios::binary);
	fd.exceptions(std::ifstream::badbit | std::ifstream::failbit | std::ifstream::eofbit);
	fd.seekg(0, std::ios::end);
	std::streampos length(fd.tellg());
	if (length != 0) {
		fd.seekg(0, std::ios::beg);
		buf.resize(static_cast<size_t>(length));
		fd.read(buf.data(), buf.size()); }}


auto LoadLines(const std::string& path) -> std::vector<std::string> {
	std::ifstream fd(path);
	std::vector<std::string> out;
	std::string line;
	while (getline(fd, line)) {
		out.emplace_back(line); }
	return out; }


auto JoinPath(std::string a, const std::string& b) -> std::string {
	if (!a.empty() && a.back() != '\\' && a.back() != '/') {
		a.push_back('\\'); }
	if (!b.empty()) {
		if (b.front() == '\\') {
			a = b; }
		else {
			a += b; }}
	return a; }


auto JoinPath(std::string a, const std::string& b, const std::string& c) -> std::string {
	a = JoinPath(move(a), b);
	a = JoinPath(move(a), c);
	return a; }


void EnsureOpenable(const std::wstring& path) {
	const auto tmp = rclt::UTF8Codec::Encode(path);
	const char* pathPtr = tmp.c_str();
	OFSTRUCT ofs;
	HFILE hfile;
	memset(&ofs, 0, sizeof(OFSTRUCT));
	ofs.cBytes = sizeof(OFSTRUCT);
	hfile = OpenFile(pathPtr, &ofs, OF_EXIST);
	if (hfile == 0) {
		throw std::runtime_error("file is not openable");}}


/**
 * based on https://stackoverflow.com/questions/1517685/recursive-createdirectory
 */
void EnsureDirectoryExists(const std::string& path) {
	size_t pos = 0;
	do {
		pos = path.find_first_of("\\/", pos+1);
		CreateDirectoryA(path.substr(0, pos).c_str(), nullptr);
	} while (pos != std::string::npos); }


auto DirName(std::string fn) -> std::string {
	int i;
	for (i = int(fn.size()); i>=0; --i) {
		if (fn[i] == '\\' || fn[i] == '/') {
			break; }
	}
	fn = fn.substr(0, i<0?0:i);
	return fn; }


}  // namespace rcls
}  // namespace rqdq
