#pragma once

#include <Arduino.h>
#include "Component.h"
#include "Util/Action.h"
#include "Util/List.hpp"
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
        byte pin;

        bool currentState;
        bool previousState;
        ButtonEdge edgeType = ButtonEdge::Falling;

        bool pressedTick;
        bool releasedTick;

        uint8_t debounceCounter;
        const uint8_t debounceThreshold = 2;

        bool ReadRaw() const;
        void ResetFlags();
        Debouncer<bool> debouncer;

    public:
        Util::Action<Button *> OnClick;

        Button(uint8_t p);
        Button(const Button &other)
            : pin(other.pin),
              currentState(other.currentState),
              previousState(other.previousState),
              edgeType(other.edgeType),
              pressedTick(other.pressedTick),
              releasedTick(other.releasedTick),
              debounceCounter(other.debounceCounter),
              OnClick(other.OnClick) // assumes Action supports copy
        {
            // no need to reset flags, they are already copied
        }

        Button(Button &&other) noexcept
            : pin(other.pin),
              currentState(other.currentState),
              previousState(other.previousState),
              edgeType(other.edgeType),
              pressedTick(other.pressedTick),
              releasedTick(other.releasedTick),
              debounceCounter(other.debounceCounter),
              OnClick(std::move(other.OnClick))
        {
            // optionally reset other to safe defaults
            other.currentState = false;
            other.previousState = false;
            other.pressedTick = false;
            other.releasedTick = false;
        }
        void SetEdge(ButtonEdge edge) { edgeType = edge; }

        void Tick() override;

        const bool IsHeld() const;
        const bool IsPressed() const;
        const bool IsReleased() const;
    };
}
