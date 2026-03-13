#pragma once

#include <TM1638plus.h>
#include "Util/Debouncer.hpp"
#include "Components/Component.h"
#include "Util/Util.h"
#include "Components/Button.hpp"

namespace Components
{
    class WADA : public Component
    {
    public:
        WADA(byte strobe, byte clock, byte dio)
            : tm(TM1638plus(strobe, clock, dio, false))
        {
            tm.displayBegin();
            tm.reset();
        }

        void Write(const char *text)
        {
            tm.displayText(text);
        }

        void Write(unsigned long num)
        {
            tm.displayIntNum(num);
        }

        void WriteSeg(byte pos, byte value)
        {
            tm.display7Seg(pos, value);
        }

        void SetLed(byte pos, bool state)
        {
            tm.setLED(pos, state ? 1 : 0);
        }

        void reset()
        {
            tm.reset();
        }

        Components::Button *GetButton(byte index)
        {
            return &buttons[index];
        }

        void Tick() override
        {
            auto buttonStates = tm.readButtons();
            for (int i = 0; i < 8; i++)
            {
                bool cur = buttonStates & 0x01;
                buttons[i].UpdateState(cur);
                buttonStates >>= 1;
            }
        }

    private:
        TM1638plus tm;
        Components::Button buttons[8] = {
            Components::Button(),
            Components::Button(),
            Components::Button(),
            Components::Button(),
            Components::Button(),
            Components::Button(),
            Components::Button(),
            Components::Button()};
    };
}
