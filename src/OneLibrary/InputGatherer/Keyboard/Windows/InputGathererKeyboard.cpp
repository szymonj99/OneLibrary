#ifdef OS_WINDOWS

#include <OneLibrary/InputGathererKeyboard.h>

/**
 * Create an InputGatherer responsible for gathering input from the keyboard.
 * @param allowConsuming Specifies if the InputGatherer can consume input and prevent it from being passed further, or not.
 */
ol::InputGathererKeyboard::InputGathererKeyboard(const bool allowConsuming)
{
	this->m_bAllowConsuming = allowConsuming;
    this->_init();
}

void ol::InputGathererKeyboard::_init()
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

void ol::InputGathererKeyboard::_terminate()
{
    if (this->m_bAllowConsuming)
    {
        this->m_EndHook();
    }
    else
    {
        this->m_EndRawInput();
    }
    // TODO: Check if this is okay when the thread is not initialised.
    if (this->m_thInputGatherThread.joinable()) { this->m_thInputGatherThread.join(); }
}

ol::InputGathererKeyboard::~InputGathererKeyboard()
{
    this->_terminate();
}

ol::Input ol::InputGathererKeyboard::GatherInput()
{
    return ol::InputGathererKeyboard::m_bufInputs.Get();
}

void ol::InputGathererKeyboard::m_StartHook()
{
    this->m_thInputGatherThread = std::thread([&]
    {
        // This thread does GetMessage and sends the input received to the queue.
        // PeekMessage and SetWindowsHookEx needs to be called in the same thread that will call off to GetMessage
        // Let's try doing this without the timer initially and see if we are timing out at all.
        MSG msg {};
        // Do I need to do this for Raw Input as well?
        // Force the system to create a message queue.
        // https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-postthreadmessagea
        PeekMessage(&msg, nullptr, WM_USER, WM_USER, PM_NOREMOVE);
        this->m_pHook = SetWindowsHookEx(WH_KEYBOARD_LL, &ol::InputGathererKeyboard::LowLevelHookProcedure, nullptr, 0);

        while (true)
        {
            // In theory, this is all we need to do.
            this->m_WaitForLowLevelHook();
        }
    });
}

// Okay so...
// Weird thing about this is:
// After the refactor to use a thread that calls this separately, we for some reason don't ever
// Actually call std::cout << ... anything as messages come through.
// However, our hook still receives the messages, and as far as I can tell, doesn't time out.
// Unless this is happening as I didn't configure this correctly?
// We also can't remove this, as we need to call off to GetMessage.
void ol::InputGathererKeyboard::m_WaitForLowLevelHook()
{
    MSG msg{};
    // See the comments above the GetMessage call for Raw Input.
    // Here, the filter seems to work just fine.
    // However, in Raw Input, I had to use WM_KEYFIRST - 1 or else we didn't get the input events.
    // How? I have no idea.
    // Is that a Windows bug, or poor implementation by me?
    if (GetMessage(&msg, nullptr, WM_KEYFIRST, WM_KEYLAST) > 0)
    {
        // Do we need to call TranslateMessage?
        // In theory, yes but let's try doing it without it.
        // Could be better that way.
        DispatchMessage(&msg);
    }
    else
    {
        // TODO: Potentially implement an error handler of some sorts.
        // Maybe a function like CallError(RawInput) or CallError(LowLevelHook) ?
        std::cerr << "Error occurred when getting Low Level Hook input from a keyboard" << std::endl;
        PostQuitMessage(0);
        std::exit(0);
    }
}

