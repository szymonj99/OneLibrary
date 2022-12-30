#ifdef OS_WINDOWS

#include <OneLibrary/KeyboardReceiver.h>

#define DEBUG_KEYBOARD_INPUT 1

ol::KeyboardReceiver::KeyboardReceiver() {}

ol::KeyboardReceiver::~KeyboardReceiver() {}

void ol::KeyboardReceiver::KeyPress(const ol::KeyboardInt kScanCode, const ol::KeyboardInt kFlags)
{
#ifdef DEBUG_KEYBOARD_INPUT
	INPUT input{};
	input.type = INPUT_KEYBOARD;
	input.ki.wVk = kScanCode;
	input.ki.dwFlags = kFlags;
	input.ki.time = 0;

	// I'm not 100% sure how we will later implement the scan codes.
	// https://docs.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-keybdinput
	// input.ki.wScan;

	SendInput(1, &input, sizeof(input));
#endif
}

#endif