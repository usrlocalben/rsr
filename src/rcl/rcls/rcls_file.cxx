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
#undef min
#undef max

namespace rqdq {
namespace rcls {

using namespace std;

vector<string> fileglob(const string& pathpat) {
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
long long getmtime(const string& fn) {
	long long mtime = -1;
	HANDLE hFile = CreateFileA(fn.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile != INVALID_HANDLE_VALUE) {
		FILETIME ftCreate, ftAccess, ftWrite;
		if (!GetFileTime(hFile, &ftCreate, &ftAccess, &ftWrite)) {
			mtime = 0;
		}
		else {
			mtime = (long long)ftWrite.dwHighDateTime << 32 | ftWrite.dwLowDateTime;
		}
		CloseHandle(hFile);
	}
	return mtime;
}


vector<char> file_get_contents(const string& fn) {
	vector<char> buf;
	ifstream f(fn);
	f.exceptions(ifstream::badbit | ifstream::failbit | ifstream::eofbit);
	f.seekg(0, ios::end);
	streampos length(f.tellg());
	if (length) {
		cout << "reading " << length << " bytes from " << fn << endl;
		f.seekg(0, ios::beg);
		buf.resize(static_cast<size_t>(length));
		f.read(buf.data(), static_cast<std::size_t>(length));
	}
	else {
		cout << "nothing to read from " << fn << endl;
	}
	return buf;
}


void file_get_contents(const string& fn, vector<char>& buf) {
	ifstream f(fn);
	f.exceptions(ifstream::badbit | ifstream::failbit | ifstream::eofbit);
	f.seekg(0, ios::end);
	streampos length(f.tellg());
	if (length) {
		f.seekg(0, ios::beg);
		buf.resize(static_cast<size_t>(length));
		f.read(&buf.front(), static_cast<std::size_t>(length));
	}
}


vector<string> loadFileAsLines(const string& filename) {
	ifstream f(filename);
	vector<string> data;

	string ln;
	while (!getline(f, ln).eof()) {
		data.push_back(ln); }

	return data; }

}  // close package namespace
}  // close enterprise namespace
