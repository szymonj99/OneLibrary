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
        std::jthread m_thInputGatherThread;
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
        void m_fInit();
        void m_fTerminate();

    public:
        /**
         * Create a new input gatherer for mouse events.
         * @param kAllowConsuming If true, the gatherer can be set to consume the events, meaning the event will not be passed to other windows or hooks.
         */
        explicit InputGathererMouse(const bool kAllowConsuming = true);
        ~InputGathererMouse();
        void Toggle() override;
        void Shutdown() override;
    };
}
