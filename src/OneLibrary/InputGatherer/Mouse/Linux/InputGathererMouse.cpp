#ifdef OS_LINUX

#include <OneLibrary/InputGathererMouse.h>

namespace fs = std::filesystem;

ol::InputGathererMouse::InputGathererMouse(const bool kAllowConsuming)
{
	this->m_bAllowConsuming = kAllowConsuming;
	// Iterate over files in `/dev/input/by-path`.
	// It is better to interact with the event files rather than `mouseX` files as they provide more information, and are not reliant on X server.
	// Files ending in `-event-kbd` are keyboard event files.
	// Files ending in `-event-mouse` are mouse event files.

	fs::path devicePath("/dev/input/by-path");

	for (const auto& entry : fs::directory_iterator(devicePath))
	{
		// device files are symlinks
		if (entry.is_symlink() && entry.path().filename().string().ends_with("-event-mouse"))
		{
			// TODO: Add error handling.

			const auto kPath = fs::canonical(entry.path()).string();

			this->m_vDeviceFiles.emplace_back(::open(kPath.c_str(), O_RDONLY));
			libevdev* input{};
			const auto r = libevdev_new_from_fd(this->m_vDeviceFiles.back(), &input);

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

void ol::InputGathererMouse::m_fDeviceHandler(libevdev *device)
{
	// TODO: Add exit condition here.
    // TODO: Add killswitch.

    bool setNextRelativeX = true, setNextRelativeY = true;
	int16_t previousX = 0, previousY = 0, previousWheel = 0;

	while (true)
	{
		input_event ev{};
		// Block until an input event is available.
		// Doing this is okay because this function will be executed in a separate thread.
		// TODO: Add error handling.
		const auto returnCode = libevdev_next_event(device, LIBEVDEV_READ_FLAG_BLOCKING, &ev);

		ol::Input input{};
		input.inputType = ol::eInputType::Mouse;

        // TODO: I wonder if we could potentially disable some of these events.
        // Doing so would filter them out from this device only, meaning fewer case statements.
        // Let's get it working correctly first.
        // TODO: Add support for hot-plugging.

		switch (ev.type)
		{
			// AFAIK, these events will only have movement in one direction at a time. E.g., X=-5 and Y=+10 is not possible.
			// Those would be two separate inputs.
			// TODO: 'Some hardware send events when a key is repeated. These events have a value of 2.' <-- Add this to the keyboard gatherer.
            // Note to self: call `continue` on events that we don't want to pass to the client.
            // Call `break` if we want to pass the event to the client.
			case EV_KEY:
            {
                switch (ev.code)
                {
                    case BTN_LEFT:
                    {
                        if (ev.value == 1) { input.eventType = ol::eEventType::MButtonLeftDown; }
                        if (ev.value == 0) { input.eventType = ol::eEventType::MButtonLeftUp; }
                        break;
                    }
                    case BTN_RIGHT:
                    {
                        // TODO: Figure out why on my trackpad, right clicking actually is registered as a left click.
                        if (ev.value == 1) { input.eventType = ol::eEventType::MButtonRightDown; }
                        if (ev.value == 0) { input.eventType = ol::eEventType::MButtonRightUp; }
                        break;
                    }
                    case BTN_MIDDLE:
                    {
                        if (ev.value == 1) { input.eventType = ol::eEventType::MButtonMiddleDown; }
                        if (ev.value == 0) { input.eventType = ol::eEventType::MButtonMiddleUp; }
                        break;
                    }
                    case BTN_TOUCH:
                    {
                        // In my case, ev.value == 1 when the state of the trackpad goes from no fingers, to a finger touching it.
                        // Pressing the trackpad with more fingers does not result in increasing ev.value values.
                        // When the state of the trackpad goes from touched to untouched (no fingers touching it), ev.value == 0.
                        setNextRelativeX = (ev.value == 1);
                        setNextRelativeY = (ev.value == 1);
                        continue;
                    }
                    case BTN_TOOL_FINGER:
                    {
                        // This event happens every time the touch state of the trackpad changes.
                        // E.g. the number of fingers touching the trackpad changes.
                        // We can ignore this.
                        // In my case, ev.value starts out at 0.
                        // With one finger pressed, ev.value == 1.
                        // With two fingers pressed, ev.value == 0.
                        // With more fingers, this event does not occur.
                        continue;
                    }
                    case BTN_TOOL_DOUBLETAP:
                    {
                        // Not sure if we should really do anything here.
                        // Potentially, right click when the user does a two-finger press?
                        // Then, two-finger pressing and moving up or down could result in scrolling up or down.
                        continue;
                    }
                    case BTN_TOOL_TRIPLETAP:
                    {
                        // Not sure if we should really do anything here.
                        continue;
                    }
                    case BTN_TOOL_QUADTAP:
                    {
                        // Not sure if we should really do anything here.
                        continue;
                    }
                    case BTN_TOOL_QUINTTAP:
                    {
                        // Not sure if we should really do anything here.
                        continue;
                    }
                    default:
                    {
                        std::cout << "Got unhandled button event code " << ev.code << std::endl;
                        continue;
                    }
                }
                break;
            }
			case EV_REL: // TODO: My laptop trackpad doesn't use EV_REL events, therefore we need to test this in the future somehow.
            {
                switch (ev.code)
                {
                    case REL_X:
                    {
                        input.mouse.x = static_cast<int16_t>(ev.value);
                        input.eventType = ol::eEventType::MMove;
                        break;
                    }
                    case REL_Y:
                    {
                        input.mouse.y = static_cast<int16_t>(ev.value);
                        input.eventType = ol::eEventType::MMove;
                        break;
                    }
                    case REL_WHEEL:
                    {
                        input.mouse.scroll = static_cast<int16_t>(ev.value);
                        input.eventType = ol::eEventType::MScroll;
                        break;
                    }
                    default:
                    {
                        std::cout << "Got unhandled relative event code " << ev.code << std::endl;
                        continue;
                    }
                }
                break;
            }
			case EV_ABS:
            {
                switch (ev.code)
                {
                    case ABS_X:
                    {
                        if (!setNextRelativeX)
                        {
                            input.eventType = ol::eEventType::MMove;
                            input.mouse.x = static_cast<int16_t>(ev.value) - previousX;
                            previousX = static_cast<int16_t>(ev.value);
                            break;
                        }
                        previousX = static_cast<int16_t>(ev.value);
                        setNextRelativeX = false;
                        continue;
                    }
                    case ABS_Y:
                    {
                        if (!setNextRelativeY)
                        {
                            input.eventType = ol::eEventType::MMove;
                            input.mouse.y = static_cast<int16_t>(ev.value) - previousY;
                            previousY = static_cast<int16_t>(ev.value);
                            break;
                        }
                        previousY = static_cast<int16_t>(ev.value);
                        setNextRelativeY = false;
                        continue;
                    }
                    case ABS_WHEEL: // TODO: Not sure about this one.
                    {
                        input.eventType = ol::eEventType::MScroll;
                        input.mouse.scroll = static_cast<int16_t>(ev.value) - previousWheel;
                        previousWheel = static_cast<int16_t>(ev.value);
                        break;
                    }
                    case ABS_MT_TRACKING_ID:
                    {
                        // In my case, this event happens when the trackpad is either touched, or not touched anymore.
                        // Every time the trackpad gets touched, ev.value gets incremented.
                        // Every time a finger is taken off the trackpad, ev.value is -1.
                        continue;
                    }
                    case ABS_MT_POSITION_X:
                    {
                        // In my case, this and ABS_X have the same values, but are two different events.
                        // Therefore, we can ignore this.
                        continue;
                    }
                    case ABS_MT_POSITION_Y:
                    {
                        // In my case, this and ABS_Y have the same values, but are two different events.
                        // Therefore, we can ignore this.
                        continue;
                    }
                    case ABS_MT_SLOT:
                    {
                        // I don't think we will make use of this, as we aren't bothered with the input device being modified. I think.
                        continue;
                    }
                    default:
                    {
                        std::cout << "Got unhandled absolute event code " << ev.code << std::endl;
                        continue;
                    }
                }
                break;
            }
            case EV_MSC:
            {
                // In my case, ev.code is always 5 (REL_RZ), whatever that is.
                // And ev.value keeps going up for as long as the trackpad is being touched.
                // ev.value resets to 0 when the trackpad is not being touched.
                // This doesn't seem all that useful for us.
                continue;
            }
            case EV_SYN:
            {
                // We aren't bothered with this event type, as it is only used to say when different events occurred.
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

ol::InputGathererMouse::~InputGathererMouse()
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

ol::Input ol::InputGathererMouse::GatherInput()
{
	return this->m_bufInputs.Get();
}

#endif
