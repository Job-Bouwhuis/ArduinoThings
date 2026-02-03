#include "Util/Toggle.h"

namespace Util
{
    Toggle::Toggle() : cur(false)
    {}

    void Toggle::Swap() {
        cur = !cur;
    }

    void Toggle::EnsureTrue() {
        if (!cur) cur = true;
    }

    void Toggle::EnsureFalse() {
        if (cur) cur = false;
    }

    bool Toggle::GetState() const {
        return cur;
    }
}