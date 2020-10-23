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

	auto IsOutOfDate() const -> bool;
	auto Refresh() -> bool;
	auto IsValid() const -> bool;
	auto GetRoot() const -> JsonValue;

private:
	void Reload();
	auto GetMTime() const -> int64_t; };


}  // close package namespace
}  // close enterprise namespace
