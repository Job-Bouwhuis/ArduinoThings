#pragma once

#include <Arduino.h>

namespace Components
{
    class Potentiometer
    {
    public:
        Potentiometer(uint8_t pin, float minValue = 0.0f, float maxValue = 1.0f)
            : analogPin(pin), minVal(minValue), maxVal(maxValue)
        {
            pinMode(pin, INPUT);
        }

        void SetRange(float minValue, float maxValue)
        {
            minVal = minValue;
            maxVal = maxValue;
        }

        float GetValue()
        {
            int raw = analogRead(analogPin);
            float normalized = float(raw) / 1023.0f;
            return minVal + normalized * (maxVal - minVal);
        }

        inline int GetValueRounded()
        {
            return static_cast<int>(GetValue() + 0.5f);
        }

    private:
        uint8_t analogPin;
        float minVal;
        float maxVal;
    };
}