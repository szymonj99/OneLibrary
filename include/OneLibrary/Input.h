#pragma once

#include <OneLibrary/Constants.h>
#include <OneLibrary/Enums.h>

namespace ol
{
	/**
	 * This stores the mouse input data in relative co-ordinates. This does not store if a button was clicked, as that is managed in the ol::Input class.
	 */
    // TODO: Potentially rename this to MouseCoords
	struct MouseInput
	{
        /**
         * Relative mouse movement x-coordinate since the last input event.
         */
        ol::MouseInt x = 0;
		/**
		 * Relative mouse movement y-coordinate since the last input event.
		 */
        ol::MouseInt y = 0;
		/**
		 * Relative mouse scroll movement. A value of 1 means that the user has scrolled up and -1 means that the user has scrolled down (but did they scroll up by a single notch on the scroll wheel, a single line, or a certain amount of lines?).
		 */
        ol::MouseInt scroll = 0;
        // We can either: Have which buttons were clicked here
        // Or have it as an event type on the Input struct.
	};

	/**
	 * Stores which key's state has been changed.
	 */
    // TODO: Potentially rename this, but I'm not sure what a better name would be.
	struct KeyboardInput
	{
        /**
         * The virtual key kode, the sate of which has been altered since the last event.
         */
        ol::KeyboardInt key = 0;
        //ol::KeyboardInt state = 0;
        // I think the above state is not necessary as we have the eventType enum now, which tells us if a key was pressed or released
	};

	// This class is what will be sent from the Server to the Client.
	// I think for this, adding some tests would be easy.
	// In the future, this could be made ASN.1-compliant.
    // TODO: Potentially turn this into a class
    // So that we can construct it with a type.
	/**
	 * This is a platform-agnostic approach to sending the input events between different machines.
	 */
	struct Input
	{
        /**
         * Specifies if this input comes from a mouse or keyboard.
         */
        ol::eInputType inputType = ol::eInputType::None;
        /**
         * The type of this event; this is different than inputType.
         */
		ol::eEventType eventType = ol::eEventType::None;
		/**
		 * Changes in mouse state since the last event.
		 */
		ol::MouseInput mouse = ol::MouseInput();
		/**
		 * Changes in keyboard state since the last event.
		 */
		ol::KeyboardInput keyboard = ol::KeyboardInput();
	};
}
