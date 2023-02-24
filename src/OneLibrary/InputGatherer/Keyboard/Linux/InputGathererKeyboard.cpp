#ifdef OS_LINUX

#include <OneLibrary/InputGathererKeyboard.h>

ol::InputGathererKeyboard::InputGathererKeyboard(const bool kAllowConsuming)
{
    // Not Implemented yet
}

ol::InputGathererKeyboard::~InputGathererKeyboard()
{
    // Not Implemented yet
}

ol::Input ol::InputGathererKeyboard::GatherInput()
{
    return this->m_bufInputs.Get();
}

#endif
