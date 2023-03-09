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
        // To avoid potential issues with calling virtual functions in the destructor, e can instead
        // call these functions which will only be defined in the current derived class and not in the base class.
        // These are currently only called on Windows.
        // Do _NOT_ call these by yourself.
        void m_fInit();
        void m_fTerminate();

        void m_fStartHook() override;
        void m_fWaitForLowLevelHook() override;
        void m_fEndHook() override;

        void m_fStartRawInput() override;
        void m_fWaitForRawInput() override;
        void m_fEndRawInput() override;

        // This is what's called every time a low-level mouse event happens.
        static LRESULT CALLBACK LowLevelHookProcedure(const int nCode, const WPARAM wParam, const LPARAM lParam);
        // This is what's called every time a raw input event happens.
        static LRESULT CALLBACK RawInputProcedure(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam);
#elif OS_LINUX
        void m_fDeviceHandler(libevdev* device) override;
        // TODO: Figure out if we want to include this in the base class :thinking:
        static void m_fSignalHandler(const int32_t signal);
#endif

    public:
        /**
         * Create a new input gatherer for mouse events.
         * @param kAllowConsuming If true, the gatherer can be set to consume the events, meaning the event will not be passed to other windows or hooks.
         */
        explicit InputGathererMouse(const bool kAllowConsuming = true);
        ~InputGathererMouse();
        ol::Input GatherInput() override;
        void Toggle() override;
    };
}
