#ifdef OS_WINDOWS

#include <OneLibrary/InputGathererMouse.h>

ol::InputGathererMouse::InputGathererMouse(const bool kAllowConsuming)
{
    this->m_bAllowConsuming = kAllowConsuming;
    this->_init();
}

void ol::InputGathererMouse::_init()
{
    // TODO: Potentially add in a check to see if m_pHook or the Raw Input Window are null.
    // That would allow this class to be extended by a third-party without having to worry about implementation intricacies.
    if (this->m_bAllowConsuming)
    {
        this->m_StartHook();
    }
    else
    {
        this->m_StartRawInput();
    }
}

void ol::InputGathererMouse::_terminate()
{
    if (this->m_bAllowConsuming)
    {
        PostThreadMessageW(GetThreadId(this->m_thInputGatherThread.native_handle()), WM_QUIT, reinterpret_cast<WPARAM>(nullptr), reinterpret_cast<LPARAM>(nullptr));
        this->m_EndHook();
    }
    else
    {
        PostMessageW(this->m_hRawInputMessageWindow, WM_QUIT, reinterpret_cast<WPARAM>(nullptr), reinterpret_cast<LPARAM>(nullptr));
        this->m_EndRawInput();
    }
    // TODO: Check if this is okay when the thread is not initialised.
    if (this->m_thInputGatherThread.joinable()) { this->m_thInputGatherThread.join(); }
}

ol::InputGathererMouse::~InputGathererMouse()
{
    this->_terminate();
}

ol::Input ol::InputGathererMouse::GatherInput()
{
    return ol::InputGathererMouse::m_bufInputs.Get();
}

void ol::InputGathererMouse::m_StartHook()
{
    std::binary_semaphore threadInitialised{0};
    this->m_thInputGatherThread = std::thread([&]
    {
        // This thread does GetMessage and sends the input received to the queue.
        // PeekMessage and SetWindowsHookEx needs to be called in the same thread that will call off to GetMessage
        // Let's try doing this without the timer initially and see if we are timing out at all.

        MSG msg {};
        // Do I need to do this for Raw Input as well?
        // Force the system to create a message queue.
        // https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-postthreadmessagea
        PeekMessageW(&msg, nullptr, WM_USER, WM_USER, PM_NOREMOVE);
        this->m_pHook = SetWindowsHookExW(WH_MOUSE_LL, &ol::InputGathererMouse::LowLevelHookProcedure, nullptr, 0);
        threadInitialised.release();
        while (ol::InputGathererMouse::m_bRunning) { this->m_WaitForLowLevelHook(); }
    });
    threadInitialised.acquire();
}

void ol::InputGathererMouse::m_WaitForLowLevelHook()
{
    MSG msg {};
    const auto kResult = GetMessageW(&msg, nullptr, WM_QUIT, WM_MOUSELAST);
    if (kResult > 0)
    {
        DispatchMessageW(&msg);
    }
    else if (kResult == 0) // WM_QUIT message
    {
        ol::InputGathererMouse::m_bRunning = false;
        return;
    }
    else
    {
        // Maybe a function like CallError(RawInput) or CallError(LowLevelHook) ?
        std::cerr << "Error occurred when getting Low Level Hook input from a mouse" << std::endl;
        PostQuitMessage(0);
        std::exit(0);
    }
}

