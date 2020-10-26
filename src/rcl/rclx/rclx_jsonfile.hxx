#pragma once
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "src/rcl/rcls/rcls_file.hxx"

#include "3rdparty/gason/gason.h"

namespace rqdq {
namespace rclx {

class JSONFile {
	const std::string path_{};
	int generation_{0};
	long long lastMTime_{0};
	std::vector<char> bytes_{};
	JsonValue jsonRoot_;
	std::optional<JsonAllocator> allocator_;
	bool valid_{false};

public:
	explicit JSONFile(std::string path);

	// XXX copyable, but new instance could be refreshed
	//     when the original is a generation behind
	JSONFile(const JSONFile& other) :
		path_(other.path_) {
		Reload(); }

	auto IsOutOfDate() const -> bool;
	auto Refresh() -> bool;
	auto IsValid() const -> bool;
	auto GetRoot() const -> JsonValue;
	auto Path() const -> const std::string& {
		return path_; }

private:
	void Reload();
	auto GetMTime() const -> int64_t; };


}  // close package namespace
}  // close enterprise namespace
