#pragma once

#include <OneLibrary/InputSimulator.h>

namespace ol
{
    /**
     * This allows for injecting of mouse events.
     */
    class InputSimulatorMouse : public ol::InputSimulator
    {
    public:
        InputSimulatorMouse();
        ~InputSimulatorMouse();
        void PerformInput(const ol::Input& kInput) override;
    };
}
