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

    // TODO: I wonder if we can support for example:
    // On Windows: Next Track 0xB0, Previous Track 0xB1, Stop Media 0xB2 etc. etc.
    // TODO: Check if we need to support LessThan and GreaterThan.
    // On Windows, BOTH of the keys <> have the same code, 0xE2.
    // So I assume that the extra data in the keyboard struct will store which one has been pressed.
    // However, these _should_ be handled by Comma and Period.
    // Platform-independent key code definitions.
    enum class eKeyCode : uint16_t
    {
        Backspace = 0,
        Tab,
        Clear,
        Enter,
        Shift,
        LeftShift,
        RightShift,
        Ctrl,
        LeftCtrl,
        RightCtrl,
        Alt,
        LeftAlt,
        RightAlt,
        Pause,
        CapsLock,
        Esc,
        Space,
        PageUp,
        PageDown,
        End,
        Home,
        LeftArrow,
        RightArrow,
        UpArrow,
        DownArrow,
        Select,
        Print,
        Execute,
        PrintScreen,
        Insert,
        Delete,
        Help,
        Number_0,
        Number_1,
        Number_2,
        Number_3,
        Number_4,
        Number_5,
        Number_6,
        Number_7,
        Number_8,
        Number_9,
        A,
        B,
        C,
        D,
        E,
        F,
        G,
        H,
        I,
        J,
        K,
        L,
        M,
        N,
        O,
        P,
        Q,
        R,
        S,
        T,
        U,
        V,
        W,
        X,
        Y,
        Z,
        WindowsLeft,
        WindowsRight,
        Applications,
        Numpad_0,
        Numpad_1,
        Numpad_2,
        Numpad_3,
        Numpad_4,
        Numpad_5,
        Numpad_6,
        Numpad_7,
        Numpad_8,
        Numpad_9,
        Multiply,
        Add,
        Separator,
        Subtract,
        Decimal,
        Divide,
        F1,
        F2,
        F3,
        F4,
        F5,
        F6,
        F7,
        F8,
        F9,
        F10,
        F11,
        F12,
        F13,
        F14,
        F15,
        F16,
        F17,
        F18,
        F19,
        F20,
        F21,
        F22,
        F23,
        F24,
        NumLock,
        ScrollLock,
        VolumeDown,
        VolumeUp,
        VolumeMute,
        LeftBracket,
        RightBracket,
        LeftSlash,
        RightSlash,
        SemiColon,
        Plus,
        Comma,
        Minus,
        Period,
        Tilde,
        Quote,
        Fn,
        NONE = 65535 // I love being inconsistent with None/NONE values being 0/65535 respectively. :)
    };

    // TODO: C++20 added ranges for enums. Add them for the enum classes we are using.
    // being() and end() functions, so we can call `for (const auto e : enum class)`.
}
