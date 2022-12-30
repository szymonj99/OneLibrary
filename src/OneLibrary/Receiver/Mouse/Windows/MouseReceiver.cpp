#ifdef OS_WINDOWS

#include <OneLibrary/MouseReceiver.h>

#define DEBUG_MOUSE_MOVE 1

ol::MouseReceiver::MouseReceiver(){}

ol::MouseReceiver::~MouseReceiver(){}

void ol::MouseReceiver::MoveMouseTo(const ol::MouseInt x, const ol::MouseInt y)
{
#ifdef DEBUG_MOUSE_MOVE
	INPUT input{};
	input.type = INPUT_MOUSE;
	input.mi.dx = x;
	input.mi.dy = y;
	input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
	input.mi.time = 0;
	SendInput(1, &input, sizeof(input));
#endif
}

void ol::MouseReceiver::MoveMouseRelative(const ol::MouseInt x, const ol::MouseInt y)
{
#ifdef DEBUG_MOUSE_MOVE
	INPUT input{};
	input.type = INPUT_MOUSE;
	input.mi.dx = x;
	input.mi.dy = y;
	input.mi.dwFlags = MOUSEEVENTF_MOVE;
	input.mi.time = 0;
	SendInput(1, &input, sizeof(input));
#endif
}

#endif
