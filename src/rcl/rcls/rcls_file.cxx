#include "src/rcl/rcls/rcls_file.hxx"

#include "src/rcl/rclt/rclt_util.hxx"

#include <algorithm>
#include <cctype>
#include <codecvt>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory_resource>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include <Windows.h>

namespace rqdq {
namespace {

class FindFileContext {
	HANDLE handle_{INVALID_HANDLE_VALUE};
	WIN32_FIND_DATAW data_;

	// private ctor
	FindFileContext() = default;

public:
	FindFileContext(const FindFileContext&) = delete;
	auto operator=(const FindFileContext&) -> FindFileContext& = delete;

	FindFileContext(FindFileContext&& other) noexcept {
		other.Swap(*this); }

	auto operator=(FindFileContext&& other) noexcept -> FindFileContext& {
		Swap(other);
		return *this; }

	~FindFileContext() {
		if (handle_ != INVALID_HANDLE_VALUE) {
			FindClose(handle_); }}

	void Swap(FindFileContext& other) {
		std::swap(handle_, other.handle_);
		std::swap(data_, other.data_); }

	friend auto FindFile(const wchar_t*) -> FindFileContext;
	friend auto FindFile(const std::wstring&) -> FindFileContext;
	friend auto FindFile(const std::pmr::wstring&) -> FindFileContext;

	auto Loaded() const -> bool {
		return handle_ != INVALID_HANDLE_VALUE; }

	void Next() {
		if (!Loaded()) return;
		if (FindNextFileW(handle_, &data_) == 0) {
			if (GetLastError() != ERROR_NO_MORE_FILES) {
				throw std::runtime_error("FindNextFileW failure"); }
			FindClose(handle_);
			handle_ = INVALID_HANDLE_VALUE; }}

	auto IsDir() const -> bool {
		return data_.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY; }

