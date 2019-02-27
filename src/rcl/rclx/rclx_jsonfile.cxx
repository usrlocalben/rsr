#include "src/rcl/rclx/rclx_jsonfile.hxx"
#include "src/rcl/rcls/rcls_file.hxx"

#include <iostream>
#include <memory>

namespace rqdq {
namespace rclx {


void JSONFile::Reload() {
	d_jsonRoot = std::make_unique<JsonValue>();
	d_allocator = std::make_unique<JsonAllocator>();
	d_bytes.clear();

	rcls::LoadBytes(d_path, d_bytes);
	d_bytes.emplace_back(0);
	char* dataBegin = d_bytes.data();
	char* dataEnd;

	int status = jsonParse(dataBegin, &dataEnd, d_jsonRoot.get(), *d_allocator.get());
	if (status != JSON_OK) {
		std::cerr << jsonStrError(status) << " @" << (dataEnd - dataBegin) << std::endl;
		d_valid = false; }
	else {
		d_valid = true;
		d_generation++; }}


}  // close package namespace
}  // close enterprise namespace
