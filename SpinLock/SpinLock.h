#pragma once

#include <atomic>
#include <thread>

#if defined(__i386__) || defined(__x86_64__)
    #include <emmintrin.h>
    #define SPINLOCK_PAUSE() _mm_pause()
#elif defined(__arm__)|| defined(__aarch64__)
    #include <arm_acle.h>
    #define SPINLOCK_PAUSE() __yield()
#else
    #warning "SpinLock: CPU architecture not supported"
    #define SPINLOCK_PAUSE()
#endif

class SpinLock
{
public:
    bool try_lock() noexcept
    {
        return !mFlag.test_and_set(std::memory_order_acquire);
    }

    void lock() noexcept
    {
        for (int i = 0; i < 5; ++i)
        {
            if (try_lock())
                return;
        }

        for (int i = 0; i < 10; ++i)
        {
            if (try_lock())
                return;
            SPINLOCK_PAUSE();
        }

        while (true)
        {
            for (int i = 0; i < 3000; ++i)
            {
                if (try_lock())
                    return;
                SPINLOCK_PAUSE();
                SPINLOCK_PAUSE();
                SPINLOCK_PAUSE();
                SPINLOCK_PAUSE();
                SPINLOCK_PAUSE();
                SPINLOCK_PAUSE();
                SPINLOCK_PAUSE();
                SPINLOCK_PAUSE();
                SPINLOCK_PAUSE();
                SPINLOCK_PAUSE();
            }
            std::this_thread::yield();
        }
    }

    void unlock() noexcept
    {
        mFlag.clear(std::memory_order_release);
    }

private:
    std::atomic_flag mFlag = ATOMIC_FLAG_INIT;
};
