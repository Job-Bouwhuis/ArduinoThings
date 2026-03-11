#pragma once

#include "Task.hpp"
#include "Components/lcd.hpp"

namespace Tasks
{
    class PrintTask : public Task
    {
    public:
        PrintTask(Components::Lcd *lcd) : lcd(lcd)
        {
        }

        void Tick() override
        {
            CoroBegin();
            lcd->Write("Hello World!");

            CoroWait(500);
            lcd->Write("This is cool");

            CoroWait(500);
            lcd->ClearLine(0);

            lcd->Write("<shield><a>yes");
            CoroEnd();
        }

    private:
        Components::Lcd *lcd;
        int i = 0;
    };
}