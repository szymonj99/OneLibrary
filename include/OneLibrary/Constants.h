#pragma once

#include <cstdint>
#include <utility>

namespace ol
{
	using InputInt = int16_t; // Type of packet. e.g. Mouse, Keyboard, Keep Alive.
	using MouseInt = int16_t; // Mouse co-ordinate
	using KeyboardInt = uint16_t; // Keyboard key flag (pressed, not pressed etc.) or keycode
}
