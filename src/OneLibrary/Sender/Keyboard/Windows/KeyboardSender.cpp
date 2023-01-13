#ifdef OS_WINDOWS

#include <OneLibrary/KeyboardSender.h>

// Note to self:
// - I have a feeling that this keyboard/mouse sender/receiver could be cleaned up a little using higher order functions and more.

// Let's add in an option to use either low level hooks or raw input.
// Low level hooks allow for input consumption whereas raw input doesn't.
// This `allowConsumption` option would only need to exist for the senders as the receivers can use the `SendInput` function.
// By default, I think we can set it to not allow consuming of input as that seems to be a somewhat specific usecase.
ol::KeyboardSender::KeyboardSender(bool allowConsuming = false)
{
	this->m_bAllowConsuming = allowConsuming;
	if (this->m_bAllowConsuming)
	{
		m_StartHook();
	}
	else
	{
		m_StartRawInput();
	}
}

void ol::KeyboardSender::m_StartHook()
{
	MSG msg;
	PeekMessage(&msg, nullptr, WM_USER, WM_USER, PM_NOREMOVE); // Force the system to create a message queue.
	m_pHook = SetWindowsHookEx(WH_KEYBOARD_LL, &ol::KeyboardSender::HookProc, nullptr, 0);
    // This will also act as a keep alive "feature".
    this->m_pTimer = std::make_unique<ol::MessageTimer>(ol::kTimerTimeout, ol::eThreadMessages::Keyboard, GetCurrentThreadId());
}

void ol::KeyboardSender::m_StartRawInput()
{
	// Raw input boilerplate code
	// Note to self:
	// - Do we need to register the window class, and to create a window?
	// - Or do we only need to define rawInput[0].hwndTarget = 0; ?
	// - I think so. Let's do more research in the future.

	// Window class
	this->m_wRawInputWindowClass.hInstance = (HINSTANCE)1;
	this->m_wRawInputWindowClass.lpszClassName = "OneControl - Keyboard Proc";
	this->m_wRawInputWindowClass.lpfnWndProc = ol::KeyboardSender::RawInputKeyboardProc;
	RegisterClass(&this->m_wRawInputWindowClass);

	// Create message window:
	// Invisible window that we use to get raw input messages.
	this->m_hRawInputMessageWindow = CreateWindow(this->m_wRawInputWindowClass.lpszClassName, 0, 0, 0, 0, 0, 0, HWND_MESSAGE, 0, this->m_wRawInputWindowClass.hInstance, 0);

	RAWINPUTDEVICE rawInput[1]{};

	rawInput[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
	rawInput[0].usUsage = HID_USAGE_GENERIC_KEYBOARD;
	// RIDEV_INPUTSINK is used so that we can get messages from the raw input device even when the application is no longer in focus.
	rawInput[0].dwFlags = RIDEV_INPUTSINK;
	rawInput[0].hwndTarget = this->m_hRawInputMessageWindow;

	if (!RegisterRawInputDevices(rawInput, 1, sizeof(rawInput[0])))
	{
		std::cout << "Registering of raw input devices failed.\n";
		//std::cin.get();
		std::exit(-1);
	}
}

ol::KeyboardSender::~KeyboardSender()
{
	if (this->m_bAllowConsuming)
	{
		m_EndHook();
	}
	else
	{
		m_EndRawInput();
	}
}

void ol::KeyboardSender::m_EndHook()
{
	if (m_pHook)
	{
		UnhookWindowsHookEx(m_pHook);
	}
}

void ol::KeyboardSender::m_EndRawInput()
{
	RAWINPUTDEVICE rawInput[1]{};

	rawInput[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
	rawInput[0].usUsage = HID_USAGE_GENERIC_KEYBOARD;
	rawInput[0].dwFlags = RIDEV_REMOVE; // Stop this application from receiving more input events

	// I am unsure if this needs to be 0 or if we can use this->m_hRawInputMessageWindow
	rawInput[0].hwndTarget = 0;

	// This acts as unregistering the raw input device: https://gamedev.net/forums/topic/629795-unregistering-raw-input/4970645/
	if (!RegisterRawInputDevices(rawInput, 1, sizeof(rawInput[0])))
	{
		std::cout << "Unregistering of raw input devices failed.\n";
		//std::cin.get();
		std::exit(-1);
	}
}

LRESULT CALLBACK ol::KeyboardSender::HookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode < 0)
	{
		return CallNextHookEx(0, nCode, wParam, lParam);
	}

	// if return 1, the mouse movements will NOT be passed along further
	// if return CallNextHookEx(0, nCode, wParam, lParam), the movement will be passed along further
	if (ol::KeyboardSender::SendToClient)
	{
		const auto hookStruct = *(KBDLLHOOKSTRUCT*)lParam;
        ol::Input input;
		input.type = ol::eInputType::Keyboard;
		input.keyboard.key = hookStruct.vkCode;
		input.keyboard.state = hookStruct.flags;

		PostThreadMessage(GetCurrentThreadId(), static_cast<UINT>(ol::eThreadMessages::Keyboard), 0, reinterpret_cast<LPARAM>(&input));

		return 1;
	}
	return CallNextHookEx(0, nCode, wParam, lParam);
}

