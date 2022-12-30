#pragma once

#include <cstdint>
#include <utility>

namespace ol
{
	using InputInt = int32_t; // Type of packet. e.g. Mouse, Keyboard, Keep Alive.
	using MouseInt = int32_t; // Mouse co-ordinate
	using MousePair = std::pair<MouseInt, MouseInt>; // Pair of mouse movement co-ordinates
	using KeyboardInt = uint32_t; // Keyboard key flag (pressed, not pressed etc.) or keycode
	using KeyboardPair = std::pair<KeyboardInt, KeyboardInt>; // A pair of the key and its state
	using VersionInt = uint32_t; // Versioning scheme type

	const uint16_t kDefaultPort = 40480;

#ifdef OS_WINDOWS
	const uint32_t kTimerTimeout = 1000;
#endif
}
