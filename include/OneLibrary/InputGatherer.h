#pragma once

#ifdef OS_WINDOWS
    #include <Windows.h>
    #include <hidusage.h>
#elif OS_LINUX
    #include <filesystem>
    #include <fstream>
    #include <vector>
    #include <linux/input.h>
    #include <libevdev/libevdev.h>
    #include <libevdev/libevdev-uinput.h>
    #include <fcntl.h>
    #include <csignal>
    #include <cerrno>
    #include <cstring>
    #include <regex>
#elif OS_APPLE
#endif

#include <memory>
#include <iostream>
#include <utility>
#include <cstdint>
#include <thread>
#include <cassert>

#include <OneLibrary/Constants.h>
#include <OneLibrary/Enums.h>
#include <OneLibrary/Input.h>
#include <OneLibrary/ThreadsafeQueue.h>
#include <OneLibrary/InputTable.h>

namespace ol
{
    /**
     * The base class that other gatherers inherit from. How this works differs per platform, but the end result is consistent between platforms.
     */
    class InputGatherer
    {
    protected:
        // We don't want people to be able to instantiate this abstract base class.
        // We do not want someone to create an InputGatherer without specifying what it is, so let's delete the constructor for this abstract base class.
        // Or do we want to make the constructor private instead?
        InputGatherer() = default;
        ~InputGatherer() = default;

        // All input gatherers will have a buffer that stores the inputs it has gathered.
        // All input gatherers will have:
        // - An option to either consume or copy input - This will only be in effect with the likes of low level hooks on windows;
        // Raw Input on Windows does not allow for consumption of inputs.

        /**
         * If true, this object is allowed to consume the inputs it is set to gather.
         */
        std::atomic<bool> m_bAllowConsuming = true;
        /**
         * If true, this object is consuming inputs. Those inputs will not be passed to other windows or event listeners.
         * TODO: Look into this a little bit more. Raw Input should in theory be m_bConsuming = false, and m_bGathering = true or similar.
         */
        std::atomic<bool> m_bConsuming = true; // If true, the input is consumed and the message is not passed forward.
        /**
         * A buffer that stores the input events.
         */
        ol::ThreadsafeQueue<ol::Input> m_bufInputs{};

        std::atomic<bool> m_bRunning = true;
        std::atomic<bool> m_bGathering = true;

        std::atomic<bool> m_bCalledTerminate = false;

#ifdef OS_LINUX
        std::vector<libevdev*> m_vVirtualDevices{};
        std::vector<std::jthread> m_vDeviceHandlers{};
        std::vector<int32_t> m_vDeviceFiles{};

        virtual void m_fDeviceHandler(libevdev* device) = 0;
#endif

    public:
        /**
         * Block until this object gathered some input from the user.
         * @return The input representative of the event that has happened.
         */
        [[nodiscard]] virtual ol::Input GatherInput() = 0;
        virtual void Toggle() = 0;
        virtual void Shutdown() = 0;
        [[nodiscard]] virtual uint64_t AvailableInputs() = 0;
    };
}
