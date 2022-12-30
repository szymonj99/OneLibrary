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
	};

	struct KeyboardInput
	{
        ol::KeyboardInt key = 0;
        ol::KeyboardInt state = 0;
	};

	// This class is what will be sent from the Server to the Client.
	// I think for this, adding some tests would be easy.
	// In the future, this could be made ASN.1-compliant.
	struct Input
	{
		ol::eInputType type = ol::eInputType::Uninitialised;
		ol::MouseInput mouse = ol::MouseInput();
		ol::KeyboardInput keyboard = ol::KeyboardInput();
	};
}
