#pragma once

#include "Task.hpp"
#include <Arduino.h>
#include <stdlib.h>

namespace Tasks
{
    class HeapAllocTask : public Task
    {
    public:
        HeapAllocTask(size_t allocSize = 1024) : allocSize(allocSize)
        {
        }

        void Tick() override
        {
            CoroBegin();

            while (true)
            {
                ptr = malloc(allocSize);
                if (ptr)
                {
                    memset(ptr, 0xAA, allocSize);
                    allocated.push_back(ptr);
                }

                CoroWait(1000);
            }

            CoroEnd();
        }

        ~HeapAllocTask()
        {
            for (void *ptr : allocated)
                free(ptr);
            allocated.clear();
        }

    private:
        void *ptr;
        size_t allocSize;
        std::vector<void *> allocated;
    };
}