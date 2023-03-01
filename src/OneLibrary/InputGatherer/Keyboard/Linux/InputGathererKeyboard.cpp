#ifdef OS_LINUX

#include <OneLibrary/InputGathererKeyboard.h>

namespace fs = std::filesystem;

ol::InputGathererKeyboard::InputGathererKeyboard(const bool kAllowConsuming)
{
    this->m_bAllowConsuming = kAllowConsuming;

	fs::path devicePath("/dev/input/by-path");

	for (const auto& entry : fs::directory_iterator(devicePath))
	{
		// device files are symlinks
		if (entry.is_symlink() && entry.path().filename().string().ends_with("-event-kbd"))
		{
			// TODO: Add error handling.

			const auto kPath = fs::canonical(entry.path()).string();

            this->m_vDeviceFiles.emplace_back(::open(kPath.c_str(), O_RDONLY));
			libevdev* input{};
            libevdev_new_from_fd(this->m_vDeviceFiles.back(), &input);

			// When we grab the input, the events are not passed to any other window.
            if (this->m_bAllowConsuming)
            {
                libevdev_grab(input, LIBEVDEV_GRAB);
            }

            this->m_vVirtualDevices.push_back(input);

            // We do not call `libevdev_set_fd_nonblock` as the reading of these input events will be done on a separate thread.
			// Therefore, we do actually want to block until an input is available.
			// Doing so will allow us to not have to be busy-waiting.

			this->m_vDeviceHandlers.emplace_back([&](libevdev* device){ this->m_fDeviceHandler(device); }, this->m_vVirtualDevices.back());
		}
	}
}

void ol::InputGathererKeyboard::m_fDeviceHandler(libevdev* device)
{
	// TODO: Add exit condition here.
    // TODO: Add killswitch.
    // TODO: Add support for hotplugging devices.

	while (true)
	{
		input_event ev{};
		// Block until an input event is available.
		// Doing this is okay because this function will be executed in a separate thread.
		// TODO: Add error handling.
		const auto returnCode = libevdev_next_event(device, LIBEVDEV_READ_FLAG_BLOCKING, &ev);

		ol::Input input{};
		input.inputType = ol::eInputType::Keyboard;

        // TODO: Add hot-plugging support.

		switch (ev.type)
		{
			case EV_KEY:
            {
                // 'Some hardware send events when a key is repeated. These events have a value of 2.'
                // I think ideally, repeat keystrokes don't need to be sent to the client.
                // This is because simulating a keydown press, would in theory keep it pressed down until keyup event is sent.
                // However, in practice I found it necessary to send these repeat keystrokes across.
                if (ev.value > 0)
                {
                    input.eventType = ol::eEventType::KBKeyDown;
                }
                else
                {
                    input.eventType = ol::eEventType::KBKeyUp;
                }

                input.keyboard.key = ol::LinuxToKeyCode.at(ev.code);
                break;
            }
            case EV_MSC:
            {
                // This can tell us which key has been pressed.
                // EV_KEY provides us with this, so let's use that instead.
                continue;
            }
            case EV_SYN:
            {
                // We don't have a reason AFAIK to interact with this event type.
                continue;
            }
			default:
            {
                std::cout << "Got unhandled event type " << ev.type << std::endl;
                continue;
            }
		}

		this->m_bufInputs.Add(input);
	}
}

ol::InputGathererKeyboard::~InputGathererKeyboard()
{
    for (auto& virtualDevice : this->m_vVirtualDevices)
    {
        libevdev_free(virtualDevice);
    }

    for (auto& fileDescriptor : this->m_vDeviceFiles)
    {
        ::close(fileDescriptor);
    }
}

ol::Input ol::InputGathererKeyboard::GatherInput()
{
	return this->m_bufInputs.Get();
}

#endif
