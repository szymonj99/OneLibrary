#include <catch2/catch_test_macros.hpp>

#include <OneLibrary/InputTable.h>

TEST_CASE("All Windows Key Codes are mapped correctly", "[InputTable]")
{
    for (const auto& firstPair : ol::KeyCodeToWindows)
    {
        REQUIRE(ol::WindowsToKeyCode.at(firstPair.second) == firstPair.first);
    }

    for (const auto& firstPair : ol::WindowsToKeyCode)
    {
        REQUIRE(ol::KeyCodeToWindows.at(firstPair.second) == firstPair.first);
    }
}

TEST_CASE("All Linux Key Codes are mapped correctly", "[InputTable]")
{
    for (const auto& firstPair : ol::KeyCodeToLinux)
    {
        REQUIRE(ol::LinuxToKeyCode.at(firstPair.second) == firstPair.first);
    }

    for (const auto& firstPair : ol::LinuxToKeyCode)
    {
        REQUIRE(ol::KeyCodeToLinux.at(firstPair.second) == firstPair.first);
    }
}

// TODO: I wonder if this is actually necessary... I don't actually think so.
//TEST_CASE("Windows and Linux have same number of mapped key codes", "[InputTable]")
//{
//    REQUIRE(ol::WindowsToKeyCode.size() == ol::KeyCodeToWindows.size());
//    REQUIRE(ol::LinuxToKeyCode.size() == ol::KeyCodeToLinux.size());
//    REQUIRE(ol::WindowsToKeyCode.size() == ol::LinuxToKeyCode.size());
//}
