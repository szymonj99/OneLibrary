#ifdef OS_WINDOWS

#include <OneLibrary/InputGathererClipboard.h>

namespace
{
    ol::InputGathererClipboard* Instance = nullptr;
}

ol::InputGathererClipboard::InputGathererClipboard(const bool kAllowConsuming)
{
    assert(!Instance);
    Instance = this;
    this->m_bAllowConsuming = kAllowConsuming;

    std::binary_semaphore threadInitialised{0};
    this->m_thInputGatherThread = std::jthread([&]
    {
        // Create the window here and register it to receive clipboard messages
        this->m_wClipboardWindowClass.lpfnWndProc = ol::InputGathererClipboard::WndProc;
        this->m_wClipboardWindowClass.hInstance = nullptr;
        this->m_wClipboardWindowClass.lpszClassName = L"OneLibrary - Clipboard Procedure";
        ::RegisterClassW(&this->m_wClipboardWindowClass);

        this->m_hClipboardMessageWindow = ::CreateWindowExW(WS_EX_TOOLWINDOW, this->m_wClipboardWindowClass.lpszClassName, nullptr, 0, 0, 0, 0, 0, nullptr, nullptr, nullptr, nullptr);

        ::AddClipboardFormatListener(this->m_hClipboardMessageWindow);

        ::MSG msg{};
        ::PeekMessageW(&msg, this->m_hClipboardMessageWindow, WM_USER, WM_USER, PM_NOREMOVE);
        threadInitialised.release();
        while (this->m_bRunning) { this->m_fWaitForClipboard(); }
    });

    threadInitialised.acquire();
}

void ol::InputGathererClipboard::Shutdown() {};

void ol::InputGathererClipboard::m_fWaitForClipboard()
{
    ::MSG msg{};
    // Using the raw input message window here is okay as we have registered it as a RIDEV_INPUTSINK
    // Unlike Low Level hooks, where the hWnd has to be nullptr.
    const auto kResult = ::GetMessageW(&msg, this->m_hClipboardMessageWindow, WM_QUIT, WM_CLIPBOARDUPDATE);
    if (kResult > 0)
    {
        // Note: Calling TranslateMessage(&msg); is only necessary for keyboard input.
        // https://learn.microsoft.com/en-us/windows/win32/learnwin32/window-messages
        // However, let's try not using it for the time being.
        // Hand off every message to our Raw Input Procedure
        ::DispatchMessageW(&msg);
    }
    else if (kResult == 0) // WM_QUIT message
    {
        this->m_bRunning = false;
        std::cout << "Got result 0" << std::endl;
        return;
    }
    else
    {
        // TODO: Add error handling and change these to correct return codes.
        std::cerr << "Error occurred when getting Clipboard message" << std::endl;
        ::PostQuitMessage(0);
        std::exit(0);
    }
}

ol::InputGathererClipboard::~InputGathererClipboard()
{
    ::PostMessageW(this->m_hClipboardMessageWindow, WM_QUIT, reinterpret_cast<WPARAM>(nullptr), reinterpret_cast<LPARAM>(nullptr));
    ::RemoveClipboardFormatListener(this->m_hClipboardMessageWindow);
    ::DestroyWindow(this->m_hClipboardMessageWindow);
    if (this->m_thInputGatherThread.joinable()) { this->m_thInputGatherThread.join(); }
    Instance = nullptr;
}

LRESULT CALLBACK ol::InputGathererClipboard::WndProc(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam)
{
    ol::Input input{};
    input.eventType = ol::eEventType::ClipboardChange;
    input.inputType = ol::eInputType::Clipboard;

    switch (message)
    {
        case WM_CLIPBOARDUPDATE:
        {
            std::cout << "Got a change in clipboard" << std::endl;
            // Handle clipboard update message here

            if (!OpenClipboard(nullptr))
            {
                std::cerr << "Failed to open Clipboard" << std::endl;
                break;
            }

            const auto hData = GetClipboardData(CF_UNICODETEXT);
            if (hData == nullptr)
            {
                // TODO: This fails when trying to copy files. In the future, fix it.
                std::cerr << "Failed to get Clipboard Data. Potentially because of trying to copy a file, when only text is supported" << std::endl;
                CloseClipboard();
                break;
            }

            const auto pData = GlobalLock(hData);

            if (pData == nullptr)
            {
                std::cerr << "Failed to lock Clipboard Data" << std::endl;
                CloseClipboard();
                break;
            }

            const auto kData = std::string(static_cast<char*>(pData), (GlobalSize(hData) / sizeof(char)));

            input.clipboard.SetContents(kData);
            std::cout << "Clipboard Data: " << kData << std::endl;

            GlobalUnlock(hData);
            CloseClipboard();

            break;
        }
        case WM_QUIT:
        {
            std::cout << "Got quit message" << std::endl;
            Instance->m_bRunning = false;
            return 0;
        }
        default:
        {
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }

    if (Instance->m_bGathering)
    {
        Instance->m_bufInputs.Add(input);
    }

    return 0;
}

void ol::InputGathererClipboard::Toggle()
{
    this->m_bGathering = !this->m_bGathering;
    this->m_bConsuming = this->m_bGathering.operator bool();
}

ol::Input ol::InputGathererClipboard::GatherInput()
{
    return this->m_bufInputs.Get();
}

uint64_t ol::InputGathererClipboard::AvailableInputs()
{
    return this->m_bufInputs.Length();
}

#endif
