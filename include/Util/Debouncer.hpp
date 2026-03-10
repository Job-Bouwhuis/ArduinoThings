#pragma once

template <typename T>
class Debouncer
{
private:
    T currentValue;
    T previousValue;
    T pendingValue;
    bool hasPending;
    unsigned long lastChangeTime;
    unsigned long thresholdMs;

public:
    Debouncer(unsigned long debounceMs = 200)
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

    bool Update(T newValue)
    {
        unsigned long now = millis();

        // New candidate value — restart the debounce window
        if (!hasPending || newValue != pendingValue)
        {
            pendingValue = newValue;
            hasPending = true;
            lastChangeTime = now;
            return false;
        }

        // Candidate matches, but hasn't settled yet
        if ((now - lastChangeTime) < thresholdMs)
            return false;

        // Settled — only commit if it actually changed
        hasPending = false;
        if (currentValue == pendingValue)
            return false;

        previousValue = currentValue;
        currentValue = pendingValue;
        return true;
    }

    const T &Get() const { return currentValue; }
    const T &Previous() const { return previousValue; }
};