// Note to self:
// This can only be called by Windows when we are in a GetMessage loop.
// Makes sense. Otherwise, Windows doesn't have control of our program according to someone that can explain this better than I can, on StackOverflow.
LRESULT CALLBACK ol::InputGathererMouse::LowLevelHookProcedure(const int nCode, const WPARAM wParam, const LPARAM lParam)
{
    // HC_ACTION is actually defined as 0, so this could be simplified,
    // but `barrier` uses HC_ACTION only without checking if `nCode < 0`
    // Windows Docs use:
    // `If nCode is less than zero, the hook procedure must pass the message to the CallNextHookEx function without further processing and should return the value returned by CallNextHookEx.`
    if (nCode != HC_ACTION)
    {
        return CallNextHookEx(nullptr, nCode, wParam, lParam);
    }

    // What we could potentially do in the future is:
    // PostMessage or PostThreadMessage with the lParam and wParam.
    // That way we exit out of this function ASAP as advised by Microsoft,
    // And hopefully not resulting in performance degradation because of a low level hook.

    const auto hookStruct = reinterpret_cast<MSLLHOOKSTRUCT*>(lParam);
    ol::Input input{};
    input.inputType = ol::eInputType::Mouse;

    // TODO: Handling all of these individually I think would be the only correct way, as
    // MSLLHOOKSTRUCT->mouseData differs between messages
    // https://learn.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-msllhookstruct?redirectedfrom=MSDN
    // TODO: Investigate WM_QUIT etc.
    switch (wParam)
    {
        // TODO: Add in some logging using spdlog or similar.
        // When we have a mouse button clicked, we need to look at mouseData
        // Otherwise we look at x and y.
        // I wonder if we can label this as `[[likely]]` as there will likely be a lot more of mouse movement than other actions.
        [[likely]]
        case WM_MOUSEMOVE:
        {
            POINT point{};
            GetCursorPos(&point);

            input.eventType = ol::eEventType::MMove;
            input.mouse.x = hookStruct->pt.x - point.x;
            input.mouse.y = hookStruct->pt.y - point.y;
            break;
        }
        case WM_LBUTTONDOWN:
        {
            input.eventType = ol::eEventType::MButtonLeftDown;
            break;
        }
        case WM_LBUTTONUP:
        {
            input.eventType = ol::eEventType::MButtonLeftUp;
            break;
        }
        case WM_MOUSEWHEEL:
        {
            // TODO: Fix this: const auto fwKeys = GET_KEYSTATE_WPARAM(hookStruct->mouseData);
            const auto zDelta = GET_WHEEL_DELTA_WPARAM(hookStruct->mouseData);

            //std::cout << "fwKeys: " << fwKeys << "\n";

            input.eventType = ol::eEventType::MScroll;

            // The wheel delta on Windows is 120 by default.
            // How do we want to handle this?
            // Increment by 1 for every delta, or do we allow for more granularity?
            input.mouse.scroll = zDelta;

            break;
        }
        case WM_RBUTTONDOWN:
        {
            input.eventType = ol::eEventType::MButtonRightDown;
            break;
        }
        case WM_RBUTTONUP:
        {
            input.eventType = ol::eEventType::MButtonRightUp;
            break;
        }
        case WM_MBUTTONDOWN:
        {
            input.eventType = ol::eEventType::MButtonMiddleDown;
            break;
        }
        case WM_MBUTTONUP:
        {
            input.eventType = ol::eEventType::MButtonMiddleUp;
            break;
        }
        // I think we don't need to handle `case WM_MOUSEHWHEEL:`
        [[unlikely]]
        default:
        {
            std::cerr << "Got Unhandled Message: (int) " << wParam << "\n";
            // Let's not consume an input that we haven't handled and instead just pass it.
            return CallNextHookEx(nullptr, nCode, wParam, lParam);
        }
    }

    // Should we potentially do something like PostMessage or PostThreadMessage
    // So that in the event of a slow-down in the queue, the hook would not slow the system down?
    // I think with a queue length that we have specified of 4 billion, it won't be a problem.
    if (ol::InputGathererMouse::m_bGathering)
    {
        ol::InputGathererMouse::m_bufInputs.Add(input);
    }

    // if return 1, the mouse movement will be captured and not passed to further windows.
    // if return CallNextHookEx(0, nCode, wParam, lParam), the movement will be passed further to other windows.
    return ol::InputGathererMouse::m_bConsuming ? 1 : CallNextHookEx(nullptr, nCode, wParam, lParam);
}

void ol::InputGathererMouse::m_EndHook()
{
    if (this->m_pHook)
    {
        UnhookWindowsHookEx(this->m_pHook);
    }
}

