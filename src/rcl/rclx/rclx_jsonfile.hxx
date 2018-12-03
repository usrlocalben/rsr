#pragma once
#include "src/rcl/rcls/rcls_file.hxx"

#include "3rdparty/gason/gason.h"

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace rqdq {
namespace rclx {

class JSONFile {
public:
	JSONFile(const std::string& path) :d_path(path) {
		d_lastMTime = getMTime();
		reload(); }

	bool isOutOfDate() const {
		long long mTime = getMTime();
		if (mTime != d_lastMTime) {
			return true; }
		return false; }

	bool refresh() {
		long long mTime = getMTime();
		if (mTime != d_lastMTime) {
			d_lastMTime = mTime;
			reload();
			return true; }
		return false; }

	bool isValid() const {
		return d_valid; }

	const JsonValue& docroot() const {
		if (!d_valid) {
			throw std::runtime_error("document is not valid"); }
		return *d_jsonRoot.get(); }

private:
	void reload();

	long long getMTime() const {
		return rcls::getmtime(d_path); }

private:
	int d_generation = 0;
	long long d_lastMTime = 0;
	std::string d_path;
	std::vector<char> d_bytes;
	std::unique_ptr<JsonValue> d_jsonRoot;
	std::unique_ptr<JsonAllocator> d_allocator;
	bool d_valid = false;
	};

}  // close package namespace
}  // close enterprise namespace
