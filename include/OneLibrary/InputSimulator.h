#pragma once

#ifdef OS_WINDOWS
    #include <Windows.h>
    #include <hidusage.h>
#elif OS_LINUX
    #include <linux/uinput.h>
    #include <linux/input.h>
    #include <linux/ioctl.h>
    #include <iostream>
    #include <fstream>
    #include <cstdio>
    #include <fcntl.h>
    #include <linux/input.h>
    #include <linux/uinput.h>
    #include <unistd.h>
#elif OS_APPLE
#endif

#include <OneLibrary/Input.h>

namespace ol
{
class InputSimulator
{
protected:
    InputSimulator() = default;
    ~InputSimulator() = default;

public:
    virtual void PerformInput(const ol::Input& kInput) = 0;
};
}