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

                for (auto &kv : eventMap)
                {
                        int threshold = kv.key;
                        LightEvent &ev = kv.value;

                        bool crossedBelow = (lastValue >= threshold && newValue < threshold);
                        bool crossedAbove = (lastValue <= threshold && newValue > threshold);

                        if (crossedBelow)
                                ev.below(this, newValue);
                        if (crossedAbove)
                                ev.above(this, newValue);
                        if (crossedBelow || crossedAbove)
                                ev.either(this, newValue);
                }

                lastValue = newValue;
                value = newValue;
        }

        const int LightSensor::GetValue() const
        {
                return value;
        }

        void LightSensor::AddWatcher(int eventType, int lightLevel, void (*event)(LightSensor *, int))
        {
                auto ev = eventMap.Get(lightLevel);
                if (ev == nullptr)
                {
                        LightEvent newEv;
                        eventMap.Set(lightLevel, newEv);
                        ev = eventMap.Get(lightLevel);
                }

                switch (eventType)
                {
                case LIGHT_BELOW:
                        ev->below.Add(event);
                        break;
                case LIGHT_ABOVE:
                        ev->above.Add(event);
                        break;
                case LIGHT_EITHER:
                        ev->either.Add(event);
                        break;
                default:
                        // TODO: use a better erroring system than nothing
                        break;
                }
        }
}