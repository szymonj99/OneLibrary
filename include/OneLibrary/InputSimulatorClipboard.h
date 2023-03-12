#pragma once

#include <OneLibrary/InputSimulator.h>

namespace ol
{
    class InputSimulatorClipboard : public ol::InputSimulator
    {
    public:
        InputSimulatorClipboard();
        ~InputSimulatorClipboard();
        void PerformInput(const ol::Input& kInput) override;
    };
}
