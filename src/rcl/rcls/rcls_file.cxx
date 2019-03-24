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

using namespace std;

vector<string> FindGlob(const string& pathpat) {
	vector<string> lst;
	WIN32_FIND_DATA ffd;

	HANDLE hFind = FindFirstFileW(rclt::UTF8Codec::Decode(pathpat).c_str(), &ffd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if ((ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0U) {
				continue; }
			lst.push_back(rclt::UTF8Codec::Encode(wstring{ffd.cFileName}));
		} while (FindNextFile(hFind, &ffd) != 0); }
	return lst; }


/*
* example from
* http://nickperrysays.wordpress.com/2011/05/24/monitoring-a-file-last-modified-date-with-visual-c/
*/
int64_t GetMTime(const string& path) {
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


vector<char> LoadBytes(const string& path) {
	vector<char> buf;
	ifstream f(path);
	f.exceptions(ifstream::badbit | ifstream::failbit | ifstream::eofbit);
	f.seekg(0, ios::end);
	streampos length(f.tellg());
	if (length != 0) {
		cout << "reading " << length << " bytes from " << path << endl;
		f.seekg(0, ios::beg);
		buf.resize(static_cast<size_t>(length));
		f.read(buf.data(), static_cast<std::size_t>(length)); }
	else {
		cout << "nothing to read from " << path << endl; }
	return buf; }


void LoadBytes(const string& path, vector<char>& buf) {
	ifstream fd(path);
	fd.exceptions(ifstream::badbit | ifstream::failbit | ifstream::eofbit);
	fd.seekg(0, ios::end);
	streampos length(fd.tellg());
	if (length != 0) {
		fd.seekg(0, ios::beg);
		buf.resize(static_cast<size_t>(length));
		fd.read(buf.data(), buf.size()); }}


vector<string> LoadLines(const string& path) {
	ifstream fd(path);
	vector<string> out;
	string line;
	while (getline(fd, line)) {
		out.emplace_back(line); }
	return out; }


std::string JoinPath(const std::string& a, const std::string& b) {
	std::string out(a);
	if (out.back() != '\\') {
		out.push_back('\\'); }

	if (b.empty()) {
		return out; }

	if (b.front() == '\\') {
		out.assign(b); }
	else {
		out += b; }

	return out; }


std::string JoinPath(const std::string& a, const std::string& b, const std::string& c) {
	std::string out = JoinPath(a, b);
	return JoinPath(out, c); }


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


}  // namespace rcls
}  // namespace rqdq
