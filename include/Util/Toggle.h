#ifndef TOGGLE_H
#define TOGGLE_H

namespace Util
{
    class Toggle 
    {
    private:
        bool cur;

    public:
        Toggle();

        void Swap();
        void EnsureTrue();
        void EnsureFalse();
        bool GetState() const;
    };
}

#endif