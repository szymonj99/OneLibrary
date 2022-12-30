#ifdef OS_WINDOWS

#pragma once

#include <Windows.h>

#include <cstdint>
#include <chrono>
#include <thread>
#include <memory>
#include <iostream>

#include <OneLibrary/Enums.h>
#include <OneLibrary/Constants.h>

namespace ol
{
	class MessageTimer
	{
	private:
		std::chrono::milliseconds m_Duration = std::chrono::milliseconds{0};
		std::unique_ptr<std::thread> m_pThread = nullptr;
        ol::eThreadMessages m_Message = ol::eThreadMessages::Empty;
		DWORD m_ThreadID = 0;
		void Function();
		bool m_StopRequested = false;
		
	public:
		MessageTimer(const uint32_t milliseconds, const ol::eThreadMessages messageToSend, const DWORD threadID);
		~MessageTimer();
	};
}

#endif
