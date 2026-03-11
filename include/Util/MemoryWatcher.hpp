#include <cstddef>
#include <Arduino.h>
#include <unistd.h>

extern char _sdata;
extern char _edata;
extern char _sbss;
extern char _ebss;
extern char _estack;

extern "C" char __heap_start__;

namespace Util
{
    class MemoryWatcher
    {
    private:
        const int TOTAL_MEMORY = 40960;

    public:
        size_t freeMemory()
        {
            char stackVar;
            char *heapTop = reinterpret_cast<char *>(sbrk(0));
            return &stackVar - heapTop - 20000; // 20k subtracted because thats where the nucleo freezes, for some reason. so ill just assume thats where the memory ends
        }
    };
}