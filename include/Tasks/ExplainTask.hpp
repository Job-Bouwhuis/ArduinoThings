#pragma once

#include "Task.hpp"
#include "Util/Includes.h"
#include <functional>
#include <vector>

namespace Tasks
{
    class ExplainTask : public Task
    {
    public:
        ExplainTask(const std::vector<String> &messages,
                    uint32_t messageDelayMs,
                    std::function<void()> onComplete)
            : messages(messages), messageDelayMs(messageDelayMs), onComplete(onComplete)
        {
        }

        void Tick() override
        {
            CoroBegin();

            while (messageIndex < messages.size())
            {
                buzzer->PlayEffectTone(800, 500);
                lcd->Clear();
                lcd->Write(messages[messageIndex]);
                CoroWait(messageDelayMs);
                messageIndex++;
            }

            buzzer->PlayEffectTone(1000, 200);
            buzzer->PlayEffectTone(1200, 350);

            if (onComplete)
                onComplete();

            CoroEnd();
        }

    private:
        std::vector<String> messages;
        uint32_t messageDelayMs;
        std::function<void()> onComplete;
        size_t messageIndex = 0;
    };
}
