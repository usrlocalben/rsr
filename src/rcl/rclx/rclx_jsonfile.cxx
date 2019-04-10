#include "src/rcl/rclx/rclx_jsonfile.hxx"
#include "src/rcl/rcls/rcls_file.hxx"

#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>

namespace rqdq {
namespace rclx {

JSONFile::JSONFile(std::string path) :d_path(std::move(path)) {
	d_lastMTime = GetMTime();
	Reload(); }


bool JSONFile::IsOutOfDate() const {
	return GetMTime() != d_lastMTime; }


bool JSONFile::Refresh() {
	if (IsOutOfDate()) {
		d_lastMTime = GetMTime();
		Reload();
		return true; }
	return false;}


bool JSONFile::IsValid() const {
	return d_valid; }


JsonValue JSONFile::GetRoot() const {
	if (!d_valid) {
		throw std::runtime_error("document is not valid"); }
	return d_jsonRoot; }


int64_t JSONFile::GetMTime() const {
	return rcls::GetMTime(d_path); }


void JSONFile::Reload() {
	d_allocator = JsonAllocator{};
	d_bytes.clear();

	rcls::LoadBytes(d_path, d_bytes);
	d_bytes.emplace_back(0);
	char* dataBegin = d_bytes.data();
	char* dataEnd;

	int status = jsonParse(dataBegin, &dataEnd, &d_jsonRoot, *d_allocator);
	if (status != JSON_OK) {
		std::cerr << jsonStrError(status) << " @" << (dataEnd - dataBegin) << std::endl;
		d_valid = false; }
	else {
		d_valid = true;
		d_generation++; }}


}  // namespace rclx
}  // namespace rqdq
