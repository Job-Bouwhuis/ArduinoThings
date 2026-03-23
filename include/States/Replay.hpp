#pragma once
#include "Util/Includes.h"

namespace States
{
    class Replay : public State
    {
    public:
        Replay(StateMachine *sm) : State(sm, "replay") {}
        String replayState;

        void SetReplayState(String state)
        {
            replayState = state;
        }

        void OnEnter() override
        {
            lcd->Write("Replay: Btn1\nExit: Btn2");

            

            wadaButton1->OnClick.Add([&](Components::Button *btn)
                                     {
                                         Serial.println("i have been pressed!");
                                         this->machine->RequestTransition(replayState);
                                         //
                                     });

            wadaButton2->OnClick.Add([this](Components::Button *btn)
                                     {
                                         Serial.println("i also have been pressed!");
                                         this->machine->RequestTransition("mainmenu");
                                         //
                                     });
        }
    };
}
