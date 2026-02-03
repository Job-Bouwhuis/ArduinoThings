#include <cstddef>
#include <Arduino.h>

// Linker symbols for RAM sections
extern char _sdata;   // start of initialized data
extern char _edata;   // end of initialized data
extern char _sbss;    // start of BSS
extern char _ebss;    // end of BSS
extern char _estack;  // top of stack

namespace Util
{
    class MemoryWatcher
    {
    private:
        const int TOTAL_MEMORY = 40960;

    public:
        void PrintMemoryToSerial()
        {
            char stackVar;

            // RAM boundaries
            char* ramStart = &_ebss;        // free RAM starts after BSS
            char* ramEnd   = &_estack;      // top of RAM

            size_t totalRam = ramEnd - ramStart;          // total available RAM after static vars
            size_t usedStack = ramEnd - &stackVar;       // stack used so far
            size_t freeRam   = &stackVar - ramStart;     // approx free RAM

            Serial.print("Mem:");

            Serial.print(" | Total RAM: "); Serial.print(totalRam);
            Serial.print(" | Free RAM: "); Serial.print(freeRam);
            Serial.print(" | Stack used: "); Serial.println(usedStack);
        }
    };
}