#pragma once
#include <list>
#include <functional>

namespace Util
{
    template <typename... Args>
    class Action
    {
    private:
        std::list<std::function<void(Args...)>> callbacks;

    public:
        Action() = default;

        template <typename F>
        void Add(F &&f)
        {
            callbacks.push_back(std::forward<F>(f));
        }

        void Clear()
        {
            callbacks.clear();
        }

        bool HasSubs() const
        {
            return !callbacks.empty();
        }

        void Invoke(Args... args)
        {
            for (auto &cb : callbacks)
            {
                cb(args...);
            }
        }

        void operator()(Args... args)
        {
            Invoke(args...);
        }
    };
}