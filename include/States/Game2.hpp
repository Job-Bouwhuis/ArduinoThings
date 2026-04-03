#pragma once

#include "Util/Includes.h"
#include "Tasks/ExplainTask.hpp"
#include "States/SessionResult.hpp"
#include <random>
#include <chrono>

namespace Games
{
    class Game2 : public States::State
    {
    public:
        Game2(States::StateMachine *sm)
            : State(sm, "game2"), generator(std::mt19937(seed))
        {
        }

        void OnEnter() override
        {
            ResetGame();
            SetupButtons();
            lcd->Backlight(true);

            tasks.AddTask(new Tasks::ExplainTask(
                {
                    "Read katakana\nPick romaji",
                    "Btn1/2 select\nBtn3 confirm",
                },
                1800,
                [this]()
                {
                    isPlaying = true;
                    lastSecondTick = millis();
                    RenderQuestion();
                }));
        }

        void Tick() override
        {
            if (!isPlaying)
                return;

            if (gameOver)
            {
                if (!gameOverShown)
                {
                    lcd->Clear();
                    if (didWin)
                    {
                        lcd->Write("You won!\nScore: #num", score);
                        buzzer->PlayEffectTone(1600, 260);
                        buzzer->PlayEffectTone(1900, 340);
                    }
                    else
                    {
                        lcd->Write("You lost!\nScore: #num", score);
                        buzzer->PlayEffectTone(900, 280);
                        buzzer->PlayEffectTone(700, 340);
                    }
                    gameOverShown = true;
                }

                if (millis() - gameOverStart >= GAME_OVER_DELAY)
                {
                    lcd->Clear();
                    States::PrepareGameResult("game2", didWin, score);
                    stateMachine.RequestTransition("gameover");
                }
                return;
            }

            unsigned long now = millis();
            if (now - lastSecondTick >= 1000)
            {
                lastSecondTick += 1000;
                if (secondsLeft > 0)
                    secondsLeft--;

                if (secondsLeft == 0)
                    HandleWrongAnswer();

                if (!gameOver)
                    RenderQuestion();
            }
        }

        void OnExit() override
        {
            lcd->Clear();
            wada->WriteByteAsBits(0);
        }

    private:
        static constexpr int TARGET_CORRECT = 10;
        static constexpr int MAX_MISTAKES = 3;
        static constexpr int TIMER_LONG = 60;
        static constexpr int TIMER_SHORT = 30;
        static constexpr unsigned long GAME_OVER_DELAY = 2200;

        struct KanaEntry
        {
            uint8_t glyph;
            const char *romaji;
        };

        static constexpr KanaEntry kKanaList[] = {
            {0b10110001, "a"},
            {0b10110010, "i"},
            {0b10110011, "u"},
            {0b10110100, "e"},
            {0b10110101, "o"},
            {0b10110110, "ka"},
            {0b10110111, "ki"},
            {0b10111000, "ku"},
            {0b10111001, "ke"},
            {0b10111010, "ko"},
            {0b10111011, "sa"},
            {0b10111100, "shi"},
            {0b10111101, "su"},
            {0b10111110, "se"},
            {0b10111111, "so"},
            {0b11000000, "ta"},
            {0b11000001, "chi"},
            {0b11000010, "tsu"},
            {0b11000011, "te"},
            {0b11000100, "to"},
            {0b11000101, "na"},
            {0b11000110, "ni"},
            {0b11000111, "nu"},
            {0b11001000, "ne"},
            {0b11001001, "no"},
            {0b11001010, "ha"},
            {0b11001011, "hi"},
            {0b11001100, "fu"},
            {0b11001101, "he"},
            {0b11001110, "ho"},
            {0b11001111, "ma"},
            {0b11010000, "mi"},
            {0b11010001, "mu"},
            {0b11010010, "me"},
            {0b11010011, "mo"},
            {0b11010100, "ya"},
            {0b11010101, "yu"},
            {0b11010110, "yo"},
            {0b11010111, "ra"},
            {0b11011000, "ri"},
            {0b11011001, "ru"},
            {0b11011010, "re"},
            {0b11011011, "ro"},
            {0b11011100, "wa"},
            {0b11011101, "wo"},
            {0b11011110, "n"},
        };

        void ResetGame()
        {
            isPlaying = false;
            gameOver = false;
            gameOverShown = false;
            didWin = false;

            score = 0;
            correctCount = 0;
            mistakes = 0;

            selectedOption = 0;
            correctOption = 0;
            secondsLeft = TIMER_LONG;
            currentTimerLimit = TIMER_LONG;

            currentKanaIndex = 0;
            optionIndices[0] = 0;
            optionIndices[1] = 0;

            lastSecondTick = millis();
            gameOverStart = millis();

            wada->WriteByteAsBits(0);
            GenerateQuestion();
        }