void ol::InputGathererMouse::m_StartRawInput()
{
    std::binary_semaphore rawInputInitialised{0};
    this->m_thInputGatherThread = std::thread([&]
    {
        this->m_wRawInputWindowClass.hInstance = nullptr;
        this->m_wRawInputWindowClass.lpszClassName = L"OneControl - Mouse Procedure";
        this->m_wRawInputWindowClass.lpfnWndProc = ol::InputGathererMouse::RawInputProcedure;
        RegisterClassW(&this->m_wRawInputWindowClass);

        // Create message window:
        // Invisible window that we use to get raw input messages.
        this->m_hRawInputMessageWindow = CreateWindowW(this->m_wRawInputWindowClass.lpszClassName, nullptr, 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, this->m_wRawInputWindowClass.hInstance, nullptr);

        RAWINPUTDEVICE rawInput[1]{};

        rawInput[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
        rawInput[0].usUsage = HID_USAGE_GENERIC_MOUSE;
        // RIDEV_INPUTSINK is used so that we can get messages from the raw input device even when the application is no longer in focus.
        // MOUSE_MOVE_RELATIVE is defined as 0, but I add it here for clarity.
        // Also don't group inputs together. --- Note, this actually fails registering the raw input device. For some reason.
        //rawInput[0].dwFlags = RIDEV_INPUTSINK | MOUSE_MOVE_RELATIVE | MOUSE_MOVE_NOCOALESCE;
        rawInput[0].dwFlags = RIDEV_INPUTSINK | MOUSE_MOVE_RELATIVE;
        rawInput[0].hwndTarget = this->m_hRawInputMessageWindow;

        if (!RegisterRawInputDevices(rawInput, 1, sizeof(rawInput[0])))
        {
            std::cerr << "Registering of raw input devices failed.\n";
            std::exit(-1);
        }

        // Although I'm not sure if this is necessary for raw input, we do what we did for low level hooks.
        // We force the system to create a message queue on this thread.
        MSG msg{};
        PeekMessageW(&msg, this->m_hRawInputMessageWindow, WM_USER, WM_USER, PM_NOREMOVE);
        rawInputInitialised.release();
        while (ol::InputGathererMouse::m_bRunning) { this->m_WaitForRawInput(); }
    });
    rawInputInitialised.acquire();
}

void ol::InputGathererMouse::m_WaitForRawInput()
{
    MSG msg {};
    // Using the raw input message window here is okay as we have registered it as a RIDEV_INPUTSINK
    // Unlike Low Level hooks, where the hWnd has to be nullptr.
    const auto kResult = GetMessageW(&msg, this->m_hRawInputMessageWindow, WM_QUIT, WM_INPUT);

    if (kResult > 0)
    {
        // Note: Calling TranslateMessage(&msg); is only necessary for keyboard input.
        // https://learn.microsoft.com/en-us/windows/win32/learnwin32/window-messages
        // Hand off every message to our Raw Input Procedure
        DispatchMessageW(&msg);
    }
    else if (kResult == 0) // WM_QUIT message
    {
        ol::InputGathererMouse::m_bRunning = false;
        return;
    }
    else
    {
        // TODO: Add error handling and correct error codes here.
        std::cerr << "Error occurred when getting Raw Input from a mouse" << std::endl;
        PostQuitMessage(0);
        std::exit(0);
    }
}

LRESULT CALLBACK ol::InputGathererMouse::RawInputProcedure(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam)
{
    // Message seems to always be 255 (WM_INPUT, 0xFF)
    // And wParam seems to always be 1.
    // TODO: Investigate WM_QUIT, WM_DESTROY and WM_CLOSE messages here.
    switch (message)
    {
        case WM_INPUT:
        {
            UINT dwSize = 0;
            GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, nullptr, &dwSize, sizeof(RAWINPUTHEADER));
            const auto lpb = std::make_shared<BYTE[]>(dwSize);
            if (!lpb || (GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, lpb.get(), &dwSize,
                                         sizeof(RAWINPUTHEADER)) != dwSize)) {
                std::cerr << "Raw Input Data size is not correct (mouse)" << std::endl;
                return 0;
            }

            const auto rawInput = reinterpret_cast<RAWINPUT*>(lpb.get());
            // No need to check if raw->header.dwType == RIM_TYPEMOUSE
            // As we have registered the raw input device as a mouse only.
            // In theory, this is not necessary as we only registered this InputGatherer to receive Keyboard events.
            // TODO: Maybe add an `assert` call?
            if (rawInput->header.dwType != RIM_TYPEMOUSE) { return 0; }

            ol::Input input{};
            input.inputType = ol::eInputType::Mouse;

            switch (rawInput->data.mouse.ulButtons)
            {
                // This is actually mouse movement.
                // Raw input doesn't define a RI_ for it though.
                case 0:
                    input.mouse.x = rawInput->data.mouse.lLastX;
                    input.mouse.y = rawInput->data.mouse.lLastY;
                    input.eventType = ol::eEventType::MMove;
                    break;
                case RI_MOUSE_LEFT_BUTTON_DOWN:
                    input.eventType = ol::eEventType::MButtonLeftDown;
                    break;
                case RI_MOUSE_LEFT_BUTTON_UP:
                    input.eventType = ol::eEventType::MButtonLeftUp;
                    break;
                case RI_MOUSE_RIGHT_BUTTON_DOWN:
                    input.eventType = ol::eEventType::MButtonRightDown;
                    break;
                case RI_MOUSE_RIGHT_BUTTON_UP:
                    input.eventType = ol::eEventType::MButtonRightUp;
                    break;
                case RI_MOUSE_MIDDLE_BUTTON_DOWN:
                    input.eventType = ol::eEventType::MButtonMiddleDown;
                    break;
                case RI_MOUSE_MIDDLE_BUTTON_UP:
                    input.eventType = ol::eEventType::MButtonMiddleUp;
                    break;
                    // There are more events such as RI_MOUSE_BUTTON_4_DOWN/UP and RI_MOUSE_BUTTON_5_DOWN/UP
                    // But we won't worry about those until issues arise.
                case RI_MOUSE_WHEEL:
                    input.eventType = ol::eEventType::MScroll;
                    // TODO: Get scroll value here.
                    if ((rawInput->data.mouse.usButtonFlags & RI_MOUSE_WHEEL) == RI_MOUSE_WHEEL) {
                        // Microsoft Magic.
                        const auto wheelDelta = (float) (short) (rawInput->data.mouse.usButtonData);
                        std::cout << "Wheel Delta: " << wheelDelta << "\n";
                    }
                    else
                    {
                        std::cout << "The button flags don't match or something\n";
                        break;
                    }
                    break;
                // I won't worry about RI_MOUSE_HWHEEL for now, as that is a horizontal scroll wheel, whatever that is.
                [[unlikely]]
                default:
                {
                    std::cerr << "Got Unhandled Message: (int) " << rawInput->data.mouse.ulButtons << "\n";
                    // If we don't handle the input, notify other windows that this message has not been processed.
                    // Unlike low level hooks, raw input can't consume/swallow inputs anyway.
                    return DefWindowProc(hWnd, message, wParam, lParam);
                }
            }

            if (ol::InputGathererMouse::m_bGathering)
            {
                ol::InputGathererMouse::m_bufInputs.Add(input);
            }

            return 0;
        }
        default:
        {
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
}

void ol::InputGathererMouse::m_EndRawInput()
{
    RAWINPUTDEVICE rawInput[1]{};

    rawInput[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
    rawInput[0].usUsage = HID_USAGE_GENERIC_MOUSE;
    rawInput[0].dwFlags = RIDEV_REMOVE; // Stop this application from receiving more input events

    // Note: Do not use `rawInput[0].hwndTarget = this->m_hRawInputMessageWindow;`
    // Doing so results in the device not being unregistered correctly.
    rawInput[0].hwndTarget = nullptr;

    // This acts as unregistering the raw input device: https://gamedev.net/forums/topic/629795-unregistering-raw-input/4970645/
    if (!RegisterRawInputDevices(rawInput, 1, sizeof(rawInput[0])))
    {
        std::cerr << "Unregistering of raw input devices failed.\n";
        // TODO: Add enums for error messages
        std::exit(-1);
    }
}

void ol::InputGathererMouse::Toggle()
{
    ol::InputGathererMouse::m_bGathering = !ol::InputGathererMouse::m_bGathering;
    ol::InputGathererMouse::m_bConsuming = ol::InputGathererMouse::m_bGathering;
}

#endif
