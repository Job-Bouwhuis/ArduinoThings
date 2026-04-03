#pragma once

#include "Util/Includes.h"
#include "Tasks/ExplainTask.hpp"
#include "States/SessionResult.hpp"
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
            tasks.AddTask(new Tasks::ExplainTask(
                {
                    "Use WADA Buttons\nbtn 1 to submit",
                },
                2500,
                [this]()
                {
                    tasks.AddTask(new RoundTask(this));
                }));
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
                                         if (secondsLeft > 1)
                                             secondsLeft = 1;
                                         buzzer->PlayEffectTone(1400, 70); });

            wadaButton2->OnClick.Add([this](Components::Button *btn)
                                     { ToggleAnswerBit(0b1000000); });

            wadaButton3->OnClick.Add([this](Components::Button *btn)
                                     { ToggleAnswerBit(0b0100000); });

            wadaButton4->OnClick.Add([this](Components::Button *btn)
                                     { ToggleAnswerBit(0b0010000); });

            wadaButton5->OnClick.Add([this](Components::Button *btn)
                                     { ToggleAnswerBit(0b0001000); });

            wadaButton6->OnClick.Add([this](Components::Button *btn)
                                     { ToggleAnswerBit(0b0000100); });

            wadaButton7->OnClick.Add([this](Components::Button *btn)
                                     { ToggleAnswerBit(0b0000010); });

            wadaButton8->OnClick.Add([this](Components::Button *btn)
                                     { ToggleAnswerBit(0b0000001); });
        }

        void ToggleAnswerBit(byte mask)
        {
            answer ^= mask;
            wada->WriteByteAsBits(answer);
            const bool isEnabled = (answer & mask) != 0;
            buzzer->PlayEffectTone(isEnabled ? 1800 : 1000, 45);
        }

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

                    buzzer->PlayEffectTone(1500, 60);

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
                    buzzer->PlayEffectTone(1800, 160);
                    buzzer->PlayEffectTone(2100, 220);
                    lcd->Write("You won!");
                    lcd->Write("#cury\\#Score: #num", 1, game->score);
                }
                else
                {
                    buzzer->PlayEffectTone(900, 180);
                    buzzer->PlayEffectTone(700, 260);
                    lcd->Write("You lost!\nAnswer #bin", game->targetNum);
                }

                CoroWait(3000);
                lcd->Clear();
                CoroEnd();

                States::PrepareGameResult("game1", game->lives > 0, game->score);
                stateMachine.RequestTransition("gameover");
            }

        private:
            Game1 *game;
        };

        void SetSeconds()
        {
            if (round < 3)
                secondsLeft = 60;
            else
                secondsLeft = 30;
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
