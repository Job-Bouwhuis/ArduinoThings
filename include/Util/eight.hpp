#pragma once
#include <arduino.h>

/// @brief a number that only counts to eight
struct eight
{
    unsigned value : 3;

    eight() : value(0) {}

    eight(int v) : value(v & 7) {}
    eight(byte v) : value(v & 7) {}

    void decrement() { value = (value == 0) ? 7 : value - 1; }
    void increment() { value = (value == 7) ? 0 : value + 1; }

    eight &operator++()
    {
        increment();
        return *this;
    }

    eight operator++(int)
    {
        eight temp = *this;
        increment();
        return temp;
    }

    eight &operator--()
    {
        decrement();
        return *this;
    }

    eight operator--(int)
    {
        eight temp = *this;
        decrement();
        return temp;
    }

    eight operator+(int n) const
    {
        int res = (value + n) % 8;
        if (res < 0)
            res += 8;
        return eight(res);
    }

    eight operator-(int n) const
    {
        return *this + (-n);
    }

    operator byte() const { return value; }
};