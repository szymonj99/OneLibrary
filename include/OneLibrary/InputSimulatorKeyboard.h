#pragma once

#include <OneLibrary/InputSimulator.h>

namespace ol
{
    /**
     * This allows for injecting of keyboard events.
     */
    class InputSimulatorKeyboard : public ol::InputSimulator
    {
    public:
        InputSimulatorKeyboard();
        ~InputSimulatorKeyboard();
        void PerformInput(const ol::Input& kInput) override;
    };
}
