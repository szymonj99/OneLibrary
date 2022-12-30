#pragma once

#include <cstdint>

namespace ol
{
	enum class eMachineState : uint32_t
	{
		Uninitialised = 0,
		Server,
		Client
	};

	enum class eInputType : uint32_t
	{
		Uninitialised = 0,
		KeepAlive,
		Mouse,
		Keyboard,
		Failed,
		HookStopped
	};

#ifdef OS_WINDOWS
    #define WM_USER 0x0400
    #define WM_APP  0x8000
	enum class eThreadMessages : uint32_t
	{
	    Empty = WM_APP + 1,
	    Mouse,
	    Keyboard,
	    KeepAlive
	};
#endif
}
