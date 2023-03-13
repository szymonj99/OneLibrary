#pragma once

#include <OneLibrary/InputGatherer.h>

namespace ol
{
    /**
     * This gathers keyboard input events.
     */
    class InputGathererKeyboard : public ol::InputGatherer
    {
    private:
#ifdef OS_WINDOWS
        std::thread m_thInputGatherThread;
        HHOOK m_pHook = nullptr;

        WNDCLASS m_wRawInputWindowClass{};
        HWND m_hRawInputMessageWindow{};

        void m_fStartHook();
        void m_fWaitForLowLevelHook();
        void m_fEndHook();

        void m_fStartRawInput();
        void m_fWaitForRawInput();
        void m_fEndRawInput();

        // This is what's called every time a low-level mouse event happens.
        static LRESULT CALLBACK LowLevelHookProcedure(const int nCode, const WPARAM wParam, const LPARAM lParam);
        // This is what's called every time a raw input event happens.
        static LRESULT CALLBACK RawInputProcedure(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam);
#elif OS_LINUX
        void m_fDeviceHandler(libevdev* device) override;
        static void m_fSignalHandler(const int32_t kSignal);
#endif
        // To avoid potential issues with calling virtual functions in the destructor, e can instead
        // call these functions which will only be defined in the current derived class and not in the base class.
        // These are currently only called on Windows.
        // Do _NOT_ call these by yourself.
        void m_fInit();
        void m_fTerminate();

    public:
        /**
         * Create an input gatherer responsible for gathering keyboard input events.
         * @param kAllowConsuming If true, this gatherer is allowed to consume input events.
         */
        explicit InputGathererKeyboard(const bool kAllowConsuming = true);
        ~InputGathererKeyboard();
        /**
         * Blocks until a keyboard event happens.
         * @return An input object representative of the event.
         */
        ol::Input GatherInput() override;
        void Toggle() override;
        void Shutdown() override;
    };
}
