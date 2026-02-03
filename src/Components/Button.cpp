#include "Components/Button.h"

namespace Components
{
    Button::Button(uint8_t p) : pin(p),
                                currentState(false),
                                previousState(false),
                                pressedTick(false),
                                releasedTick(false),
                                debounceCounter(0)
    {
        pinMode(pin, INPUT_PULLUP);
        currentState = ReadRaw();
        previousState = currentState;
    }

    bool Button::ReadRaw() const
    {
        return digitalRead(pin) == LOW;
    }

    void Button::Tick()
    {
        ResetFlags();

        bool raw = ReadRaw();

        if (raw != currentState)
        {
            debounceCounter++;

            if (debounceCounter >= debounceThreshold)
            {
                previousState = currentState;
                currentState = raw;
                debounceCounter = 0;

                // Pressed
                if (currentState && !previousState)
                {
                    pressedTick = true;

                    if (edgeType == ButtonEdge::Falling)
                        OnClick(this);
                }

                // Released
                if (!currentState && previousState)
                {
                    releasedTick = true;

                    if (edgeType == ButtonEdge::Rising)
                        OnClick(this);
                }
            }
        }
        else
        {
            debounceCounter = 0;
        }
    }

    const bool Button::IsHeld() const { return currentState; }

    const bool Button::IsPressed() const { return pressedTick; }

    const bool Button::IsReleased() const { return releasedTick; }

    void Button::ResetFlags()
    {
        pressedTick = false;
        releasedTick = false;
    }
}
