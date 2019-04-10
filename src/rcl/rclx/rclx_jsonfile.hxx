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
public:
	explicit JSONFile(std::string path);
	bool IsOutOfDate() const;
	bool Refresh();
	bool IsValid() const;
	JsonValue GetRoot() const;

private:
	void Reload();
	int64_t GetMTime() const;

private:
	int d_generation{0};
	long long d_lastMTime{0};
	std::string d_path{};
	std::vector<char> d_bytes{};
	JsonValue d_jsonRoot;
	std::optional<JsonAllocator> d_allocator;
	bool d_valid{false};
	};

}  // namespace rclx
}  // namespace rqdq
