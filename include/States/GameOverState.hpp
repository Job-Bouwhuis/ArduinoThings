#pragma once

#include "Util/Includes.h"
#include "States/SessionResult.hpp"

namespace States
{
    class GameOverState : public State
    {
    public:
        GameOverState(StateMachine *sm) : State(sm, "gameover") {}

        void OnEnter() override
        {
            Render();

            wadaButton1->OnClick.Add([this](Components::Button *btn)
                                     {
                                         SessionResultData &data = GetSessionResultData();

                                         if (data.globalSessionWin)
                                         {
                                             ResetSessionWins();
                                             TransitionTo("mainmenu");
                                             return;
                                         }

                                         TransitionTo(data.restartState); });

            wadaButton2->OnClick.Add([this](Components::Button *btn)
                                     {
                                         SessionResultData &data = GetSessionResultData();

                                         if (data.globalSessionWin)
                                             ResetSessionWins();

                                         TransitionTo("mainmenu"); });
        }

        void OnExit() override
        {
            lcd->Clear();
            wada->WriteByteAsBits(0);
        }

    private:
        void Render()
        {
            SessionResultData &data = GetSessionResultData();
            lcd->Clear();

            if (data.timeUp)
            {
                lcd->Write("Time up!        \n1 Retry 2 Menu  ");
                return;
            }

            if (data.globalSessionWin)
            {
                lcd->Write("All 4 won!      \n1 New   2 Menu  ");
                return;
            }

            if (data.lastGameWon)
            {
                lcd->Write("You won!        \n1 Replay 2 Menu ");
                return;
            }

            lcd->Write("Game over!      \n1 Retry 2 Menu  ");
        }
    };
}
