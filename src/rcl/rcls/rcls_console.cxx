#include "src/rcl/rcls/rcls_console.hxx"

#include <stdexcept>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

#include "src/rcl/rcls/rcls_text_canvas.hxx"
#include "src/rcl/rclt/rclt_util.hxx"

#include <Windows.h>

namespace rqdq {
namespace {

DWORD oldConsoleInputMode;


}  // namespace

namespace rcls {

Console::Console(HANDLE hStdout, HANDLE hStdin)
	:d_stdout{hStdout}, d_stdin{hStdin}, d_activeKeys(65536, false)  {
	GetConsoleMode(d_stdin, &oldConsoleInputMode);
	SetConsoleMode(d_stdin, 0); }


Console::~Console() {
	SetConsoleMode(d_stdin, oldConsoleInputMode); }


Console& Console::GetInstance() {
	auto hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	auto hStdin = GetStdHandle(STD_INPUT_HANDLE);
	static Console console{ hStdout, hStdin };
	return console; }


void Console::MapKey(uint16_t scanCode, char ch) {
	d_remap[scanCode] = ch; }


void Console::UnmapKey(uint16_t scanCode) {
	d_remap.erase(scanCode); }


std::pair<int, int> Console::GetDimensions() {
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	if (GetConsoleScreenBufferInfo(d_stdout, &csbi) == 0) {
		throw std::runtime_error("GetConsoleScreenBufferInfo failure"); }
	return { csbi.dwSize.X, csbi.dwSize.Y }; }


void Console::SetDimensions(int width, int height) {
	const SMALL_RECT tiny{ 0, 0, 1, 1 };
	BOOL success = SetConsoleWindowInfo(d_stdout, TRUE, &tiny);
	if (success == 0) {
		throw std::runtime_error("SetConsoleWindowInfo tiny failure"); }

	COORD newSize;
	newSize.X = width;
	newSize.Y = height;
	if (SetConsoleScreenBufferSize(d_stdout, newSize) == 0) {
		throw std::runtime_error("SetConsoleScreenBufferSize failure"); }

	const SMALL_RECT window{ 0, 0, static_cast<short>(width-1), static_cast<short>(height-1) };
	success = SetConsoleWindowInfo(d_stdout, TRUE, &window);
	if (success == 0) {
		throw std::runtime_error("SetConsoleWindowInfo tiny failure"); }
	d_width = width;
	d_height = height; }


std::pair<int, int> Console::GetCursorPosition() {
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	if (GetConsoleScreenBufferInfo(d_stdout, &csbi) == 0) {
		throw std::runtime_error("GetConsoleScreenBufferInfo failure"); }
	return { csbi.dwCursorPosition.X, csbi.dwCursorPosition.Y }; }


Console& Console::ShowCursor() {
    COORD coord;
    coord.X = d_x; coord.Y = d_y;
    SetConsoleCursorPosition(d_stdout, coord);
	return *this; }


Console& Console::Write(const std::wstring& text) {
	const wchar_t* ptr = text.c_str();
	DWORD written;
	COORD destCoord{ SHORT(d_x), SHORT(d_y) };
	WriteConsoleOutputCharacterW(d_stdout, ptr, text.length(), destCoord, &written);
	AdvanceCursor(written);
	return *this; }


Console& Console::Write(const std::string& text) {
	auto tmp = rclt::UTF8Codec::Decode(text);
	return Write(tmp); }


Console& Console::Clear() {
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	const COORD coordScreen{ 0, 0 };
	DWORD written;
	FillConsoleOutputCharacterW(d_stdout, TEXT(' '), d_width*d_height, coordScreen, &written);
	GetConsoleScreenBufferInfo(d_stdout, &csbi);
	FillConsoleOutputAttribute(d_stdout, MakeAttribute(Color::Black, Color::White), d_width*d_height, coordScreen, &written);
	SetConsoleCursorPosition(d_stdout, coordScreen);
	return *this; }


}  // namespace rcls
}  // namespace rqdq
