#pragma once

#include <Arduino.h>
#include "Component.h"
#include "Util/Action.h"
#include "Util/List.hpp"

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

    public:
        Util::Action<Button *> OnClick;

        Button(uint8_t p);
        void SetEdge(ButtonEdge edge) { edgeType = edge; }

        void Tick() override;

        const bool IsHeld() const;
        const bool IsPressed() const;
        const bool IsReleased() const;
    };
}
