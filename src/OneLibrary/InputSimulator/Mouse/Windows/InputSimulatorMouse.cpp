#ifdef OS_WINDOWS

#include <OneLibrary/InputSimulatorMouse.h>

ol::InputSimulatorMouse::InputSimulatorMouse() {}

ol::InputSimulatorMouse::~InputSimulatorMouse() {}

void ol::InputSimulatorMouse::PerformInput(const ol::Input& kInput)
{
    INPUT input{};
    input.type = INPUT_MOUSE;

    input.mi.mouseData = 0;
    input.mi.dwExtraInfo = 0;
    input.mi.time = 0;

    // TODO: Figure out if slower-polling mice have issues with actually having let's say,
    // a mouse move event as well as data for e.g. scrolling in the same event.
    // I think Windows would split it into individual events, but I'm not 100% sure.
    switch (kInput.eventType)
    {
        [[likely]]
        case ol::eEventType::MMove:
            input.mi.dx = kInput.mouse.x;
            input.mi.dy = kInput.mouse.y;
            // MOUSEEVENTF_MOVE_NOCOALESCE = Mouse movement events will not be joined together, and will instead
            // Remain as individual events.
            //input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSE_MOVE_RELATIVE;
            input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSE_MOVE_RELATIVE | MOUSEEVENTF_MOVE_NOCOALESCE;
            break;
        case ol::eEventType::MButtonLeftDown:
            input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
            break;
        case ol::eEventType::MButtonLeftUp:
            input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
            break;
        case ol::eEventType::MButtonRightDown:
            input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
            break;
        case ol::eEventType::MButtonRightUp:
            input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
            break;
        case ol::eEventType::MButtonMiddleDown:
            input.mi.dwFlags = MOUSEEVENTF_MIDDLEDOWN;
            break;
        case ol::eEventType::MButtonMiddleUp:
            input.mi.dwFlags = MOUSEEVENTF_MIDDLEUP;
            break;
        case ol::eEventType::MScroll:
            input.mi.mouseData = kInput.mouse.scroll;
            input.mi.dwFlags = MOUSEEVENTF_WHEEL;
            break;
        [[unlikely]]
        default:
            // I wonder if it'd be worth it to std::cerr << error; here, or something similar to let the user know.
            break;
    }

    SendInput(1, &input, sizeof(INPUT));
}

#endif
