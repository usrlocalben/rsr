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
		d_lastMTime = GetMTime();
		Reload(); }

	bool IsOutOfDate() const {
		return GetMTime() != d_lastMTime; }

	bool Refresh() {
		if (IsOutOfDate()) {
			d_lastMTime = GetMTime();
			Reload();
			return true; }
		return false;}

	bool IsValid() const {
		return d_valid; }

	const JsonValue& GetRoot() const {
		if (!d_valid) {
			throw std::runtime_error("document is not valid"); }
		return *d_jsonRoot.get(); }

private:
	void Reload();

	int64_t GetMTime() const {
		return rcls::GetMTime(d_path); }

private:
	int d_generation = 0;
	long long d_lastMTime = 0;
	std::string d_path;
	std::vector<char> d_bytes;
	std::unique_ptr<JsonValue> d_jsonRoot;
	std::unique_ptr<JsonAllocator> d_allocator;
	bool d_valid = false;
	};

}  // namespace rclx
}  // namespace rqdq
