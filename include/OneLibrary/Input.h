#pragma once

#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>

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
		bool operator==(const ol::MouseInput&) const = default;
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
        ol::eKeyCode key = ol::eKeyCode::NONE;

		bool operator==(const ol::KeyboardInput&) const = default;
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

		bool operator==(const ol::Input&) const = default;
	};

    std::istream& operator>>(std::istream& stream, ol::eInputType& inputType);
    std::ostream& operator<<(std::ostream& stream, const ol::eInputType& kInputType);

    std::istream& operator>>(std::istream& stream, ol::eEventType& eventType);
    std::ostream& operator<<(std::ostream& stream, const ol::eEventType& kEventType);

    std::istream& operator>>(std::istream& stream, ol::MouseInput& mouseInput);
    std::ostream& operator<<(std::ostream& stream, const ol::MouseInput& kMouseInput);

    std::istream& operator>>(std::istream& stream, ol::KeyboardInput& keyboardInput);
    std::ostream& operator<<(std::ostream& stream, const ol::KeyboardInput& kKeyboardInput);

    std::istream& operator>>(std::istream& stream, ol::Input& input);
    std::ostream& operator<<(std::ostream& stream, const ol::Input& kInput);

    // TODO: Figure out if we need to do:
    // std::istringstream and std::ostringstream, or if we can just use std::stringstream
    // For now, I will add both.

    std::stringstream& operator>>(std::stringstream& stream, ol::eInputType& inputType);
    std::istringstream& operator>>(std::istringstream& stream, ol::eInputType& inputType);

    std::stringstream& operator<<(std::stringstream& stream, const ol::eInputType& kInputType);
    std::ostringstream& operator<<(std::ostringstream& stream, const ol::eInputType& kInputType);

    std::stringstream& operator>>(std::stringstream& stream, ol::eEventType& eventType);
    std::istringstream& operator>>(std::istringstream& stream, ol::eEventType& eventType);

    std::stringstream& operator<<(std::stringstream& stream, const ol::eEventType& kEventType);
    std::ostringstream& operator<<(std::ostringstream& stream, const ol::eEventType& kEventType);

    std::stringstream& operator>>(std::stringstream& stream, ol::MouseInput& mouseInput);
    std::istringstream& operator>>(std::istringstream& stream, ol::MouseInput& mouseInput);

    std::stringstream& operator<<(std::stringstream& stream, const ol::MouseInput& kMouseInput);
    std::ostringstream& operator<<(std::ostringstream& stream, const ol::MouseInput& kMouseInput);

    std::stringstream& operator>>(std::stringstream& stream, ol::KeyboardInput& keyboardInput);
    std::istringstream& operator>>(std::istringstream& stream, ol::KeyboardInput& keyboardInput);

    std::stringstream& operator<<(std::stringstream& stream, const ol::KeyboardInput& kKeyboardInput);
    std::ostringstream& operator<<(std::ostringstream& stream, const ol::KeyboardInput& kKeyboardInput);

    std::stringstream& operator>>(std::stringstream& stream, ol::Input& input);
    std::istringstream& operator>>(std::istringstream& stream, ol::Input& input);

    std::stringstream& operator<<(std::stringstream& stream, const ol::Input& kInput);
    std::ostringstream& operator<<(std::ostringstream& stream, const ol::Input& kInput);
}
