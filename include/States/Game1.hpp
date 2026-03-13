#pragma once
#include "Util/Includes.h"

namespace Games
{
    class Game1 : public States::State
    {
    public:
        Game1(States::StateMachine *sm) : State(sm, "game1")
        {
        }

        void OnEnter() override
        {
            lcd->Write("This is Game 1!\nHave Fun <heart>");
        }
    };
}