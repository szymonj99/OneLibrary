#ifdef OS_LINUX

#include <OneLibrary/InputGathererKeyboard.h>

namespace fs = std::filesystem;

namespace ol
{
    // Not quite a singleton pattern, but almost.
    ol::InputGathererKeyboard* Instance = nullptr;
    // 20 is just a random value I picked that should be high enough to handle all threads shutting down.
    std::counting_semaphore<20> semShuttingDown{0};
}

ol::InputGathererKeyboard::InputGathererKeyboard(const bool kAllowConsuming)
{
    assert(!Instance);
    Instance = this;
    this->m_bAllowConsuming = kAllowConsuming;

    // We add a signal handler so that libevdev_next_event can exit early if the threads are interrupted.
    // Note: Using std::signal does not work here.
    // Using std::signal does not make `libevdev_next_event` return -EINTR, ever.
    struct sigaction sa{};
    sa.sa_handler = ol::InputGathererKeyboard::m_fSignalHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, nullptr);

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

            const auto kReturnCode = ::libevdev_new_from_fd(this->m_vDeviceFiles.back(), &input);
            if (kReturnCode != 0) // Error
            {
                std::cerr << "Could not create a new libevdev device: " << std::strerror(-kReturnCode) << std::endl;
                this->m_bRunning = false;
                return;
            }

    		// When we grab the input, the events are not passed to any other window.
            if (this->m_bAllowConsuming)
            {
                // TODO: Add error handling
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

void ol::InputGathererKeyboard::m_fSignalHandler(const int32_t signal)
{
    // This only ever receives SIGINT == 2 as that is the only registered signal.
    Instance->m_bRunning = false;
}

void ol::InputGathererKeyboard::m_fDeviceHandler(libevdev* device)
{
    // TODO: Add killswitch.
    // TODO: Add support for hotplugging devices.

	while (this->m_bRunning)
	{
		::input_event ev{};
		// Block until an input event is available.
		// Doing this is okay because this function will be executed in a separate thread.
        // It gets interrupted by SIGINT when the threads want to exit as we call `pthread_kill`.
		// TODO: Add error handling.
		const auto kReturnCode = ::libevdev_next_event(device, LIBEVDEV_READ_FLAG_BLOCKING, &ev);

        // This is only ever called with sigaction, and not with std::signal.
        if (kReturnCode == -EINTR) // This has been interrupted
        {
            this->m_bRunning = false;
            semShuttingDown.release();
            return;
        }
        else if (kReturnCode != 0) // Error
        {
            // std::strerror is not thread-safe
            // TODO: Use a thread-safe alternative.
            std::cerr << "Error when trying to get the next event from a Keyboard Device Handler: " << std::strerror(-kReturnCode) << std::endl;
            this->m_bRunning = false;
            return;
        }
        // else kReturnCode == 0 == success

		ol::Input input{};
		input.inputType = ol::eInputType::Keyboard;

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
                std::cout << "Got unhandled keyboard event type: (int) " << ev.type << std::endl;
                continue;
            }
		}

        if (input.keyboard.key == ol::eKeyCode::End)
        {
            this->m_bRunning = false;
        }

        // Remember to return any special key codes. Otherwise, how will the caller know when a new Toggle key has been pressed?
        // TODO: Potentially move this into a function.
        // TODO: How do we want to handle the Toggle And shutdown keys?
        if (this->m_bGathering || input.keyboard.key == ol::eKeyCode::Home || input.keyboard.key == ol::eKeyCode::End)
        {
            this->m_bufInputs.Add(input);
        }
	}
}

ol::InputGathererKeyboard::~InputGathererKeyboard()
{
    for (auto& th: this->m_vDeviceHandlers)
    {
        ::pthread_kill(th.native_handle(), SIGINT);
    }

    // Wait for each thread to finish being interrupted so that we do not free any libevdev devices whilst they're still in use.
    for (size_t i = 0; i < this->m_vDeviceHandlers.size(); i++)
    {
        semShuttingDown.acquire();
    }

    // Note, calling `libevdev_free` only frees the resources, and does not make `libevdev_next_event` blocking call to exit early.
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

ol::Input ol::InputGathererKeyboard::GatherInput()
{
	return this->m_bufInputs.Get();
}

void ol::InputGathererKeyboard::Toggle()
{
    for (auto& device : this->m_vVirtualDevices)
    {
        ::libevdev_grab(device, this->m_bGathering ? libevdev_grab_mode::LIBEVDEV_UNGRAB : libevdev_grab_mode::LIBEVDEV_GRAB);
    }

    this->m_bGathering = !this->m_bGathering;
    this->m_bConsuming = this->m_bGathering.operator bool();
}

#endif
