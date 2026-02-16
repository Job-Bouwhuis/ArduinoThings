#pragma once
#include "Arduino.h"

class Led2
{

public:
    Led2(byte pin) : pin(pin)
    {
    }

private:
    byte pin;
};