#ifdef OS_LINUX

#include <OneLibrary/InputSimulatorMouse.h>

ol::InputSimulatorMouse::InputSimulatorMouse()
{
	libevdev_set_name(this->m_pVirtualDevice, "OneLibrary-Input-Mouse");
	const auto kVendor = 0x4F4C; // "OL" -> Ascii to Hex, short for OneLibrary
	libevdev_set_id_vendor(this->m_pVirtualDevice, kVendor);
	const auto kProduct = 0x4F4C;
	libevdev_set_id_product(this->m_pVirtualDevice, kProduct);

	libevdev_enable_event_type(this->m_pVirtualDevice, EV_KEY);
	libevdev_enable_event_type(this->m_pVirtualDevice, EV_REL);

	// Relative X and Y coords
	libevdev_enable_event_code(this->m_pVirtualDevice, EV_REL, REL_X, nullptr);
	libevdev_enable_event_code(this->m_pVirtualDevice, EV_REL, REL_Y, nullptr);


	libevdev_enable_event_code(this->m_pVirtualDevice, EV_KEY, BTN_LEFT, nullptr);
	libevdev_enable_event_code(this->m_pVirtualDevice, EV_KEY, BTN_RIGHT, nullptr);
	libevdev_enable_event_code(this->m_pVirtualDevice, EV_KEY, BTN_MIDDLE, nullptr);
	libevdev_enable_event_code(this->m_pVirtualDevice, EV_KEY, BTN_WHEEL, nullptr);

	libevdev_uinput* dev{}; // Store the output of we get from creating the device
	libevdev_uinput_create_from_device(this->m_pVirtualDevice, LIBEVDEV_UINPUT_OPEN_MANAGED, &dev);
	this->m_pVirtualInput = dev;
}

ol::InputSimulatorMouse::~InputSimulatorMouse()
{
    libevdev_free(this->m_pVirtualDevice);
    libevdev_uinput_destroy(this->m_pVirtualInput);
}

void ol::InputSimulatorMouse::PerformInput(const ol::Input& kInput)
{
	constexpr uint16_t BUTTON_DOWN = 1;
	constexpr uint16_t BUTTON_UP = 0;

	switch (kInput.eventType)
	{
	case ol::eEventType::MMove:
		libevdev_uinput_write_event(this->m_pVirtualInput, EV_REL, REL_X, kInput.mouse.x);
		libevdev_uinput_write_event(this->m_pVirtualInput, EV_REL, REL_Y, kInput.mouse.y);
        break;
	case ol::eEventType::MScroll:
		libevdev_uinput_write_event(this->m_pVirtualInput, EV_REL, REL_WHEEL, kInput.mouse.scroll);
        break;
	case ol::eEventType::MButtonLeftDown:
		libevdev_uinput_write_event(this->m_pVirtualInput, EV_KEY, BTN_LEFT, BUTTON_DOWN);
        break;
	case ol::eEventType::MButtonLeftUp:
		libevdev_uinput_write_event(this->m_pVirtualInput, EV_KEY, BTN_LEFT, BUTTON_UP);
        break;
	case ol::eEventType::MButtonRightDown:
		libevdev_uinput_write_event(this->m_pVirtualInput, EV_KEY, BTN_RIGHT, BUTTON_DOWN);
        break;
	case ol::eEventType::MButtonRightUp:
		libevdev_uinput_write_event(this->m_pVirtualInput, EV_KEY, BTN_RIGHT, BUTTON_UP);
        break;
	case ol::eEventType::MButtonMiddleDown:
		libevdev_uinput_write_event(this->m_pVirtualInput, EV_KEY, BTN_MIDDLE, BUTTON_DOWN);
        break;
	case ol::eEventType::MButtonMiddleUp:
		libevdev_uinput_write_event(this->m_pVirtualInput, EV_KEY, BTN_MIDDLE, BUTTON_UP);
        break;
	default:
        std::cout << "Event type: " << kInput.eventType << std::endl;
		std::cerr << "Got unhandled input in mouse simulator." << std::endl;
		return;
	}

	libevdev_uinput_write_event(this->m_pVirtualInput, EV_SYN, SYN_REPORT, 0);
}

#endif
