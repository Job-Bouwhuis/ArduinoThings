#include "Components/LedController.h"

namespace Components
{
    LedController::LedController(uint8_t p) : pin(p) {
        Init(false);
    }

    LedController::LedController(uint8_t p, bool initial) : pin(p) {
        Init(initial);
    }

    void LedController::Init(bool state)
    {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, state);
    }

    void LedController::Tick()
    {
      
    }

    void LedController::SetState(bool s)
    {
        if(s)
            On();
        else
            Off();
    }

    const bool LedController::GetState() const
    {
        return digitalRead(pin) == HIGH;
    }


    void LedController::Toggle()
    {
        if(GetState())
            Off();
        else
            On();
    }

    void LedController::On()
    {
        digitalWrite(pin, HIGH);
    }

    void LedController::Off()
    {
        digitalWrite(pin, LOW);
    }
}
