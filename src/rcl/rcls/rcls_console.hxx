#pragma once
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

namespace rqdq {
namespace rcls {

constexpr uint32_t kCKRightAlt = 0x01;
constexpr uint32_t kCKLeftAlt = 0x02;
constexpr uint32_t kCKRightCtrl = 0x04;
constexpr uint32_t kCKLeftCtrl = 0x08;
constexpr uint32_t kCKShift = 0x10;

enum ScanCode {
	Esc = 1,
	Key1 = 2,
	Key2 = 3,
	Key3 = 4,
	Key4 = 5,
	Key5 = 6,
	Key6 = 7,
	Key7 = 8,
	Key8 = 9,
	Key9 = 10,
	Key0 = 11,
	Minus = 12,
	Equals = 13,
	Backspace = 14,
	Tab = 15,
	Q = 16,
	W = 17,
	E = 18,
	R = 19,
	T = 20,
	Y = 21,
	U = 22,
	I = 23,
	O = 24,
	P = 25,
	OpenBracket = 26,
	CloseBracket = 27,
	Enter = 28,
	LeftCtrl = 29,
	A = 30,
	S = 31,
	D = 32,
	F = 33,
	G = 34,
	H = 35,
	J = 36,
	K = 37,
	L = 38,
	Semicolon = 39,
	Quote = 40,
	Tilde = 41,
	LeftShift = 42,
	Backslash = 43,
	Z = 44,
	X = 45,
	C = 46,
	V = 47,
	B = 48,
	N = 49,
	M = 50,
	Comma = 51,
	Period = 52,
	ForwardSlash = 53,
	RightShift = 54,
	// 55 ?
	Alt = 56,  // same scancode for left&right? but flags are different... strange
	Space = 57,
	// 58 ?
	F1 = 59,
	F2 = 60,
	F3 = 61,
	F4 = 62,
	F5 = 63,
	F6 = 64,
	F7 = 65,
	F8 = 66,
	F9 = 67,
	F10 = 68,
	// ...
	Home = 71,
	CursorUp = 72,
	PageUp = 73,
	//
	CursorLeft = 75,
	//
	CursorRight = 77,
	//
	End = 79,
	CursorDown = 80,
	PageDown = 81,
	Insert = 82,
	Delete = 83,
    // ...
	F11 = 87,
	F12 = 88,
	LeftWindows = 91, };


enum BoxChar {
	lowerLeft = 0xc0,
	lowerRight = 0xd9,
	upperLeft = 0xda,
	upperRight = 0xbf,
	horizontalLine = 0xc4,
	verticalLine = 0xb3,
	leftTee = 0xb4,
	rightTee = 0xc3 };


class Console {
	// lifetime
private:
	Console(void*, void*);
	~Console();
	Console& operator=(const Console&) = delete;
	Console(const Console&) = delete;
public:
	static Console& GetInstance();

	// size, cursor, buffer management
	void SetDimensions(int width, int height);
	std::pair<int, int> GetDimensions();
	std::pair<int, int> GetCursorPosition();
	void MapKey(uint16_t scanCode, char ch);
	void UnmapKey(uint16_t scanCode);

	// drawing api
	Console& Position(int x, int y) { d_x = x; d_y = y; return *this; }
	Console& LeftEdge(int x) { d_leftEdge = x; return *this; }
	Console& CR() { d_x = d_leftEdge; d_y++; return *this; }
	Console& ShowCursor();
	Console& Write(const std::wstring& text);
	Console& Write(const std::string& text);
	Console& Write(const std::wstring& text, char attr);
	Console& Write(const std::string& text, char attr);
	Console& Clear();

	Console& PushPosition() {
		d_positionStack.emplace_back(d_x, d_y);
		return *this; }
	Console& PopPosition() {
		std::tie(d_x, d_y) = d_positionStack.back();
		d_positionStack.pop_back();
		return *this; }
	Console& AdvanceCursor(int n) {
		d_y += (d_x + n) / d_width;
		d_x = (d_x + n) % d_width;
		return *this; }

private:
	std::vector<std::pair<int, int>> d_positionStack;
	int d_x = 0;
	int d_y = 0;
	int d_height = 0;
	int d_width = 0;
	int d_leftEdge = 0;
	void* d_stdout = nullptr;
	void* d_stdin = nullptr;
	std::unordered_map<uint16_t, int> d_remap;
	std::vector<bool> d_activeKeys; };


}  // namespace rcls
}  // namespace rqdq
