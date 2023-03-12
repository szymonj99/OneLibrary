#include <OneLibrary/Input.h>

namespace ol
{
    // Note: StringStreams use the " " space as a delimiter.
    // TODO: It would be nice to improve the format of how these are serialised.
    // Doing so would allow us to call std::cout and have the output actually be readable rather than having just a few numbers together.

    std::istream& operator>>(std::istream& stream, ol::eInputType& inputType)
    {
        uint16_t t;
        stream >> t;
        inputType = static_cast<ol::eInputType>(t);
        return stream;
    }

    std::ostream& operator<<(std::ostream& stream, const ol::eInputType& kInputType)
    {
        stream << static_cast<uint16_t>(kInputType);
        return stream;
    }

    std::istream& operator>>(std::istream& stream, ol::eEventType& eventType)
    {
        uint16_t e;
        stream >> e;
        eventType = static_cast<ol::eEventType>(e);
        return stream;
    }

    std::ostream& operator<<(std::ostream& stream, const ol::eEventType& kEventType)
    {
        stream << static_cast<uint16_t>(kEventType);
        return stream;
    }

    std::istream& operator>>(std::istream& stream, ol::MouseInput& mouseInput)
    {
        stream >> mouseInput.x >> mouseInput.y >> mouseInput.scroll;
        return stream;
    }

    std::ostream& operator<<(std::ostream& stream, const ol::MouseInput& kMouseInput)
    {
        stream << kMouseInput.x << " " << kMouseInput.y << " " << kMouseInput.scroll;
        return stream;
    }

    std::istream& operator>>(std::istream& stream, ol::KeyboardInput& keyboardInput)
    {
        uint16_t k;
        stream >> k;
        keyboardInput.key = static_cast<ol::eKeyCode>(k);
        return stream;
    }

    std::ostream& operator<<(std::ostream& stream, const ol::KeyboardInput& kKeyboardInput)
    {
        stream << static_cast<uint16_t>(kKeyboardInput.key);
        return stream;
    }

    std::istream& operator>>(std::istream& stream, ol::Input& input)
    {
        stream >> input.inputType;
        stream >> input.eventType;
        stream >> input.mouse;
        stream >> input.keyboard;
        stream >> input.clipboard;
        return stream;
    }

    std::ostream& operator<<(std::ostream& stream, const ol::Input& kInput)
    {
        stream << kInput.inputType << " " << kInput.eventType << " " << kInput.mouse << " " << kInput.keyboard << " " << kInput.clipboard;
        return stream;
    }

    std::stringstream& operator>>(std::stringstream& stream, ol::eInputType& inputType)
    {
        uint16_t t;
        stream >> t;
        inputType = static_cast<ol::eInputType>(t);
        return stream;
    }

    std::istringstream& operator>>(std::istringstream& stream, ol::eInputType& inputType)
    {
        uint16_t t;
        stream >> t;
        inputType = static_cast<ol::eInputType>(t);
        return stream;
    }

    std::stringstream& operator<<(std::stringstream& stream, const ol::eInputType& kInputType)
    {
        stream << static_cast<uint16_t>(kInputType);
        return stream;
    }

    std::ostringstream& operator<<(std::ostringstream& stream, const ol::eInputType& kInputType)
    {
        stream << static_cast<uint16_t>(kInputType);
        return stream;
    }

    std::stringstream& operator>>(std::stringstream& stream, ol::eEventType& eventType)
    {
        uint16_t e;
        stream >> e;
        eventType = static_cast<ol::eEventType>(e);
        return stream;
    }

    std::istringstream& operator>>(std::istringstream& stream, ol::eEventType& eventType)
    {
        uint16_t e;
        stream >> e;
        eventType = static_cast<ol::eEventType>(e);
        return stream;
    }

    std::stringstream& operator<<(std::stringstream& stream, const ol::eEventType& kEventType)
    {
        stream << static_cast<uint16_t>(kEventType);
        return stream;
    }

    std::ostringstream& operator<<(std::ostringstream& stream, const ol::eEventType& kEventType)
    {
        stream << static_cast<uint16_t>(kEventType);
        return stream;
    }

    std::stringstream& operator>>(std::stringstream& stream, ol::MouseInput& mouseInput)
    {
        stream >> mouseInput.x >> mouseInput.y >> mouseInput.scroll;
        return stream;
    }

    std::istringstream& operator>>(std::istringstream& stream, ol::MouseInput& mouseInput)
    {
        stream >> mouseInput.x >> mouseInput.y >> mouseInput.scroll;
        return stream;
    }

    std::stringstream& operator<<(std::stringstream& stream, const ol::MouseInput& kMouseInput)
    {
        stream << kMouseInput.x << " " << kMouseInput.y << " " << kMouseInput.scroll;
        return stream;
    }

    std::ostringstream& operator<<(std::ostringstream& stream, const ol::MouseInput& kMouseInput)
    {
        stream << kMouseInput.x << " " << kMouseInput.y << " " << kMouseInput.scroll;
        return stream;
    }

    std::stringstream& operator>>(std::stringstream& stream, ol::KeyboardInput& keyboardInput)
    {
        uint16_t k;
        stream >> k;
        keyboardInput.key = static_cast<ol::eKeyCode>(k);
        return stream;
    }

    std::istringstream& operator>>(std::istringstream& stream, ol::KeyboardInput& keyboardInput)
    {
        uint16_t k;
        stream >> k;
        keyboardInput.key = static_cast<ol::eKeyCode>(k);
        return stream;
    }

    std::stringstream& operator<<(std::stringstream& stream, const ol::KeyboardInput& kKeyboardInput)
    {
        stream << static_cast<uint16_t>(kKeyboardInput.key);
        return stream;
    }

    std::ostringstream& operator<<(std::ostringstream& stream, const ol::KeyboardInput& kKeyboardInput)
    {
        stream << static_cast<uint16_t>(kKeyboardInput.key);
        return stream;
    }

    std::stringstream& operator>>(std::stringstream& stream, ol::Input& input)
    {
        stream >> input.inputType;
        stream >> input.eventType;
        stream >> input.mouse;
        stream >> input.keyboard;
        stream >> input.clipboard;
        return stream;
    }

    std::istringstream& operator>>(std::istringstream& stream, ol::Input& input)
    {
        stream >> input.inputType;
        stream >> input.eventType;
        stream >> input.mouse;
        stream >> input.keyboard;
        stream >> input.clipboard;
        return stream;
    }

    std::stringstream& operator<<(std::stringstream& stream, const ol::Input& kInput)
    {
        stream << kInput.inputType << " " << kInput.eventType << " " << kInput.mouse << " " << kInput.keyboard << " " << kInput.clipboard;
        return stream;
    }

    std::ostringstream& operator<<(std::ostringstream& stream, const ol::Input& kInput)
    {
        stream << kInput.inputType << " " << kInput.eventType << " " << kInput.mouse << " " << kInput.keyboard << " " << kInput.clipboard;
        return stream;
    }
}
