#include "Components/LightSensor.h"

namespace Components
{
        LightSensor::LightSensor(int pin)
        {
                this->pin = pin;
                lastUpdateTime = 0;
                SetUpdatesPerSecond(50);
        }

        void LightSensor::SetUpdatesPerSecond(int ups)
        {
                updatesPerSecond = ups;
                if (updatesPerSecond > 0)
                        updateIntervalMs = 1000 / updatesPerSecond;
                else
                        updateIntervalMs = 0;
        }

        LightSensor::~LightSensor()
        {
        }
        void LightSensor::Tick()
        {
                unsigned long now = millis();

                if (updateIntervalMs > 0 && now - lastUpdateTime < updateIntervalMs)
                        return;

                lastUpdateTime = now;

                int newValue = analogRead(pin);

                lastValue = value;
                value = newValue;
        }

        const int LightSensor::GetValue() const
        {
                return value;
        }

        void LightSensor::AddWatcher(int eventType, int lightLevel, void (*event)(LightSensor *, int))
        {
        }
}