        void SetupButtons()
        {
            wadaButton1->OnClick.Add([this](Components::Button *btn)
                                     {
                                         if (!isPlaying || gameOver)
                                             return;

                                         selectedOption = 0;
                                         buzzer->PlayEffectTone(1200, 60);
                                         RenderQuestion(); });

            wadaButton2->OnClick.Add([this](Components::Button *btn)
                                     {
                                         if (!isPlaying || gameOver)
                                             return;

                                         selectedOption = 1;
                                         buzzer->PlayEffectTone(1200, 60);
                                         RenderQuestion(); });

            wadaButton3->OnClick.Add([this](Components::Button *btn)
                                     {
                                         if (!isPlaying || gameOver)
                                             return;

                                         if (selectedOption == correctOption)
                                         {
                                             HandleCorrectAnswer();
                                         }
                                         else
                                         {
                                             HandleWrongAnswer();
                                         }

                                         if (!gameOver)
                                             RenderQuestion(); });
        }

        void GenerateQuestion()
        {
            const int kanaCount = static_cast<int>(sizeof(kKanaList) / sizeof(kKanaList[0]));

            currentKanaIndex = kanaDist(generator) % kanaCount;

            int wrongIdx = currentKanaIndex;
            while (wrongIdx == currentKanaIndex)
                wrongIdx = kanaDist(generator) % kanaCount;

            correctOption = directionDist(generator);
            if (correctOption == 0)
            {
                optionIndices[0] = currentKanaIndex;
                optionIndices[1] = wrongIdx;
            }
            else
            {
                optionIndices[0] = wrongIdx;
                optionIndices[1] = currentKanaIndex;
            }

            selectedOption = 0;
            currentTimerLimit = (correctCount < 5) ? TIMER_LONG : TIMER_SHORT;
            secondsLeft = currentTimerLimit;
            lastSecondTick = millis();
        }

        int ComputeScoreGain() const
        {
            if (currentTimerLimit == TIMER_SHORT)
                return secondsLeft + 30;
            return secondsLeft;
        }

        void HandleCorrectAnswer()
        {
            score += ComputeScoreGain();
            correctCount++;
            wada->Write(score);

            buzzer->PlayEffectTone(1700, 90);
            buzzer->PlayEffectTone(2100, 110);

            if (correctCount >= TARGET_CORRECT)
            {
                didWin = true;
                gameOver = true;
                gameOverStart = millis();
                return;
            }

            GenerateQuestion();
        }

        void HandleWrongAnswer()
        {
            mistakes++;
            buzzer->PlayEffectTone(950, 140);

            if (mistakes >= MAX_MISTAKES)
            {
                didWin = false;
                gameOver = true;
                gameOverStart = millis();
                return;
            }

            GenerateQuestion();
        }

        void RenderQuestion()
        {
            int livesLeft = MAX_MISTAKES - mistakes;
            if (livesLeft < 0)
                livesLeft = 0;

            String top = String("#unique\\#T:") + String(secondsLeft) + " C:" + String(correctCount) + " ";
            for (int i = 0; i < MAX_MISTAKES; ++i)
            {
                top += (i < livesLeft) ? "<heartfilled>" : "<heart>";
            }

            String leftMarker = (selectedOption == 0) ? ">" : " ";
            String rightMarker = (selectedOption == 1) ? ">" : " ";

            String leftOption = String(kKanaList[optionIndices[0]].romaji);
            if (leftOption.length() > 3)
                leftOption = leftOption.substring(0, 3);
            while (leftOption.length() < 3)
                leftOption += " ";

            String rightOption = String(kKanaList[optionIndices[1]].romaji);
            if (rightOption.length() > 3)
                rightOption = rightOption.substring(0, 3);
            while (rightOption.length() < 3)
                rightOption += " ";

            String bottom = leftMarker + leftOption + " " + rightMarker + rightOption + "confirm";

            lcd->Write(top + "\n#str", bottom.c_str());

            lcd->SetCursor(15, 0);
            char kanaBuffer[2] = {static_cast<char>(kKanaList[currentKanaIndex].glyph), '\0'};
            lcd->Write("#str", kanaBuffer);
        }

        bool isPlaying = false;
        bool gameOver = false;
        bool gameOverShown = false;
        bool didWin = false;

        int score = 0;
        int correctCount = 0;
        int mistakes = 0;

        int currentKanaIndex = 0;
        int optionIndices[2] = {0, 0};
        int selectedOption = 0;
        int correctOption = 0;

        int currentTimerLimit = TIMER_LONG;
        int secondsLeft = TIMER_LONG;

        unsigned long lastSecondTick = 0;
        unsigned long gameOverStart = 0;

        uint64_t seed =
            std::chrono::high_resolution_clock::now().time_since_epoch().count() ^
            std::random_device{}();

        std::mt19937 generator;
        std::uniform_int_distribution<int> kanaDist{0, 1024};
        std::uniform_int_distribution<int> directionDist{0, 1};
    };
}
