#pragma once

#include <OneLibrary/InputGatherer.h>

namespace ol
{
    class InputGathererClipboard : public ol::InputGatherer
    {
    private:
#ifdef OS_WINDOWS
        std::jthread m_thInputGatherThread;
        WNDCLASS m_wClipboardWindowClass{};
        HWND m_hClipboardMessageWindow{};
        static LRESULT CALLBACK WndProc(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam);
        void m_fWaitForClipboard();
#endif

    public:
        // TODO: Should kAllowConsuming do anything for clipboard events?
        explicit InputGathererClipboard(const bool kAllowConsuming = true);
        ~InputGathererClipboard();
        [[nodiscard]] ol::Input GatherInput() override;
        void Toggle() override;
        void Shutdown() override;
        [[nodiscard]] uint64_t AvailableInputs() override;
    };
}
