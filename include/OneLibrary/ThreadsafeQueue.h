#pragma once

#include <cstdint>
#include <semaphore>
#include <queue>

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
        // A semaphore representative of the empty spaces is not necessary, as queue will expand as needed.
        //std::numeric_limits<uint32_t>::max()
        std::counting_semaphore<2147483647> m_sItems{0};
        std::binary_semaphore m_sMutex{1};
        std::atomic<uint64_t> m_uiLength{0};

    public:
        [[nodiscard]] ThreadsafeQueue() noexcept = default;
        /**
         * Blocks until a value can be retrieved from the queue.
         * @return A value/item that has been popped from the queue.
         */
        [[nodiscard]] T Get() noexcept
        {
            // Could I make my own wrapper for this that would throw my own exception when a thread is timed out?
            this->m_sItems.acquire();
            this->m_sMutex.acquire();

            const T value = this->m_qCollection.front();
            this->m_qCollection.pop();
            this->m_uiLength--;

            this->m_sMutex.release();

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
            this->m_sMutex.acquire();

            this->m_qCollection.push(item);
            this->m_uiLength++;

            this->m_sItems.release();
            this->m_sMutex.release();
        };

        [[nodiscard]] [[gnu::pure]] uint64_t Length() noexcept
        {
            return this->m_uiLength;
        }
    };
}
