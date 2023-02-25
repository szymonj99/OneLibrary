#ifdef OS_LINUX

#include <OneLibrary/InputGathererMouse.h>

ol::InputGathererMouse::InputGathererMouse(const bool kAllowConsuming)
{
	// Not Implemented Yet
}

ol::InputGathererMouse::~InputGathererMouse()
{
	// Not Implemented Yet
}

ol::Input ol::InputGathererMouse::GatherInput()
{
	return this->m_bufInputs.Get();
}

#endif
