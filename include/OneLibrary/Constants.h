#pragma once

#include <cstdint>
#include <utility>

namespace ol
{
	using InputInt = int32_t; // Type of packet. e.g. Mouse, Keyboard, Keep Alive.
	using MouseInt = int32_t; // Mouse co-ordinate
	using KeyboardInt = uint32_t; // Keyboard key flag (pressed, not pressed etc.) or keycode
}
