#pragma once
#include "Util/Includes.h"

namespace States
{
    class MainMenu : public State
    {
    private:
        Tasks::Task *lcdTask;
        Tasks::Task *wadaTask;

        class MainMenuLCD : public Tasks::Task
        {
        private:
            const char *messages[7] = {
                "WinterRose Game!",
                "Use wada BTN 1-4",
                "BTN 1: Game1    ",
                "BTN 2: Game2    ",
                "BTN 3: Game3    ",
                "BTN 4: Game4    ",
                "                "};
            int index = 0;
            const char *top;
            const char *bottom;

        public:
            void Tick() override
            {
                CoroBegin();

                while (1)
                {
                    top = messages[index % 7];
                    bottom = messages[(index + 1) % 7];
                    lcd->Write("#str\n#str", top, bottom);

                    CoroWait(1000);

                    index = (index + 1) % 7;
                }

                CoroEnd();
            }
        };

        class WadaAnim : public Tasks::Task
        {
        private:
            byte segPart = 1;
            eight curLed = 0;

        public:
            void Tick() override
            {
                CoroBegin();
                while (1)
                {
                    for (byte i = 0; i < 8; ++i)
                    {
                        wada->WriteSeg(i, segPart);
                    }

                    wada->SetLed(curLed - 1, false);
                    wada->SetLed(curLed, true);

                    CoroWait(50);

                    curLed++;

                    segPart <<= 1;
                    if (segPart == 0b00100000)
                        segPart = 1;
                }
                CoroEnd();
            }
        };

        class TransitionToGameTask : public Tasks::Task
        {
        private:
            MainMenu *parent;
            const String targetName;
            byte i;

        public:
            TransitionToGameTask(MainMenu *parent, const String target)
                : parent(parent), targetName(target)
            {
            }

            void Tick() override
            {
                CoroBegin();

                lcd->Write<true>("Starting #str                               ", targetName);

                for (i = 0; i < 8; ++i)
                {
                    wada->WriteSeg(i, 0);
                    wada->SetLed(i, false);
                }
                i = 0;
                CoroWait(150);

                for (; i < 8; ++i)
                {
                    wada->SetLed(i, true);
                    CoroWait(20);
                }
                i = 0;
                CoroWait(700);

                lcd->Write("                \n                ");

                CoroWait(100);

                for (i = 0; i < 8; ++i)
                    wada->SetLed(i, false);
                i = 0;
                CoroWait(50);

                parent->TransitionTo(targetName);

                CoroEnd();
            }
        };

    public:
        MainMenu(StateMachine *sm) : State(sm, "mainmenu")
        {
        }

        void OnEnter() override
        {
            lcd->Backlight(true);

            wadaButton1->OnClick.Add([this](Components::Button *btn)
                                     {
                                         lcdTask->Stop();
                                         wadaTask->Stop();

                                         tasks.AddTask(new TransitionToGameTask(this, "game1"));
                                         //
                                     });

            wadaButton2->OnClick.Add([this](Components::Button *btn)
                                     {
                                         lcdTask->Stop();
                                         wadaTask->Stop();

                                         tasks.AddTask(new TransitionToGameTask(this, "game2"));
                                         //
                                     });

            wadaButton3->OnClick.Add([this](Components::Button *btn)
                                     {
                                         lcdTask->Stop();
                                         wadaTask->Stop();

                                         tasks.AddTask(new TransitionToGameTask(this, "game3"));
                                         //
                                     });

            wadaButton4->OnClick.Add([this](Components::Button *btn)
                                     {
                                         lcdTask->Stop();
                                         wadaTask->Stop();

                                         tasks.AddTask(new TransitionToGameTask(this, "game4"));
                                         //
                                     });

            lcdTask = new MainMenuLCD();
            tasks.AddTask(lcdTask);

            wadaTask = new WadaAnim();
            tasks.AddTask(wadaTask);
        }

        void OnExit() override
        {
            lcdTask->Stop();
            wadaTask->Stop();
        }
    };
}