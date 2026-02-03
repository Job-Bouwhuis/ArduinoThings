#pragma once
#include <Arduino.h>
#include "Component.h"
#include "Util/Action.h"
#include "Util/Map.hpp"

#define LIGHT_ABOVE 0
#define LIGHT_BELOW 1
#define LIGHT_EITHER 2

namespace Components
{
        class LightSensor : public Component
        {
        public:
                LightSensor(int pin);
                ~LightSensor();

                void Tick();

                const int GetValue() const;
                void AddWatcher(int eventType, int lightLevel, void (*event)(LightSensor *, int));
                void SetUpdatesPerSecond(int ups);

        private:
                struct LightEvent
                {
                        Util::Action<LightSensor *, int> above;
                        Util::Action<LightSensor *, int> below;
                        Util::Action<LightSensor *, int> either;
                };

                int pin;
                int value = 0;
                int lastValue = 0;
                int updatesPerSecond = 0;
                int lastUpdateTime = 0;
                int updateIntervalMs = 0;

                Util::Map<int, LightEvent>
                    eventMap;
        };
}