LRESULT CALLBACK ol::InputGathererKeyboard::LowLevelHookProcedure(const int nCode, const WPARAM wParam, const LPARAM lParam)
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

    const auto hookStruct = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
    ol::Input input{};
    input.inputType = ol::eInputType::Keyboard;

    switch (wParam)
    {
    case WM_KEYDOWN:
        input.eventType = ol::eEventType::KBKeyDown;
        input.keyboard.key = ol::WindowsToKeyCode.at(hookStruct->vkCode);
        break;
    case WM_KEYUP:
        input.eventType = ol::eEventType::KBKeyUp;
        input.keyboard.key = ol::WindowsToKeyCode.at(hookStruct->vkCode);
        break;
    default:
        std::cerr << "Got Unhandled Message: hex: 0x" << wParam << "\n";
        // Let's not consume an input that we haven't handled and instead just pass it.
        return CallNextHookEx(nullptr, nCode, wParam, lParam);
    }

    // Should we potentially do something like PostMessage or PostThreadMessage
    // So that in the event of a slow-down in the queue, the hook would not slow the system down?
    // I think with a queue length that we have specified of 4 billion, it won't be a problem.
    ol::InputGathererKeyboard::m_bufInputs.Add(input);


    // if return 1, the mouse movements will NOT be passed along further
    // if return CallNextHookEx(0, nCode, wParam, lParam), the movement will be passed along further
    return ol::InputGathererKeyboard::m_bConsuming ? 1 : CallNextHookEx(nullptr, nCode, wParam, lParam);
}

void ol::InputGathererKeyboard::m_EndHook()
{
	if (this->m_pHook)
	{
		UnhookWindowsHookEx(this->m_pHook);
	}
}

