#pragma once

#ifdef OS_WINDOWS
#include <Windows.h>
#endif

#include <OneLibrary/Constants.h>

namespace ol
{
	class MouseReceiver
	{
	public:
		MouseReceiver();
		~MouseReceiver();
		void MoveMouseTo(const ol::MouseInt x, const ol::MouseInt y);
		void MoveMouseRelative(const ol::MouseInt x, const ol::MouseInt y);
	};
}