	auto Name() const -> std::wstring_view {
		// auto foo = rclt::UTF8Codec::Encode(std::wstring_view{data_.cFileName});
		// std::cerr << "name: " << foo << "\n";
		return data_.cFileName; }};


auto FindFile(const wchar_t* spec) -> FindFileContext {
	FindFileContext ffc{};
	ffc.handle_ = FindFirstFileW(spec, &ffc.data_);
	if (ffc.handle_ == INVALID_HANDLE_VALUE) {
		if (GetLastError() != ERROR_FILE_NOT_FOUND) {
			throw std::runtime_error("FindFirstFileW failure"); }}
	return ffc; }


auto FindFile(const std::wstring& spec) -> FindFileContext {
	return FindFile(spec.c_str()); }


}  // close unnamed namespace
namespace rcls {

auto ListDir(std::string_view dir, std::pmr::memory_resource* mem) -> std::pmr::vector<std::pmr::string> {
	int many = 0;
	auto tmp = rcls::JoinPath(std::string(dir), "*");
	auto wdir = rclt::UTF8Codec::Decode(tmp);

	{
		for (auto ff = FindFile(wdir); ff.Loaded(); ff.Next()) {
			auto fn = ff.Name();
			if (fn != L"." && fn != L"..") {
				many++; }}}

	std::pmr::vector<std::pmr::string> out(mem);
	out.reserve(many);

	for (auto ff = FindFile(wdir); ff.Loaded(); ff.Next()) {
		auto fn = ff.Name();
		if (fn != L"." && fn != L"..") {
			out.emplace_back(rclt::UTF8Codec::Encode(fn, mem)); }}

	return out; }

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


void LoadBytes(const std::string& path, std::vector<char>& buf) {
	std::ifstream fd(path, std::ios::binary);
	fd.exceptions(std::ifstream::badbit | std::ifstream::failbit | std::ifstream::eofbit);
	fd.seekg(0, std::ios::end);
	std::streampos length(fd.tellg());
	if (length != 0) {
		fd.seekg(0, std::ios::beg);
		buf.resize(static_cast<size_t>(length));
		fd.read(buf.data(), buf.size()); }}


auto LoadBytes(const std::string& path) -> std::vector<char> {
	std::vector<char> tmp;
	LoadBytes(path, tmp);
	return tmp; }


void LoadLines(const std::string& path, std::vector<std::string>& buf) {
	std::ifstream fd(path);
	std::string line;
	while (getline(fd, line)) {
		buf.emplace_back(line); }}


void LoadLines(const char* path, std::pmr::vector<std::pmr::string>& buf) {
	char tmp[1024];
	std::pmr::monotonic_buffer_resource pool(tmp, sizeof(tmp));
	std::pmr::string line(&pool);
	line.reserve(1023);

	std::ifstream fd(path);
	while (getline(fd, line)) {
		buf.emplace_back(line); }}


auto LoadLines(const std::string& path) -> std::vector<std::string> {
	std::vector<std::string> out;
	LoadLines(path, out);
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


auto JoinPath(std::string_view a, std::string_view b, std::pmr::memory_resource* mem) -> std::pmr::string {
	std::pmr::string out(mem);
	out.reserve(a.size() + b.size() + 1);
	out.assign(a);
	if (!a.empty() && a.back() != '\\' && a.back() != '/') {
		out.push_back('\\'); }
	if (!b.empty()) {
		if (b.front() == '\\') {
			out.assign(b); }
		else {
			out += b; }}
	return out; }


void JoinPath(std::string_view a, std::string_view b, std::pmr::string& out) {
	out.reserve(a.size() + b.size() + 1);
	out.assign(a);
	if (!a.empty() && a.back() != '\\' && a.back() != '/') {
		out.push_back('\\'); }
	if (!b.empty()) {
		if (b.front() == '\\') {
			out.assign(b); }
		else {
			out += b; }}}


void EnsureOpenable(const wchar_t* path) {
	HANDLE h;
	h = CreateFileW(path,
	                GENERIC_READ,
	                FILE_SHARE_READ,
	                NULL,
	                OPEN_EXISTING,
	                FILE_ATTRIBUTE_NORMAL,
	                NULL);
	if (h == INVALID_HANDLE_VALUE) {
		throw std::runtime_error("file is not openable");}
	CloseHandle(h); }


void EnsureOpenable(const std::wstring& path) {
	return EnsureOpenable(path.c_str()); }


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
	auto x = find_if(rbegin(fn), rend(fn), [](auto ch) { return ch=='\\' || ch=='/'; });
	if (x == rend(fn)) {
		if (fn.size() >= 2 && fn[1] == ':') {
			return fn.substr(0, 1); }
		return ""; }
	return fn.substr(0, distance(x, rend(fn))); }


auto DirNameView(std::string_view fn) -> std::string_view {
	auto x = find_if(rbegin(fn), rend(fn), [](auto ch) { return ch=='\\' || ch=='/'; });
	if (x == rend(fn)) {
		if (fn.size() >= 2 && fn[1] == ':') {
			return fn.substr(0, 1); }
		return ""; }
	return fn.substr(0, distance(x, rend(fn))); }


/**
 * from cpython ntpath.py
 */
auto SplitDrive(std::string p) -> std::pair<std::string, std::string> {
	if (p.size() >= 2) {
		const char sep = '\\';
		const char altsep = '/';
		const char colon = ':';

		auto normp = std::string{};
		normp.reserve(p.size());
		transform(begin(p), end(p), back_inserter(normp), [=](char ch) { return ch == altsep ? sep : ch; });

		if (normp[0] == sep && normp[1] == sep && normp[2] != sep) {
			// UNC path
			auto index = normp.find(sep, 2);
			if (index == std::string::npos) {
				return { "", move(p) }; }
			auto index2 = normp.find(sep, index+1);
			if (index2 == index + 1) {
				return { "", move(p) }; }
			if (index2 == -1) {
				index2 = p.size(); }
			return { p.substr(0, index2), p.substr(index2) }; }
		if (normp[1] == colon) {
			return { p.substr(0, 2), p.substr(2) }; }}
	return { "", move(p) }; }


auto SplitPath(std::string p) -> std::pair<std::string, std::string> {
	std::string d;
	tie(d, p) = SplitDrive(move(p));

	auto i = p.find_last_of("\\/") + 1;
	auto head = p.substr(0, i);
	auto tail = p.substr(i);

	auto idx = head.find_last_not_of("\\/");
	head = idx == std::string::npos ? head : head.substr(0, idx);
	return { d + head, tail }; }


auto SplitDrive(std::pmr::string p) -> std::pair<std::pmr::string, std::pmr::string> {
	if (p.size() >= 2) {
		const char sep = '\\';
		const char altsep = '/';
		const char colon = ':';

		auto normp = std::pmr::string(p.get_allocator());
		normp.reserve(p.size());
		transform(begin(p), end(p), back_inserter(normp), [=](char ch) { return ch == altsep ? sep : ch; });

		if (normp[0] == sep && normp[1] == sep && normp[2] != sep) {
			// UNC path
			auto index = normp.find(sep, 2);
			if (index == std::pmr::string::npos) {
				return { "", move(p) }; }
			auto index2 = normp.find(sep, index+1);
			if (index2 == index + 1) {
				return { "", move(p) }; }
			if (index2 == -1) {
				index2 = p.size(); }
			return { p.substr(0, index2), p.substr(index2) }; }
		if (normp[1] == colon) {
			return { p.substr(0, 2), p.substr(2) }; }}
	return { "", move(p) }; }


auto SplitPath(std::pmr::string p_) -> std::pair<std::pmr::string, std::pmr::string> {
	auto [d, p] = SplitDrive(move(p_));

	auto i = p.find_last_of("\\/") + 1;
	auto head = p.substr(0, i);
	auto tail = p.substr(i);

	auto idx = head.find_last_not_of("\\/");
	head = idx == std::pmr::string::npos ? head : head.substr(0, idx);
	return { d + head, tail }; }


}  // close package namespace
}  // close enterprise namespace
