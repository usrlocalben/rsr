#pragma once
#include <vector>
#include <string>

#include "src/rcl/rclmt/rclmt_deferred.hxx"
#include "src/rcl/rclmt/rclmt_reactor.hxx"

namespace rqdq {
namespace rclmt {

using LoadFileDeferred = rclmt::Deferred<std::vector<uint8_t>&, uint32_t>;

LoadFileDeferred& LoadFile(const std::string& path, Reactor* reactor=nullptr);
LoadFileDeferred& LoadFile(const std::wstring& path, Reactor* reactor=nullptr);


}  // namespace rclmt
}  // namespace rqdq
