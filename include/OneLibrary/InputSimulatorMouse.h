#pragma once

#include <OneLibrary/InputSimulator.h>

namespace ol
{
class InputSimulatorMouse : public ol::InputSimulator
{
public:
    InputSimulatorMouse() = default;
    ~InputSimulatorMouse() = default;
    void PerformInput(const ol::Input& kInput) override;
};
}