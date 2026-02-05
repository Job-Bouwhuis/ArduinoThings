#include "Huiswerk/Week1/solution.h"

namespace Huiswerk::Week1::Opdracht1
{
    void Solution::Tick()
    {
        float times[] = {
            0.1f, 0.2f, 0.3f, 0.4f, 0.5f,
            0.6f, 0.7f, 0.8f, 0.9f, 1.0f};

        Components::Led led(LED_BUILTIN);

        for (auto &time : times)
        {
            led.On();
            delay(time * 1000);
            led.Off();
            delay(100);
        }

        led.Off();
    }
}

namespace Huiswerk::Week1::Opdracht2
{
    Solution::Solution() : btn(Components::Button(USER_BTN)), led(Components::Led(LED_BUILTIN))
    {
    }
    void Solution::Tick()
    {
        btn.Tick();
        if (btn.IsPressed())
            led.On();
        if (btn.IsReleased())
            led.Off();
    }
}

namespace Huiswerk::Week1::Opdracht3
{
    Solution::Solution() : btn(Components::Button(USER_BTN))
    {
    }

    void Solution::Tick()
    {
        btn.Tick();

        if (btn.IsPressed())
            holdStart = millis();

        if (!btn.IsReleased())
            return;
        int time = millis();
        int diff = time - holdStart;

        Console.WriteLine(diff);
    }
}