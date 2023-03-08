#include <catch2/catch_test_macros.hpp>

#include <OneLibrary/InputGathererKeyboard.h>
#include <OneLibrary/InputSimulatorKeyboard.h>

TEST_CASE("Keyboard Gatherer retrieves events of Keyboard Simulator", "[InputGathererKeyboard]")
{
    // Note: This will fail on Linux without sudo permissions.
    // TODO: Remove above warning once improved error handling has been implemented.
    // Capture the events so the user's keyboard doesn't press any keys.
    const auto kKeyboard = std::make_unique<ol::InputGathererKeyboard>(true);
    const auto kSimulator = std::make_unique<ol::InputSimulatorKeyboard>();

    // Let's iterate over every key code possible, and stick it into the simulator.
    // Note: Some keys, such as ol::eKeyCode::Shift can be put into the simulator
    // However, you must be careful: with Shift, it would be translated to LeftShift by Windows itself.

    const auto kLastKey = static_cast<uint16_t>(ol::eKeyCode::Fn);

    for (uint16_t i = 0; i < kLastKey; i++)
    {
        // TODO: Put these into a normal data structure.
        // Shift, Ctrl, Alt, Right Alt
        // Uh, why does passing in RightAlt give me LeftCtrl in return // Still applicable?
        if (i == 4 || i == 7 || i == 10 || i == 12) { continue; }
        ol::Input simulatedInput{};
        simulatedInput.inputType = ol::eInputType::Keyboard;
        simulatedInput.keyboard.key = static_cast<ol::eKeyCode>(i);
        simulatedInput.eventType = ol::eEventType::KBKeyDown;

        kSimulator->PerformInput(simulatedInput);

        REQUIRE(simulatedInput == kKeyboard->GatherInput());

        simulatedInput.eventType = ol::eEventType::KBKeyUp;

        kSimulator->PerformInput(simulatedInput);
        REQUIRE(simulatedInput == kKeyboard->GatherInput());
    }
}
