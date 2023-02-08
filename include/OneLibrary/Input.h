#pragma once

#include <OneLibrary/Constants.h>
#include <OneLibrary/Enums.h>

namespace ol
{
	struct MouseInput
	{
        ol::MouseInt x = 0;
        ol::MouseInt y = 0;
        ol::MouseInt scroll = 0;
        // We can either: Have which buttons were clicked here
        // Or have it as an event type on the Input struct.
	};

	struct KeyboardInput
	{
        ol::KeyboardInt key = 0;
        //ol::KeyboardInt state = 0;
        // I think the above state is not necessary as we have the eventType enum now, which tells us if a key was pressed or released
	};

	// This class is what will be sent from the Server to the Client.
	// I think for this, adding some tests would be easy.
	// In the future, this could be made ASN.1-compliant.
    // TODO: Potentially turn this into a class
    // So that we can construct it with a type.
	struct Input
	{
        ol::eInputType inputType = ol::eInputType::None;
		ol::eEventType eventType = ol::eEventType::None;
		ol::MouseInput mouse = ol::MouseInput();
		ol::KeyboardInput keyboard = ol::KeyboardInput();
	};
}
