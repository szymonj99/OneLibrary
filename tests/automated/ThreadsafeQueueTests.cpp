#include <catch2/catch_test_macros.hpp>

#include <thread>

#include <OneLibrary/ThreadsafeQueue.h>

TEST_CASE("Queue can add and remove a single item", "[ThreadsafeQueue]")
{
    ol::ThreadsafeQueue<uint32_t> queue{};
    constexpr uint32_t input = 42;
    queue.Add(input);
    const uint32_t kOutput = queue.Get();
    REQUIRE_NOTHROW(input == kOutput);
}

// TODO: Add test cases with multiple threads accessing the queue at the same time.
TEST_CASE("Interrupting while Getting from an empty queue raises an exception", "[ThreadsafeQueue]")
{
    ol::ThreadsafeQueue<uint32_t> queue{};

    // TODO: Rework this to not have a hard-coded timer; but instead have the main thread signal the std::jthread that it can interrupt.
    std::jthread th{[&](){ std::this_thread::sleep_for(std::chrono::seconds{1}); queue.Interrupt(); }};

    bool ex = false;
    try
    {
        (void)queue.Get();
    }
    catch (const ol::InterruptException& e)
    {
        ex = true;
    }

    REQUIRE(ex);
}
