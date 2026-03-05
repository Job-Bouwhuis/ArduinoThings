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
        debouncer.Update(ReadRaw());
    }

    bool Button::ReadRaw() const
    {
        return digitalRead(pin) == LOW;
    }

    void Button::Tick()
    {
        bool raw = ReadRaw();

        if (debouncer.Update(raw))
        {
            // pressed
            if (debouncer.Get() && !debouncer.Previous())
            {
                pressedTick = true;
                if (edgeType == ButtonEdge::Falling)
                    OnClick(this);
            }

            // released
            if (!debouncer.Get() && debouncer.Previous())
            {
                releasedTick = true;
                if (edgeType == ButtonEdge::Rising)
                    OnClick(this);
            }
        }
    }

    const bool Button::IsHeld() const { return debouncer.Get(); }
    const bool Button::IsPressed() const { return pressedTick; }
    const bool Button::IsReleased() const { return releasedTick; }
    void Button::ResetFlags()
    {
        pressedTick = false;
        releasedTick = false;
    }
}
