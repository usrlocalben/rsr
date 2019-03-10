#include "src/rcl/rclmt/rclmt_reactor_file.hxx"

#include <algorithm>
#include <list>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "src/rcl/rclmt/rclmt_event.hxx"
#include "src/rcl/rclt/rclt_util.hxx"
#include "src/rcl/rclw/rclw_winfile.hxx"

#include <Windows.h>
#include <fmt/printf.h>

namespace rqdq {
namespace {

struct FileOp {

	FileOp() = default;
	FileOp(const FileOp&) = delete;
	FileOp& operator=(const FileOp&) = delete;

	FileOp(FileOp&& other) noexcept
		:fd(std::move(other.fd)),
		event(std::move(other.event)),
		request(other.request),
		id(other.id),
		buffer(std::move(other.buffer)),
		good(other.good),
		expectedSizeInBytes(other.expectedSizeInBytes),
		deferred(std::move(other.deferred)) {}

	int id = -1;
	rclw::WinFile fd;
	rclmt::Event event;
	rclmt::LoadFileDeferred deferred;

	bool good = true;
	int64_t expectedSizeInBytes = 0;
	DWORD error = 0;
	std::vector<uint8_t> buffer;
	OVERLAPPED request{}; };


std::list<FileOp> fileOps;

int fileOpSeq = 0;

int GetNextOpId() { return fileOpSeq++; };

auto FindOpById(int id) {
	return std::find_if(begin(fileOps), end(fileOps),
	                    [=](auto& item) { return item.id == id; }); }

void onFileOpError(int id) {
	auto op = FindOpById(id);
	if (op==fileOps.end()) {
		auto msg = fmt::sprintf("onFileOpError(): fileOp id=%d not found", id);
		throw std::runtime_error(msg); }
	op->deferred.Errback(op->error);
	fileOps.erase(op); }


void onFileOpComplete(int id) {
	auto op = FindOpById(id);
	if (op==fileOps.end()) {
		auto msg = fmt::sprintf("onFileOpComplete(): fileOp id=%d not found", id);
		throw std::runtime_error(msg); }
	op->deferred.Callback(op->buffer);
	fileOps.erase(op); }


void onFileOpEvent(int id) {
	auto search = FindOpById(id);
	if (search == fileOps.end()) {
		auto msg = fmt::sprintf("onFileOpEvent(): fileOp id=%d not found", id);
		throw std::runtime_error(msg); }

	FileOp& op = *search;
	DWORD bytesRead = 0;

	auto success = GetOverlappedResult(op.fd.Get(), &op.request, &bytesRead, FALSE);
	if (success == 0) {
		auto error = GetLastError();
		auto msg = fmt::sprintf("GetOverlappedResult unexpected error %d", error);
		throw std::runtime_error(msg); }

	if (bytesRead != op.expectedSizeInBytes) {
		auto msg = fmt::sprintf("expected %d bytes but read %d", op.expectedSizeInBytes, bytesRead);
		throw std::runtime_error(msg); }

	FileOp foo(std::move(op));
	fileOps.erase(search);
	foo.deferred.Callback(foo.buffer); }

}  // namespace

namespace rclmt {

LoadFileDeferred& LoadFile(const std::string& path, Reactor* reactor) {
	auto tmp = rclt::UTF8Codec::Decode(path);
	return LoadFile(tmp, reactor); }


LoadFileDeferred& LoadFile(const std::wstring& path, Reactor* reactor_) {
	auto& reactor = reactor_ != nullptr ? *reactor_ : Reactor::GetInstance();

	fileOps.emplace_back(FileOp{});
	auto& op = fileOps.back();
	auto& d = op.deferred;

	op.id = GetNextOpId();
	op.good = true;
	op.error = 0;
	memset(&op.request, 0, sizeof(op.request));

	op.fd = CreateFileW(path.c_str(),
	                    GENERIC_READ,
	                    FILE_SHARE_READ,
	                    nullptr,
	                    OPEN_EXISTING,
	                    FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,
	                    nullptr);
	if (op.fd.Get() == INVALID_HANDLE_VALUE) {
		op.error = GetLastError();
		op.good = false;
		op.event = rclmt::Event::MakeEvent(true);
		reactor.ListenOnce(op.event, [&](){ onFileOpError(op.id); });
		return d; }

	auto result = GetFileSizeEx(op.fd.Get(), reinterpret_cast<LARGE_INTEGER*>(&op.expectedSizeInBytes));
	if (result == 0) {
		op.error = GetLastError();
		op.good = false;
		op.event = rclmt::Event::MakeEvent(true);
		reactor.ListenOnce(op.event, [&](){ onFileOpError(op.id); });
		return d; }

	op.buffer.resize(op.expectedSizeInBytes);
	op.event = rclmt::Event::MakeEvent(false);
	op.request.hEvent = op.event.Get();

	DWORD bytesRead;

	result = ReadFile(op.fd.Get(),
	                  op.buffer.data(),
	                  op.expectedSizeInBytes,
	                  &bytesRead,
	                  &op.request);
	op.error = GetLastError();

	if (result == 0 && op.error != ERROR_IO_PENDING) {
		// error...
		op.event = rclmt::Event::MakeEvent(true);
		reactor.ListenOnce(op.event, [&](){ onFileOpError(op.id); });
		return d; }

	if (result != 0) {
		// read was processed synchronously
		op.error = 0;
		op.event = rclmt::Event::MakeEvent(true);
		reactor.ListenOnce(op.event, [&](){ onFileOpComplete(op.id); });
		return d; }

	// request submitted successfully, wait for results
	op.error = 0;
	reactor.ListenOnce(op.event, [&](){ onFileOpEvent(op.id); });
	return d; }


}  // namespace rclmt
}  // namespace rqdq
