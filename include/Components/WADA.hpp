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

        void ClearButtonEvents()
        {
            for (int i = 0; i < 8; i++)
                GetButton(i)->OnClick.Clear();
        }

        void WriteByteAsBits(byte num)
        {
            char bits[9];
            for (byte i = 0; i < 8; ++i)
                bits[i] = ((num >> (7 - i)) & 1) ? '1' : '0';
            bits[8] = '\0';

            Write(bits);
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
                auto v = buttonStates & (1 << i);
                buttons[i].UpdateState(v);
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
