#pragma once

class Debouncer
{
private:
    bool currentValue;
    bool previousValue;
    bool pendingValue;
    bool hasPending;
    unsigned long lastChangeTime;
    unsigned long thresholdMs;

public:
    Debouncer(unsigned long debounceMs = 15)
        : currentValue(), previousValue(), pendingValue(),
          hasPending(false), lastChangeTime(0), thresholdMs(debounceMs)
    {
    }

    void Reset()
    {
        hasPending = false;
        previousValue = currentValue;
        lastChangeTime = millis();
    }

    bool Update(bool newValue)
    {
        unsigned long now = millis();

        if (!hasPending || newValue != pendingValue)
        {
            pendingValue = newValue;
            hasPending = true;
            lastChangeTime = now;
            return false;
        }

        if ((now - lastChangeTime) < thresholdMs)
            return false;

        hasPending = false;
        if (currentValue == pendingValue)
            return false;

        previousValue = currentValue;
        currentValue = pendingValue;
        return true;
    }

    const bool &Get() const
    {
        return currentValue;
    }
    const bool &Previous() const
    {
        return previousValue;
    }
};
