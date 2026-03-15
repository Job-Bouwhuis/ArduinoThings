#pragma once
#include "Util/Includes.h"
#include <random>
#include <chrono>

#define MAX_ROUNDS 4

namespace Games
{
    class Game1 : public States::State
    {
    public:
        Game1(States::StateMachine *sm) : State(sm, "game1"), generator(std::mt19937(seed))
        {
        }

        void OnEnter() override
        {
            Reset();
            tasks.AddTask(new ExplainTask(this));
        }

    private:
        void Reset()
        {
            round = 1;
            answer = 0;
            targetNum = 0;
            lives = 3;
            totalLives = 3;
            score = 0;
            SetSeconds();
            GenerateRandomNumber();
            SetupButtons();
            wada->WriteByteAsBits(0);
        }

        void SetupButtons()
        {
            wadaButton1->OnClick.Add([this](Components::Button *btn)
                                     {
                                         secondsLeft = 1;
                                         //
                                     });

            wadaButton2->OnClick.Add([this](Components::Button *btn)
                                     {
                                         answer |= 0b1000000;
                                         wada->WriteByteAsBits(answer);
                                         //
                                     });

            wadaButton3->OnClick.Add([this](Components::Button *btn)
                                     {
                                         answer |= 0b0100000;
                                         wada->WriteByteAsBits(answer);
                                         //
                                     });

            wadaButton4->OnClick.Add([this](Components::Button *btn)
                                     {
                                         answer |= 0b0010000;
                                         wada->WriteByteAsBits(answer);
                                         //
                                     });

            wadaButton5->OnClick.Add([this](Components::Button *btn)
                                     {
                                         answer |= 0b0001000;
                                         wada->WriteByteAsBits(answer);
                                         //
                                     });

            wadaButton6->OnClick.Add([this](Components::Button *btn)
                                     {
                                         answer |= 0b0000100;
                                         wada->WriteByteAsBits(answer);
                                         //
                                     });

            wadaButton7->OnClick.Add([this](Components::Button *btn)
                                     {
                                         answer |= 0b0000010;
                                         wada->WriteByteAsBits(answer);
                                         //
                                     });
            wadaButton8->OnClick.Add([this](Components::Button *btn)
                                     {
                                         answer |= 0b0000001;
                                         wada->WriteByteAsBits(answer);
                                         //
                                     });
        }

        class ExplainTask : public Tasks::Task
        {
        public:
            ExplainTask(Game1 *game) : game(game) {}

            void Tick() override
            {
                CoroBegin();
                buzzer->PlayEffectTone(800, 500);
                lcd->Write("Use WADA Buttons\nbtn 1 to submit");
                CoroWait(2500);
                buzzer->PlayEffectTone(1000, 200);
                buzzer->PlayEffectTone(1200, 350);
                CoroEnd();
                tasks.AddTask(new RoundTask(game));
            }

        private:
            Game1 *game;
        };

        class RoundTask : public Tasks::Task
        {
        public:
            RoundTask(Game1 *game) : game(game)
            {
            }

            void Tick() override
            {
                CoroBegin();

                while (game->round < 5)
                {
                    if (game->lives == 0)
                        break;

                    lcd->Write("#cury#unique#num#rightalign\\#tries: #num/#num", 1, game->targetNum, game->lives, game->totalLives);

                    while (game->secondsLeft > 0)
                    {
                        lcd->Write("#unique\\#Convert:#rightalign#num\\#s", game->secondsLeft);
                        CoroWait(1000);
                        game->secondsLeft--;
                    }
                    game->SetSeconds();
                    if (game->validate())
                    {
                        lcd->Clear();
                        lcd->Write("Well done!");
                        CoroWait(1500);
                    }
                    else
                    {
                        lcd->Clear();
                        lcd->Write<true>("Unfortunate, Thats not correct");
                        CoroWait(1500);
                    }

                    game->round++;
                }

                lcd->Clear();

                if (game->lives > 0)
                {
                    // buzzer->PlayMusicTrack(game->happy_frequencies, game->happy_durations, 7);
                    lcd->Write("You won!");
                    lcd->Write("#cury\\#Score: #num", 1, game->score);
                }
                else
                {
                    // buzzer->PlayMusicTrack(game->sad_frequencies, game->sad_durations, 6);
                    lcd->Write("You lost!\nAnswer #bin", game->targetNum);
                }

                CoroWait(3000);
                lcd->Clear();
                lcd->Write("Replay: Btn1\nExit: Btn2");

                Serial.println("yadaaaa");

                wada->ClearButtonEvents();

                wadaButton1->OnClick.Add([this](Components::Button *btn)
                                         {
                                             Serial.println("i have been pressed!");
                                             Stop();
                                             stateMachine.RequestTransition("game1");
                                             //
                                         });

                wadaButton2->OnClick.Add([this](Components::Button *btn)
                                         {
                                             Serial.println("i also have been pressed!");
                                             Stop();
                                             stateMachine.RequestTransition("mainmenu");
                                             //
                                         });

                CoroYieldOnce();
                while (1)
                    CoroWait(100);
                CoroEnd();
            }

        private:
            Game1 *game;
        };

        void SetSeconds()
        {
            if (round < 3)
                secondsLeft = 30;
            else
                secondsLeft = 15;
        }
        void GenerateRandomNumber()
        {
            targetNum = distribution(generator);
        }

        bool validate()
        {
            wada->WriteByteAsBits(0);

            if (answer == targetNum)
            {
                answer = 0;
                const uint16_t happy_frequencies[] = {
                    NOTE_C4, NOTE_E4, NOTE_G4, NOTE_C5, NOTE_E5, NOTE_G5, NOTE_C6};

                const uint16_t happy_durations[] = {
                    100, 100, 100, 150, 100, 100, 200};

                buzzer->PlayMusicTrack(happy_frequencies, happy_durations, 7);
                GenerateRandomNumber();

                score += (targetNum + 5) / 10 + secondsLeft;

                SetSeconds();
                GenerateRandomNumber();

                return true;
            }
            else
            {
                answer = 0;
                const uint16_t sad_frequencies[] = {
                    NOTE_A4, NOTE_F4, NOTE_D4, NOTE_B3, NOTE_C4, NOTE_D4};

                const uint16_t sad_durations[] = {
                    150, 120, 150, 100, 150, 200};

                lives--;
                buzzer->PlayMusicTrack(sad_frequencies, sad_durations, 6);
                SetSeconds();
                return false;
            }
        }

        int round = 1;
        byte answer = 0;
        byte targetNum = 0;
        byte lives = 3;
        byte totalLives = 3;
        byte secondsLeft = 0;
        int score = 0;

        uint64_t seed =
            std::chrono::high_resolution_clock::now().time_since_epoch().count() ^
            std::random_device{}();
        std::mt19937 generator;
        std::uniform_int_distribution<int> distribution{1, 63};
    };
}