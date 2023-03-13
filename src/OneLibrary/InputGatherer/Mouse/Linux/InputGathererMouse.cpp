#ifdef OS_LINUX

#include <OneLibrary/InputGathererMouse.h>

namespace fs = std::filesystem;

namespace
{
    // Not quite a singleton pattern, but almost.
    ol::InputGathererMouse* Instance = nullptr;
    // 20 is just a random value I picked that should be high enough to handle all threads shutting down.
    std::counting_semaphore<20> semShuttingDown{0};
}

void ol::InputGathererMouse::m_fInit()
{
    // We add a signal handler so that libevdev_next_event can exit early if the threads are interrupted.
    struct sigaction sa{};
    sa.sa_handler = ol::InputGathererMouse::m_fSignalHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, nullptr);

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
            const auto kReturnCode = ::libevdev_new_from_fd(this->m_vDeviceFiles.back(), &input);
            if (kReturnCode != 0) // Error
            {
                std::cerr << "Failed to create a new libevdev device: " << std::strerror(-kReturnCode) << std::endl;
                this->m_bRunning = false;
                return;
            }

            // When we grab the input, the events are not passed to any other window.
            if (this->m_bAllowConsuming)
            {
                ::libevdev_grab(input, LIBEVDEV_GRAB);
            }

            this->m_vVirtualDevices.push_back(input);

            // We do not call `libevdev_set_fd_nonblock` as the reading of these input events will be done on a separate thread.
            // Therefore, we do actually want to block until an input is available.
            // Doing so will allow us to not have to be busy-waiting.

            this->m_vDeviceHandlers.emplace_back([&](libevdev* device){ this->m_fDeviceHandler(device); }, this->m_vVirtualDevices.back());
        }
    }

#ifdef ONELIBRARY_TESTS
// Let's naively assume that this is being called right after the input simulator has been called.
    // The input simulator results in a new event file being created in /dev/input
    // This event file is created with the next sequential ID -> if event7 was present, calling off to input simulator will generate event8.
    // We can use that here.
    // However, we can also get the exact system path for the specific virtual input device that input simulator creates.
    // How do we want to handle that? :thinking:
    // Let's go wit the naive approach for now, and talk about potential improvements in the future.
    // libevdev_uinput_get_syspath(dev) can be used to get the real path of the simulated input device

    // Get the event file with the biggest X, where X is the integer appended to event files: `eventX`

    fs::path simulatorPath("/dev/input");
    std::regex eventRegex("event[0-9]+"); // eventX where X is any full number.
    std::string eventFile{};
    for (const auto& entry : fs::directory_iterator(simulatorPath))
    {
        if (std::regex_match(entry.path().filename().string(), eventRegex))
        {
            // The first eventX file we come across will have the biggest ID.
            // Let's naively grab it.
            eventFile = entry.path().filename().string();
            break;
        }
    }

    std::string simulatedEventFilePath("/dev/input/" + eventFile);
    this->m_vDeviceFiles.emplace_back(::open(simulatedEventFilePath.c_str(), O_RDONLY));
    libevdev* simulatedInput{};
    const auto kReturnCode = ::libevdev_new_from_fd(this->m_vDeviceFiles.back(), &simulatedInput);

    if (kReturnCode != 0) // Error
    {
        std::cerr << "Failed to create a new libevdev simulated device gatherer: " << std::strerror(-kReturnCode) << std::endl;
    }

    if (this->m_bAllowConsuming)
    {
        ::libevdev_grab(simulatedInput, LIBEVDEV_GRAB);
    }

    this->m_vVirtualDevices.push_back(simulatedInput);
    this->m_vDeviceHandlers.emplace_back([&](libevdev* device){ this->m_fDeviceHandler(device); }, this->m_vVirtualDevices.back());
#endif
}

void ol::InputGathererMouse::m_fTerminate()
{
    if (this->m_bCalledTerminate) { return; }
    this->m_bCalledTerminate = true;

    for (auto& th: this->m_vDeviceHandlers)
    {
        ::pthread_kill(th.native_handle(), SIGINT);
    }

    // Wait for each thread to finish being interrupted so that we do not free any libevdev devices whilst they're still in use.
    for (size_t i = 0; i < this->m_vDeviceHandlers.size(); i++)
    {
        semShuttingDown.acquire();
    }

    for (auto& virtualDevice : this->m_vVirtualDevices)
    {
        ::libevdev_free(virtualDevice);
    }

    for (auto& fileDescriptor : this->m_vDeviceFiles)
    {
        ::close(fileDescriptor);
    }

    for (auto& th : this->m_vDeviceHandlers)
    {
        th.join();
    }

    Instance = nullptr;
}

ol::InputGathererMouse::InputGathererMouse(const bool kAllowConsuming)
{
    assert(!Instance);
    Instance = this;
    this->m_bAllowConsuming = kAllowConsuming;
    this->m_fInit();
}

void ol::InputGathererMouse::m_fDeviceHandler(libevdev *device)
{
    // TODO: Add killswitch.
    // TODO: Add support for hotplugging devices.

    bool setNextRelativeX = true, setNextRelativeY = true;
	int16_t previousX = 0, previousY = 0, previousWheel = 0;

	while (this->m_bRunning)
	{
		::input_event ev{};
		// Block until an input event is available.
		// Doing this is okay because this function will be executed in a separate thread.
		// TODO: Add error handling.
        const auto kReturnCode = ::libevdev_next_event(device, LIBEVDEV_READ_FLAG_BLOCKING, &ev);

        if (kReturnCode == -EINTR) // Thread interrupted
        {
            this->m_bRunning = false;
            semShuttingDown.release();
            return;
        }
        else if (kReturnCode != 0)
        {
            std::cerr << "Error when reading next event from libevdev: " << std::strerror(-kReturnCode) << std::endl;
            this->m_bRunning = false;
            return;
        }

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

        if (this->m_bGathering)
        {
            this->m_bufInputs.Add(input);
        }
	}
}

void ol::InputGathererMouse::m_fSignalHandler(const int32_t kSignal)
{
    Instance->m_bRunning = false;
}

void ol::InputGathererMouse::Shutdown()
{
    this->m_fTerminate();
}

ol::InputGathererMouse::~InputGathererMouse()
{
    this->Shutdown();
}

ol::Input ol::InputGathererMouse::GatherInput()
{
	return this->m_bufInputs.Get();
}

void ol::InputGathererMouse::Toggle()
{
    for (auto& device : this->m_vVirtualDevices)
    {
        ::libevdev_grab(device, this->m_bGathering ? libevdev_grab_mode::LIBEVDEV_UNGRAB : libevdev_grab_mode::LIBEVDEV_GRAB);
    }

    this->m_bGathering = !this->m_bGathering;
    this->m_bConsuming = this->m_bGathering.operator bool();
}

uint64_t ol::InputGathererMouse::AvailableInputs()
{
    return this->m_bufInputs.Length();
}

#endif
