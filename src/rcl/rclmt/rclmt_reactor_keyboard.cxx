#include "src/rcl/rclmt/rclmt_reactor_keyboard.hxx"

#include <functional>
#include <vector>

#include <Windows.h>

namespace rqdq {
namespace {

std::vector<bool> keyState(256, false);


void HandleKeyboardInput(const std::function<void(rclmt::KeyEvent)>& func) {
	INPUT_RECORD record;
	DWORD numRead;
	if (ReadConsoleInputW(GetStdHandle(STD_INPUT_HANDLE), &record, 1, &numRead) == 0) {
		throw std::runtime_error("ReadConsoleInput failure"); }
	if (record.EventType == KEY_EVENT) {
		auto& e = record.Event.KeyEvent;

		if (e.wVirtualScanCode<256) {
			keyState[e.wVirtualScanCode] = (e.bKeyDown != 0); }

		rclmt::KeyEvent ke{
			e.bKeyDown != 0,
			e.uChar.AsciiChar,
			e.dwControlKeyState,
			e.wVirtualScanCode };
		for (int n=0; n<e.wRepeatCount; n++) {
			func(ke); }}}

}  // namespace

namespace rclmt {

bool IsKeyDown(int scanCode) {
	if (scanCode < 256) {
		return keyState[scanCode]; }
	return false; }


void ListenKeyboard(std::function<void(KeyEvent)> func, Reactor* reactor_/*=nullptr*/) {
	// XXX func could be moved
	auto& reactor = reactor_ != nullptr ? *reactor_ : Reactor::GetInstance();
	reactor.ListenMany(Event(GetStdHandle(STD_INPUT_HANDLE)),
	                   [func{std::move(func)}]() { HandleKeyboardInput(func); }); }


}  // namespace rclmt
}  // namespace rqdq
