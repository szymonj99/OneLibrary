#ifdef OS_WINDOWS

#include <OneLibrary/InputSimulatorKeyboard.h>

void ol::InputSimulatorKeyboard::PerformInput(const ol::Input& kInput)
{
    INPUT input{};
    input.type = INPUT_KEYBOARD;

    input.ki.wVk = kInput.keyboard.key;
    input.ki.wScan = 0; // Not 100% sure about this. Would we use this in the future?
    input.ki.time = 0;
    input.ki.dwExtraInfo = 0; // Again, not needed at all as we don't provide extra info.

    switch (kInput.eventType)
    {
        case ol::eEventType::KBKeyDown:
            input.ki.dwFlags = 0; // Pressed down
            break;
        case ol::eEventType::KBKeyUp:
            input.ki.dwFlags = KEYEVENTF_KEYUP;
            break;
        default:
            break;
    }

    SendInput(1, &input, sizeof(INPUT));
}

#endif
