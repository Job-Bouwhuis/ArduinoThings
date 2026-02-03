#pragma once
#include <Arduino.h>
#include "Component.h"

namespace Components
{
    class LedController : public Component
    {
    private:
        byte pin;

        void Init(bool initial);
        
    public:
        LedController(byte pin, bool initialState);
        LedController(byte pin);

        void Tick() override;

        void SetState(bool state);
        void Toggle();
        void On();
        void Off();

        const bool GetState() const;
    };
}
