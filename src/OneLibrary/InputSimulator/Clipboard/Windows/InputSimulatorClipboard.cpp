#ifdef OS_WINDOWS

#include <OneLibrary/InputSimulatorClipboard.h>

ol::InputSimulatorClipboard::InputSimulatorClipboard()
{
    // Do we need to do any setup here?
}

ol::InputSimulatorClipboard::~InputSimulatorClipboard() {}

void ol::InputSimulatorClipboard::PerformInput(const ol::Input &kInput)
{
    std::cout << "Setting clipboard data" << std::endl;

    // Open the clipboard
    if (!OpenClipboard(nullptr)) {
        // Failed to open the clipboard
        std::cerr << "Failed to open clipboard" << std::endl;
        return;
    }

    if (!EmptyClipboard())
    {
        CloseClipboard();
        std::cerr << "Could not clear the clipboard" << std::endl;
        return;
    }

    // Allocate global memory for the data
    //size_t size = (kInput.clipboard.GetContents().size() + 1) * sizeof(char);
    //auto hMemory = GlobalAlloc(GMEM_MOVEABLE, size);
    auto hMemory = GlobalAlloc(GMEM_MOVEABLE, kInput.clipboard.GetContents().size() + 1);
    if (!hMemory)
    {
        // handle error
        CloseClipboard();
        std::cerr << "Could not allocate memory for the clipboard data" << std::endl;
        return;
    }

    // Lock the memory and get a pointer to it
    auto pMemory = static_cast<char*>(GlobalLock(hMemory));
    if (!pMemory) {
        // handle error
        GlobalFree(hMemory);
        CloseClipboard();
        std::cout << "Could not lock memory that will store clipboard data" << std::endl;
        return;
    }

    // Copy the data to the memory
    // TODO: This currently only copies the first character, not the whole string.
    std::cout << "Copying the data to clipboard" << std::endl;
    std::cout << "Size of data (bytes): " << kInput.clipboard.GetContents().size() << std::endl;
    std::cout << "Length of data: " << kInput.clipboard.GetContents().length() << std::endl;
    std::cout << "Clipboard data to set: " << kInput.clipboard.GetContents() << std::endl;
    //strcpy_s(pMemory, (kInput.clipboard.GetContents().size() + 1) * sizeof(char), kInput.clipboard.GetContents().c_str());
    //memcpy(pMemory, kInput.clipboard.GetContents().c_str(), kInput.clipboard.GetContents().size() + 1);
    //std::copy(pMemory, kInput.clipboard.GetContents().c_str(), kInput.clipboard.GetContents().size() + 1);
    //std::copy(kInput.clipboard.GetContents().c_str(), kInput.clipboard.GetContents().c_str() + ((kInput.clipboard.GetContents().size() + 1) * sizeof(char)), pMemory);

    memcpy_s(pMemory, kInput.clipboard.GetContents().size() + 1, kInput.clipboard.GetContents().c_str(), kInput.clipboard.GetContents().size());

   // std::copy(kInput.clipboard.GetContents(), kInput.clipboard.GetContents().size(), pMemory);

    //std::copy(kInput.clipboard.GetContents().begin(), kInput.clipboard.GetContents().end(), pMemory);
    //std::copy(kInput.clipboard.GetContents().c_str(), kInput.clipboard.GetContents().c_str() + (strlen(kInput.clipboard.GetContents().c_str()) + 1) * sizeof(char), pMemory);

    GlobalUnlock(hMemory);

    // Set the data on the clipboard
    if (!SetClipboardData(CF_TEXT, hMemory)) {
        // handle error
        std::cerr << "Could not set clipboard data" << std::endl;
        GlobalFree(hMemory);
        CloseClipboard();
        return;
    }

    // Close the clipboard
    CloseClipboard();
    std::cout << "Clipboard data set successfully" << std::endl;
}

#endif
