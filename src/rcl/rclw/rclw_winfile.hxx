#pragma once
#include <algorithm>

#include <Windows.h>

namespace rqdq {
namespace rclw {

class WinFile {
public:
	// lifecycle
	WinFile(HANDLE handle=INVALID_HANDLE_VALUE) :d_handle(handle) {}
	WinFile(const WinFile&) = delete;
	WinFile& operator=(const WinFile&) = delete;
	WinFile(WinFile&& other) noexcept {
		other.Swap(*this); }
	WinFile& operator=(WinFile&& other) noexcept {
		other.Swap(*this);
		return *this; }
	~WinFile() {
		Close(); }
	HANDLE Get() const {
		return d_handle;}
	void Swap(WinFile& other) noexcept {
		std::swap(d_handle, other.d_handle); }
	void Reset(HANDLE handle=INVALID_HANDLE_VALUE) {
		WinFile(handle).Swap(*this); }
	void Close() {
		if (d_handle != INVALID_HANDLE_VALUE) {
			const auto tmp = d_handle;
			d_handle = INVALID_HANDLE_VALUE;
			CloseHandle(tmp); }}

private:
	HANDLE d_handle = INVALID_HANDLE_VALUE; };


}  // namespace rclw
}  // namespace rqdq
