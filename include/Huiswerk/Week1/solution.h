#pragma once
#include "Components/Led.h"
#include "Components/Button.h"
#include "Util/Console.h"

namespace Huiswerk::Week1::Opdracht1
{
    class Solution
    {
    public:
        Solution() = default;
        void Tick();
    };
}

namespace Huiswerk::Week1::Opdracht2
{
    class Solution
    {
    public:
        Solution();

        void Tick();

    private:
        Components::Button btn;
        Components::Led led;
    };

}

namespace Huiswerk::Week1::Opdracht3
{
    class Solution
    {
    public:
        Solution();

        void Tick();

    private:
        Components::Button btn;
        int holdStart;
    };
}