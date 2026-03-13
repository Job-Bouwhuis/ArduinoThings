#pragma once

#define CoroBegin()      \
    switch (__coro_line) \
    {                    \
    case 0:

#define CoroYieldOnce()     \
    __coro_line = __LINE__; \
    return;                 \
    case __LINE__:

#define CoroWait(ms)                     \
    __coro_waitUntil = millis() + (ms);  \
    __coro_line = __LINE__;              \
    return;                              \
    case __LINE__:                       \
        if (millis() < __coro_waitUntil) \
            return;

#define CoroEnd() \
    }             \
    __finished = true;

namespace Tasks
{
    class Task
    {
    public:
        bool deleteOnRemoved = true;

        virtual ~Task() = default;
        virtual void Tick() = 0;
        bool IsFinished() const { return __finished; }

        inline void Stop()
        {
            __finished = true;
        }

        /// @brief honestly this isnt very safe, but it works for what im doing, so im happy with it
        inline void OnRemoved()
        {
            if (deleteOnRemoved)
                delete this;
        }

    protected:
        int __coro_line = 0;
        bool __finished = false;
        uint32_t __coro_waitUntil = 0;
    };
}