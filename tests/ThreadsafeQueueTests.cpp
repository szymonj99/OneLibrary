#include <catch2/catch_test_macros.hpp>

#include <OneLibrary/ThreadsafeQueue.h>

TEST_CASE("Queue can add and remove a single item", "[ThreadsafeQueue]")
{
    ol::ThreadsafeQueue<uint32_t> queue{};
    constexpr uint32_t input = 42;
    queue.Add(input);
    const uint32_t kOutput = queue.Get();
    REQUIRE_NOTHROW(input == kOutput);
}