void ol::InputGathererKeyboard::m_StartRawInput()
{
    // Note to self:
    // We need to create the window on the same thread that we call GetMessage from.
    this->m_thInputGatherThread = std::thread([&]
    {
        this->m_wRawInputWindowClass.hInstance = (HINSTANCE)1;
        this->m_wRawInputWindowClass.lpszClassName = "OneControl - Keyboard Procedure";
        this->m_wRawInputWindowClass.lpfnWndProc = ol::InputGathererKeyboard::RawInputProcedure;
        RegisterClass(&this->m_wRawInputWindowClass);
        // Create message window:
        // Invisible window that we use to get raw input messages.
        this->m_hRawInputMessageWindow = CreateWindow(this->m_wRawInputWindowClass.lpszClassName, nullptr, 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, this->m_wRawInputWindowClass.hInstance, nullptr);
        RAWINPUTDEVICE rawInput[1]{};
        rawInput[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
        rawInput[0].usUsage = HID_USAGE_GENERIC_KEYBOARD;
        // RIDEV_INPUTSINK is used so that we can get messages from the raw input device even when the application is no longer in focus.
        rawInput[0].dwFlags = RIDEV_INPUTSINK;
        rawInput[0].hwndTarget = this->m_hRawInputMessageWindow;
        if (!RegisterRawInputDevices(rawInput, 1, sizeof(rawInput[0])))
        {
            std::cerr << "Registering of raw input devices failed.\n";
            std::exit(-1);
        }

        // Although I'm not sure if this is necessary for raw input, we do what we did for low level hooks.
        // We force the system to create a message queue on this thread.
        MSG msg{};
        PeekMessage(&msg, this->m_hRawInputMessageWindow, WM_USER, WM_USER, PM_NOREMOVE);
        while (true)
        {
            this->m_WaitForRawInput();
        }
    });
}

void ol::InputGathererKeyboard::m_WaitForRawInput()
{
    MSG msg{};
    // Using the raw input message window here is okay as we have registered it as a RIDEV_INPUTSINK
    // Unlike Low Level hooks, where the hWnd has to be nullptr.
    // I love Windows. (Unless it's my janky programming...?)
    // In the docs the minimum and maximum filter are described as inclusive. However...
    // If we use WM_KEYFIRST, we do NOT get messages when a key is pressed.
    // If we set the minimum to WM_KEYFIRST - 1, we do...
    // Setting both parameters to 0 also works.
    if (GetMessage(&msg, this->m_hRawInputMessageWindow, WM_KEYFIRST - 1, WM_KEYLAST) > 0)
    {
        // Note: Calling TranslateMessage(&msg); is only necessary for keyboard input.
        // https://learn.microsoft.com/en-us/windows/win32/learnwin32/window-messages
        // However, let's try not using it for the time being.
        // Hand off every message to our Raw Input Procedure
        DispatchMessage(&msg);
    }
    else
    {
        std::cerr << "Error occurred when getting Raw Input from a keyboard" << std::endl;
        PostQuitMessage(0);
        std::exit(0);
    }
}

// TODO: Ensure there is a consistent coding style for switch statements.
// Some have brackets, some don't.
LRESULT CALLBACK ol::InputGathererKeyboard::RawInputProcedure(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam)
{
    switch (message)
    {
    case WM_INPUT:
    {
        UINT dwSize = 0;
        GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, nullptr, &dwSize, sizeof(RAWINPUTHEADER));
        const auto lpb = std::make_shared<BYTE[]>(dwSize);
        if (!lpb || (GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, lpb.get(), &dwSize, sizeof(RAWINPUTHEADER)) != dwSize))
        {
            std::cerr << "Raw Input Data size is not correct (keyboard)" << std::endl;
            return 0;
        }

        const auto rawInput = reinterpret_cast<RAWINPUT*>(lpb.get());

        // In theory, this is not necessary as we only registered this InputGatherer to receive Keyboard events.
        // TODO: Maybe add an `assert` call?
        if (rawInput->header.dwType != RIM_TYPEKEYBOARD) { return 0; }

        ol::Input input{};
        input.inputType = ol::eInputType::Keyboard;

        // Note to self:
        // In Low Level Hooks, we use the wParam to figure out what the message type is.
        // For example, wParam can be WM_KEYDOWN.
        // In Raw Input however... We don't use that. Thanks for being consistent, Windows.
        // It seems that wParam is always 1 when an input event happens.
        // Possible correction: wParam might be the value of the key being pressed.
        // Let's not worry about it for now and carry on with the explanation below.
        // Instead:
        // We use rawInput->data.keyboard.Message:
        // In that case, we can compare .Message to WM_KEYDOWN or WM_KEYUP.
        // OR
        // We can compare rawInput->data.keyboard.Flags to 0 (Key Down), or 1 (Key Up)
        // Both of these are viable options.
        switch (rawInput->data.keyboard.Message)
        {
            case WM_KEYDOWN:
                input.eventType = ol::eEventType::KBKeyDown;
                input.keyboard.key = ol::WindowsToKeyCode.at(rawInput->data.keyboard.VKey);
                break;
            case WM_KEYUP:
                input.eventType = ol::eEventType::KBKeyUp;
                input.keyboard.key = ol::WindowsToKeyCode.at(rawInput->data.keyboard.VKey);
                break;
            // TODO: Add in cases for Message 260 and 261 (I think those come from special keys like Ctrl, Alt etc. but I'm not 100% sure.)
            // TODO: Add in cases for Message 7865344 and 4287104000 (Scroll up and scroll down)
            [[unlikely]]
            default:
                std::cerr << "Got Unhandled Message: (int) " << rawInput->data.keyboard.Message << "\n";
                return 0;
        }

        // Should we potentially do something like PostMessage or PostThreadMessage
        // So that in the event of a slow-down in the queue, the hook would not slow the system down?
        // I think with a queue length that we have specified of 4 billion, it won't be a problem.
        ol::InputGathererKeyboard::m_bufInputs.Add(input);

        return 0;
    }
    case WM_CLOSE:
    {
        PostQuitMessage(0);
        return 0;
    }
    default:
    {
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    }
    return 0;
}

void ol::InputGathererKeyboard::m_EndRawInput()
{
    RAWINPUTDEVICE rawInput[1]{};

    rawInput[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
    rawInput[0].usUsage = HID_USAGE_GENERIC_KEYBOARD;
    rawInput[0].dwFlags = RIDEV_REMOVE; // Stop this application from receiving more input events

    // Note: Do not use `rawInput[0].hwndTarget = this->m_hRawInputMessageWindow;`
    // Doing so results in the device not being unregistered correctly.
    rawInput[0].hwndTarget = nullptr;

    // This acts as unregistering the raw input device: https://gamedev.net/forums/topic/629795-unregistering-raw-input/4970645/
    if (!RegisterRawInputDevices(rawInput, 1, sizeof(rawInput[0])))
    {
        std::cerr << "Unregistering of raw input devices failed.\n";
        // TODO: Properly define error codes.
        std::exit(-1);
    }
}

#endif
