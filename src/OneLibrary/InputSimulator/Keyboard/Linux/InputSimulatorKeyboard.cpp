#ifdef OS_LINUX

#include <OneLibrary/InputSimulatorKeyboard.h>

ol::InputSimulatorKeyboard::InputSimulatorKeyboard()
{
    libevdev_set_name(this->m_pVirtualDevice, "OneLibrary-Input-Keyboard");
    const auto kVendor = 0x4F4C; // "OL" -> Ascii to Hex, short for OneLibrary
    libevdev_set_id_vendor(this->m_pVirtualDevice, kVendor);
    const auto kProduct = 0x4F4C;
    libevdev_set_id_product(this->m_pVirtualDevice, kProduct);

    libevdev_enable_event_type(this->m_pVirtualDevice, EV_KEY);

    for (const auto& item : ol::KeyCodeToLinux)
    {
        libevdev_enable_event_code(this->m_pVirtualDevice, EV_KEY, item.second, nullptr);
    }

    libevdev_uinput* dev{}; // Store the output of we get from creating the device
    libevdev_uinput_create_from_device(this->m_pVirtualDevice, LIBEVDEV_UINPUT_OPEN_MANAGED, &dev);
    this->m_pVirtualInput = dev;
}

ol::InputSimulatorKeyboard::~InputSimulatorKeyboard()
{
    libevdev_free(this->m_pVirtualDevice);
    libevdev_uinput_destroy(this->m_pVirtualInput);
}

void ol::InputSimulatorKeyboard::PerformInput(const ol::Input& kInput)
{
    // Check if the mouse moved etc.

    constexpr uint16_t BUTTON_DOWN = 1;
    constexpr uint16_t BUTTON_UP = 0;

    switch (kInput.eventType)
    {
        case ol::eEventType::KBKeyDown:
            libevdev_uinput_write_event(this->m_pVirtualInput, EV_KEY, ol::KeyCodeToLinux.at(kInput.keyboard.key), BUTTON_DOWN);
            break;
        case ol::eEventType::KBKeyUp:
            libevdev_uinput_write_event(this->m_pVirtualInput, EV_KEY, ol::KeyCodeToLinux.at(kInput.keyboard.key), BUTTON_UP);
            break;
        default:
            std::cout << "Event type: " << kInput.eventType << std::endl;
            std::cerr << "Got unhandled input in keyboard simulator." << std::endl;
            return;
    }

    libevdev_uinput_write_event(this->m_pVirtualInput, EV_SYN, SYN_REPORT, 0);
}

#endif
