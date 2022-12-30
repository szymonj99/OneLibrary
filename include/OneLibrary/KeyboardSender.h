#pragma once

#ifdef OS_WINDOWS
    #include <Windows.h>
    #include <hidusage.h>
#elif OS_LINUX
    #include <linux/uinput.h>
    #include <linux/input.h>
    #include <linux/ioctl.h>
#elif OS_APPLE
#endif

#include <memory>
#include <iostream>
#include <utility>
#include <cstdint>
#include <thread>

#include <OneLibrary/Constants.h>
#include <OneLibrary/Enums.h>
#include <OneLibrary/Input.h>

namespace ol
{
	class KeyboardSender
	{
	private:
#ifdef OS_WINDOWS
		bool m_bAllowConsuming = false;

		// These hook functions are used when we are using low level hooks.
		HHOOK m_pHook = nullptr;
		void m_StartHook();
        ol::Input m_GetHookData();
		void m_EndHook();

		// These functions are used when we are using raw input.
		WNDCLASS m_wRawInputWindowClass = WNDCLASS();
		HWND m_hRawInputMessageWindow = HWND();
		void m_StartRawInput();
        ol::Input m_GetRawInputData();
		void m_EndRawInput();
#elif OS_LINUX
#elif OS_APPLE
#endif

	public:
		KeyboardSender(bool allowConsuming);
		~KeyboardSender();

#ifdef OS_WINDOWS
        ol::Input GetHookData();

		// Static variables!
		static inline bool SendToClient = true;
		// This is what's called every time a low level hook event happens.
		static LRESULT CALLBACK HookProc(int nCode, WPARAM wParam, LPARAM lParam);
		// This is what's called every time a raw input event happens.
		static LRESULT CALLBACK RawInputKeyboardProc(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam);
#elif OS_LINUX
#elif OS_APPLE
#endif
	};
}