#pragma once

#include <TM1638plus.h>

namespace Components
{
    class WADA
    {
    public:
        WADA(byte strobe, byte clock, byte dio)
            : tm(TM1638plus(strobe, clock, dio, false))
        {
            tm.displayBegin();
            tm.displayText("helowrld");
        }

        void Write(const char *text)
        {
            tm.reset();
            tm.displayText(text);
        }

        void Write(unsigned long num)
        {
            tm.reset();
            tm.displayIntNum(num);
        }

        void WriteSeg(byte pos, byte value)
        {
            tm.reset();
            tm.display7Seg(pos, value);
        }

        void SetLed(byte pos, bool state)
        {
            tm.reset();
            tm.setLED(pos, state ? 1 : 0);
        }

    private:
        TM1638plus tm;
    };
}