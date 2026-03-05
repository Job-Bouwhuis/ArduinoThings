#include "Components/Led.h"

namespace Components
{
    void Led::Init(bool state)
    {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, state);
    }

    void Led::Tick()
    {
    }

    void Led::SetState(bool s)
    {
        if (s)
            On();
        else
            Off();
    }

    const bool Led::GetState() const
    {
        return digitalRead(pin) == HIGH;
    }

    void Led::Toggle()
    {
        if (GetState())
            Off();
        else
            On();
    }

    void Led::On()
    {
        digitalWrite(pin, HIGH);
    }

    void Led::Off()
    {
        digitalWrite(pin, LOW);
    }
}
