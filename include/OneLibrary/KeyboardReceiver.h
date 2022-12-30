#pragma once

#ifdef OS_WINDOWS
    #include <Windows.h>
#endif

#include <OneLibrary/Constants.h>

namespace ol
{
	class KeyboardReceiver
	{
	public:
		KeyboardReceiver();
		~KeyboardReceiver();
		void KeyPress(const ol::KeyboardInt, const ol::KeyboardInt);
	};
}
