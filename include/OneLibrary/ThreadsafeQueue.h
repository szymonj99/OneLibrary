#pragma once

#include <cstdint>
#include <queue>
#include <mutex>
#include <condition_variable>

#include <OneLibrary/InterruptException.h>

namespace ol
{
    template <typename T>
    /**
     * A thread-safe wrapper around std::queue, which can act as a buffer in a producer-consumer scenario.
     * This is a First-In-First-Out (`FIFO`) data structure.
     * @tparam T Type of value stored.
     */
    class ThreadsafeQueue
    {
    private:
        std::queue<T> m_qCollection{};
        std::mutex m_mMutex{};
        std::condition_variable m_cvCondition;
        std::atomic<bool> m_bInterrupted = false;

    public:
        [[nodiscard]] ThreadsafeQueue() noexcept = default;
        ~ThreadsafeQueue() noexcept
        {
            // I think there isn't a point in Interrupting if we interrupted before.
            if (!this->m_bInterrupted) { this->Interrupt(); }
        }

        /**
         * Blocks until a value can be retrieved from the queue.
         * If the queue was interrupted whilst waiting for an item, this throws an `ol::InterruptException`.
         * @return A value/item that has been popped from the queue.
         */
        [[nodiscard]] T Get() noexcept(false)
        {
            std::unique_lock<std::mutex> lock(this->m_mMutex);

            this->m_cvCondition.wait(lock, [&]{ return !this->m_qCollection.empty() || this->m_bInterrupted; });
            if (this->m_bInterrupted) { throw ol::InterruptException(); }

            const T value = this->m_qCollection.front();
            this->m_qCollection.pop();

            return value;
        };
        // We will do this as a pass-by-value, as a few conversations on stack overflow suggested that since this container
        // in theory owns the objects, it's the best choice.
        // Otherwise, we have to mess with std::reference_wrapper<T> or lifetimes. Yuck!
        // And ol::Input (what we will use this for) are under 50 bytes each, this should not result in a drastic
        // performance impact.
        // Another potential approach is to store std::unique_ptr<T>, but that can be defined by our code later, rather than here.
        /**
         * Add an item to the queue. In theory, running out of space is highly unlikely.
         * @param item The item/value to be added to the queue.
         */
        void Add(const T item) noexcept
        {
            std::unique_lock<std::mutex> lock(this->m_mMutex);
            this->m_qCollection.push(item);
            this->m_cvCondition.notify_one();
        };

        /**
         * Makes all current (and I think future?) callers of `Get()` to raise an InterruptException. This can signal that the callers need to exit.
         */
        void Interrupt() noexcept
        {
            std::unique_lock<std::mutex> lock(this->m_mMutex);
            this->m_bInterrupted = true;
            this->m_cvCondition.notify_all();
        }
    };
}
