#pragma once

#include <OneLibrary/InputGatherer.h>

namespace ol
{
    /**
     * This gathers mouse input events.
     */
    class InputGathererMouse : public ol::InputGatherer
    {
    private:
#ifdef OS_WINDOWS
        static inline ol::ThreadsafeQueue<ol::Input> m_bufInputs{};
        static inline bool m_bConsuming = true;

        // To avoid potential issues with calling virtual functions in the destructor, e can instead
        // call these functions which will only be defined in the current derived class and not in the base class.
        // These are currently only called on Windows.
        // Do _NOT_ call these by yourself.
        void _init();
        void _terminate();

        void m_StartHook() override;
        void m_WaitForLowLevelHook() override;
        void m_EndHook() override;

        void m_StartRawInput() override;
        void m_WaitForRawInput() override;
        void m_EndRawInput() override;

        // This is what's called every time a low-level mouse event happens.
        static LRESULT CALLBACK LowLevelHookProcedure(const int nCode, const WPARAM wParam, const LPARAM lParam);
        // This is what's called every time a raw input event happens.
        static LRESULT CALLBACK RawInputProcedure(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam);
#elif OS_LINUX
        void m_fDeviceHandler(libevdev* device) override;
#endif

    public:
        /**
         * Create a new input gatherer for mouse events.
         * @param kAllowConsuming If true, the gatherer can be set to consume the events, meaning the event will not be passed to other windows or hooks.
         */
        explicit InputGathererMouse(const bool kAllowConsuming = true);
        ~InputGathererMouse();
        ol::Input GatherInput() override;
    };
}
