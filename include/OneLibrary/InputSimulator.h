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
    /**
     * A base class that allows for injection of input programmatically.
     */
    class InputSimulator
    {
    protected:
        InputSimulator() = default;
        ~InputSimulator() = default;

    public:
        /**
         * Each class that inherits from this base class needs to implement their own way of handling an input that has been received.
         */
        virtual void PerformInput(const ol::Input& kInput) = 0;
    };
}