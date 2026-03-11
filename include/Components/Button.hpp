#pragma once

#include <Arduino.h>
#include "Component.h"
#include "Util/Action.h"
#include "Util/Debouncer.hpp"

namespace Components
{
    enum class ButtonEdge
    {
        Rising,
        Falling
    };

    class Button : public Component
    {
    private:
        byte pin = 0;
        bool byPin = true; // added for wada, so that i dont have to write another button specifically for the wada. im lazy

        bool currentState = false;
        bool previousState = false;
        ButtonEdge edgeType = ButtonEdge::Rising;

        bool pressedTick = false;
        bool releasedTick = false;

        Debouncer debouncer;
        const uint8_t debounceThreshold = 2;

        bool ReadRaw() const
        {
            return digitalRead(pin) == LOW;
        }

        void ResetFlags()
        {
            pressedTick = false;
            releasedTick = false;
        }

    public:
        Util::Action<Button *> OnClick;

        Button() : byPin(false), debouncer(debounceThreshold) {}

        Button(uint8_t p) : pin(p), byPin(true), debouncer(debounceThreshold)
        {
            pinMode(pin, INPUT_PULLUP);
            currentState = ReadRaw();
            previousState = currentState;
            debouncer.Update(currentState);
        }

        // Copy constructor
        Button(const Button &other)
            : pin(other.pin),
              byPin(other.byPin),
              currentState(other.currentState),
              previousState(other.previousState),
              edgeType(other.edgeType),
              pressedTick(other.pressedTick),
              releasedTick(other.releasedTick),
              debouncer(other.debouncer),
              OnClick(other.OnClick)
        {
        }

        // Move constructor
        Button(Button &&other) noexcept
            : pin(other.pin),
              byPin(other.byPin),
              currentState(other.currentState),
              previousState(other.previousState),
              edgeType(other.edgeType),
              pressedTick(other.pressedTick),
              releasedTick(other.releasedTick),
              debouncer(std::move(other.debouncer)),
              OnClick(std::move(other.OnClick))
        {
            other.currentState = false;
            other.previousState = false;
            other.pressedTick = false;
            other.releasedTick = false;
        }

        void SetEdge(ButtonEdge edge) { edgeType = edge; }

        void Tick() override
        {
            if (byPin)
                UpdateState(ReadRaw());
        }

        void UpdateState(bool raw)
        {
            ResetFlags();

            if (debouncer.Update(raw))
            {
                // Pressed
                if (debouncer.Get() && !debouncer.Previous())
                {
                    pressedTick = true;
                    if (edgeType == ButtonEdge::Falling)
                        OnClick(this);
                }

                // Released
                if (!debouncer.Get() && debouncer.Previous())
                {
                    releasedTick = true;
                    if (edgeType == ButtonEdge::Rising)
                        OnClick(this);
                }
            }
        }

        const bool IsHeld() const { return debouncer.Get(); }
        const bool IsPressed() const { return pressedTick; }
        const bool IsReleased() const { return releasedTick; }
    };
}
