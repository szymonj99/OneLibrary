#ifdef OS_LINUX

#include <OneControl/MouseReceiver.h>

oc::MouseReceiver::MouseReceiver(){}

oc::MouseReceiver::~MouseReceiver(){}

void oc::MouseReceiver::MoveMouseTo(const oc::MouseInt x, const oc::MouseInt y)
{
#ifdef DEBUG_MOUSE_MOVE
#endif
}

void oc::MouseReceiver::MoveMouseRelative(const oc::MouseInt x, const oc::MouseInt y)
{
#ifdef DEBUG_MOUSE_MOVE
#endif
}

#endif
