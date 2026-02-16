#pragma once
#include <Arduino.h>
#include "Component.h"

namespace Components
{
    class Led : public Component
    {
    private:
        byte pin;

        void Init(bool initial);

    public:
        Led(byte pin, bool initialState);
        Led(byte pin);

        byte GetPin()
        {
            return pin;
        }

        void Tick() override;

        void SetState(bool state);
        void Toggle();
        void On();
        void Off();

        const bool GetState() const;
    };
}
