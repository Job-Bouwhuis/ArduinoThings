#pragma once
#include "Util/Includes.h"

namespace Games
{
    class Game4 : public States::State
    {
    public:
        Game4(States::StateMachine *sm) : State(sm, "game3") {}

        void OnEnter() override
        {
        }

        void Tick() override
        {
        }

        void OnExit() override
        {
        }
    };
}