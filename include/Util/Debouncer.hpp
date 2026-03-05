#pragma once

template <typename T>
class Debouncer
{
private:
    T currentValue;
    T previousValue;
    int counter;
    int threshold;

public:
    Debouncer(int debounceThreshold = 5)
        : currentValue(), previousValue(), counter(0), threshold(debounceThreshold) {}

    void Reset()
    {
        counter = 0;
        previousValue = currentValue;
    }

    bool Update(T newValue)
    {
        if (newValue != currentValue)
        {
            counter++;
            if (counter >= threshold)
            {
                previousValue = currentValue;
                currentValue = newValue;
                counter = 0;
                return true;
            }
        }
        else
        {
            counter = 0;
        }
        return false;
    }

    const T &Get() const { return currentValue; }
    const T &Previous() const { return previousValue; }
};