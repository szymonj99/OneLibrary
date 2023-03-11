#include <catch2/catch_test_macros.hpp>

#include <OneLibrary/InputGathererMouse.h>
#include <OneLibrary/InputSimulatorMouse.h>

TEST_CASE("Mouse Gatherer retrieves event of Mouse Simulator", "[InputGathererMouse]")
{
    // Note: This will fail on Linux without sudo permissions.
    // TODO: Remove above warning once improved error handling has been implemented.
    // Capture the events so the user's mouse doesn't wobble all over the screen.
    const auto kSimulator = std::make_unique<ol::InputSimulatorMouse>();
    const auto kMouse = std::make_unique<ol::InputGathererMouse>(true);

    ol::Input input{};
    input.inputType = ol::eInputType::Mouse;
    input.eventType = ol::eEventType::MMove;
    input.mouse.x = 10;
    input.mouse.y = 0;
    // This fails on Windows.
    // I think that's because of mouse acceleration or some multiplier.
    // When let's say x = 10, the input we capture is x = 7.
    kSimulator->PerformInput(input);
    const auto kSimulatedInput = kMouse->GatherInput();
    REQUIRE(input == kSimulatedInput);

    input.mouse.x = 0;
    input.mouse.y = 10;
    kSimulator->PerformInput(input);
    REQUIRE(input == kMouse->GatherInput());
}
