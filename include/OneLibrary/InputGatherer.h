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

#include <memory>
#include <iostream>
#include <utility>
#include <cstdint>
#include <thread>

#include <OneLibrary/Constants.h>
#include <OneLibrary/Enums.h>
#include <OneLibrary/Input.h>
#include <OneLibrary/ThreadsafeQueue.h>

namespace ol
{
    // This is not thread-safe.
    // In fact, you should only ever create one Mouse/Keyboard Sender at any time.
    // TODO: Implement this as a singleton potentially, or some other approach to prevent multiple instances of this.
    class InputGatherer
    {
    protected:
        // TODO: Look into this design/architecture a bit more.
        // It works currently, but I'm not 100% sure about the intricacies of it.
        InputGatherer(const bool kAllowConsuming = true){};
        ~InputGatherer() = default;

        // All input gatherers will have:
        // - An option to either consume or copy input
        // - A buffer that stores input events

        bool m_bAllowConsuming = true;
        bool m_bConsuming = true; // If true, the input is consumed and the message is not passed forward.
        ol::ThreadsafeQueue<ol::Input> m_bufInputs{};

#ifdef OS_WINDOWS
        // This can get pretty janky, pretty quickly.
        // On Windows, all gatherers will have:
        // - A thread that gathers the input events/messages (I am unsure if that will be the case for other platforms as well)
        // - A HHOOK that will store the hook when using Low Level Hooks.
        // - Raw Input variables used when Raw Input is used (copying, not consuming input events)

        std::thread m_thInputGatherThread;
        HHOOK m_pHook = nullptr;
        WNDCLASS m_wRawInputWindowClass{};
        HWND m_hRawInputMessageWindow{};

        // Every input gatherer on Windows will need to provide their own implementation of these
        // TODO: Do these need to be virtual?

        // Low Level Hooks

        virtual void m_StartHook() = 0;
        virtual void m_WaitForLowLevelHook() = 0;
        virtual void m_EndHook() = 0;

        // Raw Input

        virtual void m_StartRawInput() = 0;
        virtual void m_WaitForRawInput() = 0;
        virtual void m_EndRawInput() = 0;
#endif

    public:
        virtual ol::Input GatherInput() = 0;
    };
}