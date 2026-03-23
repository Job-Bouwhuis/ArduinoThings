#pragma once
#include "Util/Includes.h"

namespace Games
{
    class Game2 : public States::State
    {
    public:
        Tasks::Task *demoTask;

        Game2(States::StateMachine *sm) : State(sm, "game2") {

        }

        void OnEnter() override
        {
            lcd->Backlight(true);

            demoTask = new KatakanaDemoTask();
            tasks.AddTask(demoTask);
        }

        void Tick() override
        {

        }

        void OnExit() override
        {
            lcd->Clear();
        }

    private:
        class KatakanaDemoTask : public Tasks::Task
        {
        private:
            const char *messages[15] = {
                // basic
                "アイウエオ      ",
                "カキクケコ      ",

                // dakuten
                "ガギグゲゴ      ",
                "ザジズゼゾ      ",

                // handakuten
                "パピプペポ      ",
                "バビブベボ      ",

                // small chars
                "ャュョッ        ",
                "ァィゥェォ      ",

                // words
                "コンニチハ      ",
                "ゲーム          ",
                "デバッグ        ",
                "プログラム      ",

                // spicy edge cases
                "ヴァヴィヴォ    ",
                "ガッツポーズ    ",
                "ーーー          ",
            };

            int index = 0;

            const char *top;
            const char *bottom;

        public:
            void Tick() override
            {
                CoroBegin();

                while (1)
                {
                    top = messages[index % (sizeof(messages) / sizeof(messages[0]))];
                    bottom = messages[(index + 1) % (sizeof(messages) / sizeof(messages[0]))];

                    lcd->Write("#str\n#str", top, bottom);

                    CoroWait(1200);

                    index++;
                }

                CoroEnd();
            }
        };
    };
}
