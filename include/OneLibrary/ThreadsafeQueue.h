#pragma once

#include <cstdint>
#include <semaphore>
#include <queue>

namespace ol
{
    template <typename T>
    class ThreadsafeQueue
    {
    private:
        std::queue<T> m_qCollection{};
        // A semaphore representative of the empty spaces is not necessary, as queue will expand as needed.
        //std::numeric_limits<uint32_t>::max()
        std::counting_semaphore<4294967295> m_sItems{0};
        std::binary_semaphore m_sMutex{1};
    public:
        [[nodiscard]] ThreadsafeQueue() noexcept = default;
        [[nodiscard]] T Get() noexcept
        {
            this->m_sItems.acquire();
            this->m_sMutex.acquire();

            const T value = this->m_qCollection.front();
            this->m_qCollection.pop();

            this->m_sMutex.release();

            return value;
        };
        // We will do this as a pass-by-value, as a few conversations on stack overflow suggested that since this container
        // in theory owns the objects, it's the best choice.
        // Otherwise, we have to mess with std::reference_wrapper<T> or lifetimes. Yuck!
        // And ol::Input (what we will use this for) are under 50 bytes each, this should not result in a drastic
        // performance impact.
        // Another potential approach is to store std::unique_ptr<T>, but that can be defined by our code later, rather than here.
        void Add(const T item) noexcept
        {
            this->m_sMutex.acquire();

            this->m_qCollection.push(item);

            this->m_sItems.release();
            this->m_sMutex.release();
        };
    };
}