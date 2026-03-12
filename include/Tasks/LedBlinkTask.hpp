#pragma once
#include "Task.hpp"
#include <Arduino.h>
#include "Components/Led.h"

namespace Tasks
{
    class BlinkTask : public Task
    {
    public:
        BlinkTask(Components::Led led, uint32_t interval, uint32_t flashes)
            : led(led), interval(interval), remaining(flashes)
        {
            led.Off();
        }

        void Tick() override
        {
            CoroBegin();

            while (remaining > 0)
            {
                CoroWait(interval);

                led.Toggle();
                if (!led.GetState())
                    remaining--;
            }

            CoroEnd();
        }

    private:
        Components::Led led;
        uint32_t interval;
        uint32_t remaining;
    };
}
