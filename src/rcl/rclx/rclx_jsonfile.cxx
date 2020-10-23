#include "src/rcl/rclx/rclx_jsonfile.hxx"
#include "src/rcl/rcls/rcls_file.hxx"

#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>

namespace rqdq {
namespace rclx {

JSONFile::JSONFile(std::string path) :
	path_(std::move(path)) {
	lastMTime_ = GetMTime();
	Reload(); }


auto JSONFile::IsOutOfDate() const -> bool {
	return GetMTime() != lastMTime_; }


auto JSONFile::Refresh() -> bool {
	if (IsOutOfDate()) {
		lastMTime_ = GetMTime();
		Reload();
		return true; }
	return false;}


auto JSONFile::IsValid() const -> bool {
	return valid_; }


auto JSONFile::GetRoot() const -> JsonValue {
	if (!valid_) {
		throw std::runtime_error("document is not valid"); }
	return jsonRoot_; }


auto JSONFile::GetMTime() const -> int64_t {
	return rcls::GetMTime(path_); }


void JSONFile::Reload() {
	allocator_ = JsonAllocator{};
	bytes_.clear();

	rcls::LoadBytes(path_, bytes_);
	bytes_.emplace_back(static_cast<char>(0));
	char* dataBegin = bytes_.data();
	char* dataEnd;

	int status = jsonParse(dataBegin, &dataEnd, &jsonRoot_, *allocator_);
	if (status != JSON_OK) {
		std::cerr << jsonStrError(status) << " @" << (dataEnd - dataBegin) << std::endl;
		valid_ = false; }
	else {
		valid_ = true;
		generation_++; }}


}  // namespace rclx
}  // namespace rqdq
