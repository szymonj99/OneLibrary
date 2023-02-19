#pragma once

#include <cstdint>

namespace ol
{
    // On Windows, this is used to know what type of messages we get from hooks.
	enum class eInputType : uint16_t
	{
        // In the future, we could implement something like `gamepad`, `steering wheel`, `bluetooth controller` etc.
		None = 0,
        Mouse,
		Keyboard
	};

    // The problem:
    // A user can click and hold a button.
    // Therefore, we need separate events for when a button is clicked and when it is released
    enum class eEventType : uint16_t
    {
        None = 0,
        MMove,
        MButtonLeftDown,
        MButtonLeftUp,
        MButtonRightDown,
        MButtonRightUp,
        MButtonMiddleDown,
        MButtonMiddleUp,
        MScroll,
        KBKeyDown,
        KBKeyUp
    };
}
