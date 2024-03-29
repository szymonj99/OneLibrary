#pragma once

#ifdef OS_WINDOWS
    #include <Windows.h>
    #include <hidusage.h>
#elif OS_LINUX
    #include <memory>
    #include <libevdev/libevdev.h>
    #include <libevdev/libevdev-uinput.h>
#elif OS_APPLE
#endif

#include <OneLibrary/Input.h>
#include <OneLibrary/InputTable.h>

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

#ifdef OS_LINUX
        libevdev* m_pVirtualDevice = libevdev_new();
        libevdev_uinput* m_pVirtualInput{};
#endif

    public:
        /**
         * Each class that inherits from this base class needs to implement their own way of handling an input that has been received.
         */
        virtual void PerformInput(const ol::Input& kInput) = 0;
    };
}
