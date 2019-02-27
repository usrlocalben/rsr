#include "src/rcl/rcls/rcls_file.hxx"
#include "src/rcl/rclt/rclt_util.hxx"

#include <algorithm>
#include <cctype>
#include <codecvt>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <Windows.h>

namespace rqdq {
namespace rcls {

using namespace std;

vector<string> FindGlob(const string& pathpat) {
	vector<string> lst;
	WIN32_FIND_DATA ffd;

	HANDLE hFind = FindFirstFileW(rclt::UTF8Codec::decode(pathpat).c_str(), &ffd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
			lst.push_back(rclt::UTF8Codec::encode(wstring{ffd.cFileName}));
		} while (FindNextFile(hFind, &ffd) != 0); }
	return lst; }


/*
* example from
* http://nickperrysays.wordpress.com/2011/05/24/monitoring-a-file-last-modified-date-with-visual-c/
*/
int64_t GetMTime(const string& path) {
	int64_t mtime = -1;
	HANDLE hFile = CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile != INVALID_HANDLE_VALUE) {
		FILETIME ftCreate, ftAccess, ftWrite;
		if (!GetFileTime(hFile, &ftCreate, &ftAccess, &ftWrite)) {
			mtime = 0; }
		else {
			mtime = (int64_t)ftWrite.dwHighDateTime << 32 | ftWrite.dwLowDateTime; }
		CloseHandle(hFile); }
	return mtime; }


vector<char> file_get_contents(const string& path) {
	vector<char> buf;
	ifstream f(path);
	f.exceptions(ifstream::badbit | ifstream::failbit | ifstream::eofbit);
	f.seekg(0, ios::end);
	streampos length(f.tellg());
	if (length) {
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
	if (length) {
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


}  // close package namespace
}  // close enterprise namespace
