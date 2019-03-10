#include "src/rcl/rclw/rclw_guid.hxx"

#include <string>

#include <Windows.h>

namespace rqdq {
namespace rclw {

CLSID CLSIDSerializer::Deserialize(const std::wstring& data) {
	const wchar_t* ptr = data.c_str();
	CLSID out;
	if (CLSIDFromString(ptr, &out) != NOERROR) {
		throw std::runtime_error{ "clsid string invalid" }; }
	return out; }


}  // namespace rclw
}  // namespace rqdq
