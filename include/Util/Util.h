#include <Arduino.h>

#define PRINT_BINARY(val)                 \
    do {                                  \
        for (int i = 7; i >= 0; i--)     \
            Serial.print(((val) >> i) & 1); \
        Serial.println();                 \
    } while(0)
