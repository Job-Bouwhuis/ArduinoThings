#pragma once
#include "List.hpp"
#include <functional>
#include <memory>

namespace Util
{
    template <typename... Args>
    class Action
    {
    private:
        List<std::function<void(Args...)>> callbacks;

    public:
        std::shared_ptr<std::function<void(Args...)>> Add(const std::function<void(Args...)> &fn)
        {
            callbacks.Add(fn);
            return callbacks.Get(callbacks.Count() - 1);
        }

        template <typename F>
        std::shared_ptr<std::function<void(Args...)>> Add(F &&f)
        {
            std::function<void(Args...)> fn(std::forward<F>(f));
            callbacks.Add(std::move(fn));
            return callbacks.Get(callbacks.Count() - 1);
        }

        void Remove(const std::shared_ptr<std::function<void(Args...)>> &handle)
        {
            if (!handle)
                return;
            for (SIZE i = 0; i < callbacks.Count(); ++i)
            {
                if (callbacks.Get(i) == handle)
                {
                    callbacks.RemoveAt(i);
                    return;
                }
            }
        }

        void Invoke(Args... args)
        {
            for (SIZE i = 0; i < callbacks.Count(); ++i)
            {
                auto cb = callbacks[i];
                if (cb && *cb)
                {
                    (*cb)(args...);
                }
            }
        }

        void operator()(Args... args) { Invoke(args...); }

        void Clear() { callbacks.Clear(); }
    };
}
