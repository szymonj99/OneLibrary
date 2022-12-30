#ifdef OS_WINDOWS

#include <OneLibrary/MouseSender.h>

ol::MouseSender::MouseSender()
{
	m_StartHook();
}

void ol::MouseSender::m_StartHook()
{
	MSG msg;
	PeekMessage(&msg, 0, WM_USER, WM_USER, PM_NOREMOVE); // Force the system to create a message queue.
	m_pHook = SetWindowsHookEx(WH_MOUSE_LL, &ol::MouseSender::HookProc, 0, 0);
}

ol::MouseSender::~MouseSender()
{
	m_EndHook();
}

void ol::MouseSender::m_EndHook()
{
	if (m_pHook)
	{
		UnhookWindowsHookEx(m_pHook);
	}
}

// Note to self:
// This can only be called by Windows when we are in a GetMessage loop.
// Makes sense. Otherwise Windows doesn't have control of our program according to someone that can explain this better than I can, on StackOverflow.
LRESULT CALLBACK ol::MouseSender::HookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode < 0)
	{
		return CallNextHookEx(0, nCode, wParam, lParam);
	}

	// if return 1, the mouse movement will be captured and not passed to further windows.
	// if return CallNextHookEx(0, nCode, wParam, lParam), the movement will be passed further to other windows.
	if (ol::MouseSender::SendToClient)
	{
		POINT point;
		GetCursorPos(&point);

		const auto hookStruct = *(MSLLHOOKSTRUCT*)lParam;

        ol::Input input;
		input.type = ol::eInputType::Mouse;
		input.mouse.x = hookStruct.pt.x - point.x;
		input.mouse.y = hookStruct.pt.y - point.y;

		PostThreadMessage(GetCurrentThreadId(), static_cast<UINT>(ol::eThreadMessages::Mouse), 0, reinterpret_cast<LPARAM>(&input));

		return 1;
	}
	return CallNextHookEx(0, nCode, wParam, lParam);
}

ol::Input ol::MouseSender::GetHookData()
{
	MSG msg;
	while (GetMessage(&msg, 0, static_cast<UINT>(ol::eThreadMessages::Mouse), static_cast<UINT>(ol::eThreadMessages::Mouse)) > 0)
	{
		switch (msg.message)
		{
		case static_cast<UINT>(ol::eThreadMessages::Mouse):
		{
            // `continue` here?
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

#endif
