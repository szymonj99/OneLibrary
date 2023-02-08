#pragma once

#include <OneLibrary/InputSimulator.h>

namespace ol
{
class InputSimulatorKeyboard : public ol::InputSimulator
{
public:
    InputSimulatorKeyboard() = default;
    ~InputSimulatorKeyboard() = default;
    void PerformInput(const ol::Input& kInput) override;
};
}