ol::Input ol::KeyboardSender::m_GetHookData()
{
	MSG msg;
	while (GetMessage(&msg, 0, static_cast<UINT>(ol::eThreadMessages::Keyboard), static_cast<UINT>(ol::eThreadMessages::Keyboard)) > 0)
	{
		switch (msg.message)
		{
		case static_cast<UINT>(ol::eThreadMessages::Keyboard):
		{
			if ((ol::eThreadMessages)msg.lParam == ol::eThreadMessages::KeepAlive)
			{
				auto input = ol::Input();
				input.type = ol::eInputType::KeepAlive;
				return input;
			}

			const auto input = reinterpret_cast<ol::Input*>(msg.lParam);

			// Let's abort when something went wrong.
			if (input == nullptr)
			{
				auto failed = ol::Input();
				failed.type = ol::eInputType::Failed;
				return failed;
			}

			return *input;
		}

		case WM_QUIT:
		{
			auto input = ol::Input();
			input.type = ol::eInputType::HookStopped;
			return input;
		}
		default:
		{
			break;
		}
		}
	}

	return {};
}

LRESULT CALLBACK ol::KeyboardSender::RawInputKeyboardProc(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam)
{
	switch (message)
	{
	case WM_INPUT:
	{
		UINT dwSize = 0;
		GetRawInputData((HRAWINPUT)lParam, RID_INPUT, 0, &dwSize, sizeof(RAWINPUTHEADER));
		auto lpb = std::make_shared<BYTE[]>(dwSize);
		if (!lpb || (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb.get(), &dwSize, sizeof(RAWINPUTHEADER)) != dwSize))
		{
			break;
		}
		
		if (ol::KeyboardSender::SendToClient)
		{
			const auto raw = (PRAWINPUT)lpb.get();
			// No need to check if raw->header.dwType == RIM_TYPEKEYBOARD
			// As we have registered the raw input device as a keyboard only.

            ol::Input input;
			input.type = ol::eInputType::Keyboard;
			input.keyboard.key = raw->data.keyboard.VKey;
			input.keyboard.state = raw->data.keyboard.Flags;

			PostThreadMessage(GetCurrentThreadId(), static_cast<UINT>(ol::eThreadMessages::Keyboard), 0, reinterpret_cast<LPARAM>(&input));
		}

		return dwSize;
	}
	case WM_CLOSE:
	{
		std::cout << "Raw input WM_CLOSE received.\n";
		std::cin.get();
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

// Complete this.
ol::Input ol::KeyboardSender::m_GetRawInputData()
{

	MSG msg;
	while (GetMessage(&msg, 0, static_cast<UINT>(ol::eThreadMessages::Keyboard), static_cast<UINT>(ol::eThreadMessages::Keyboard)) > 0)
	{
		switch (msg.message)
		{
		case static_cast<UINT>(ol::eThreadMessages::Keyboard):
		{
			if ((ol::eThreadMessages)msg.lParam == ol::eThreadMessages::KeepAlive)
			{
				auto input = ol::Input();
				input.type = ol::eInputType::KeepAlive;
				return input;
			}

			const auto input = reinterpret_cast<ol::Input*>(msg.lParam);

			// Let's abort when something went wrong.
			if (input == nullptr)
			{
				auto failed = ol::Input();
				failed.type = ol::eInputType::Failed;
				return failed;
			}

			return *input;
		}

		case WM_QUIT:
		{
			auto input = ol::Input();
			input.type = ol::eInputType::HookStopped;
			return input;
		}
		default:
		{
			break;
		}
		}
	}

	return ol::Input();
}

// Let's maybe rename this to GetInputData() ?
ol::Input ol::KeyboardSender::GetHookData()
{
	if (this->m_bAllowConsuming)
	{
		return m_GetHookData();
	}
	else
	{
		return m_GetRawInputData();
	}
}

#endif
