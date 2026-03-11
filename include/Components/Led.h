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
        // Constructors
        Led(byte p, bool initialState) : pin(p) {}
        Led(byte p) : pin(p) { Init(false); }

        // Copy constructor
        Led(const Led &other) : pin(other.pin)
        {
            pinMode(pin, OUTPUT);
        }

        // Move constructor
        Led(Led &&other) noexcept : pin(other.pin)
        {
            pinMode(pin, OUTPUT);
        }

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
