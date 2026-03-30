#pragma once
#include "Util/Includes.h"
#include "Tasks/ExplainTask.hpp"
#include <vector>
#include <random>
#include <chrono>

namespace Games
{
    class Game4 : public States::State
    {
    public:
        Game4(States::StateMachine *sm) : State(sm, "game4") {}

        void OnEnter() override
        {
            ResetGame();
            SetupButtons();

            tasks.AddTask(new Tasks::ExplainTask(
                {
                    "Memorize LED order\nRepeat with btn1-8",
                    "5 lives\nPattern gets longer",
                },
                1800,
                [this]()
                {
                    isPlaying = true;
                    BeginRound(true);
                }));
        }

        void Tick() override
        {
            if (!isPlaying)
                return;

            unsigned long now = millis();

            if (gameOver)
            {
                if (!gameOverShown)
                {
                    lcd->Clear();
                    if (didWin)
                    {
                        lcd->Write("You won!\nScore: #num", score);
                        buzzer->PlayEffectTone(1700, 250);
                        buzzer->PlayEffectTone(2100, 360);
                    }
                    else
                    {
                        lcd->Write("Game over!\nScore: #num", score);
                        buzzer->PlayEffectTone(850, 280);
                        buzzer->PlayEffectTone(700, 340);
                    }
                    gameOverShown = true;
                }

                if (now - gameOverStart >= GAME_OVER_DELAY)
                {
                    lcd->Clear();
                    replayState->SetReplayState("game4");
                    stateMachine.RequestTransition("replay");
                }
                return;
            }

            if (flashLedIndex >= 0 && now >= flashLedUntil)
            {
                wada->SetLed(flashLedIndex, false);
                flashLedIndex = -1;
            }

            if (phase == Phase::Showing)
            {
                UpdatePatternPlayback(now);
                return;
            }

            if (phase == Phase::RoundPause && now >= phaseUntil)
                if (roundWasSuccess)x
                    BeginRound(true);
                else
                    BeginRound(false);
        }

        void OnExit() override
        {
            ClearLeds();
            wada->WriteByteAsBits(0);
            lcd->Clear();
        }

    private:
        enum class Phase
        {
            Idle,
            Showing,
            AwaitingInput,
            RoundPause,
        };

        static constexpr int MAX_LIVES = 5;
        static constexpr int TARGET_SEQUENCE_LENGTH = 10;
        static constexpr unsigned long STEP_ON_MS = 330;
        static constexpr unsigned long STEP_OFF_MS = 170;
        static constexpr unsigned long ROUND_PAUSE_MS = 900;
        static constexpr unsigned long GAME_OVER_DELAY = 2200;

        void ResetGame()
        {
            isPlaying = false;
            gameOver = false;
            gameOverShown = false;
            didWin = false;

            lives = MAX_LIVES;
            score = 0;
            inputIndex = 0;

            sequence.clear();

            phase = Phase::Idle;
            showIndex = 0;
            showLedIsOn = false;
            phaseUntil = millis();

            flashLedIndex = -1;
            flashLedUntil = 0;

            gameOverStart = millis();

            wada->Write(0UL);
            ClearLeds();
            Render("Ready");
        }

        void SetupButtons()
        {
            wadaButton1->OnClick.Add([this](Components::Button *btn)
                                     { HandleInput(0); });
            wadaButton2->OnClick.Add([this](Components::Button *btn)
                                     { HandleInput(1); });
            wadaButton3->OnClick.Add([this](Components::Button *btn)
                                     { HandleInput(2); });
            wadaButton4->OnClick.Add([this](Components::Button *btn)
                                     { HandleInput(3); });
            wadaButton5->OnClick.Add([this](Components::Button *btn)
                                     { HandleInput(4); });
            wadaButton6->OnClick.Add([this](Components::Button *btn)
                                     { HandleInput(5); });
            wadaButton7->OnClick.Add([this](Components::Button *btn)
                                     { HandleInput(6); });
            wadaButton8->OnClick.Add([this](Components::Button *btn)
                                     { HandleInput(7); });
        }

        void BeginRound(bool addNewStep)
        {
            if (addNewStep)
                sequence.push_back(buttonDist(generator));

            inputIndex = 0;
            showIndex = 0;
            showLedIsOn = false;
            phase = Phase::Showing;
            phaseUntil = millis() + 300;
            ClearLeds();
            Render("Watch");
        }

        void UpdatePatternPlayback(unsigned long now)
        {
            if (showIndex >= sequence.size())
            {
                phase = Phase::AwaitingInput;
                ClearLeds();
                Render("Repeat");
                return;
            }

            if (now < phaseUntil)
                return;

            int stepButton = sequence[showIndex];
            if (!showLedIsOn)
            {
                SetOnlyLed(stepButton);
                PlayButtonTone(stepButton, 120);
                showLedIsOn = true;
                phaseUntil = now + STEP_ON_MS;
            }
            else
            {
                ClearLeds();
                showLedIsOn = false;
                showIndex++;
                phaseUntil = now + STEP_OFF_MS;
            }
        }

        void HandleInput(int buttonIndex)
        {
            if (!isPlaying || gameOver || phase != Phase::AwaitingInput)
                return;

            PlayButtonTone(buttonIndex, 80);

            wada->SetLed(buttonIndex, true);
            flashLedIndex = buttonIndex;
            flashLedUntil = millis() + 100;

            if (buttonIndex == sequence[inputIndex])
            {
                inputIndex++;
                Render("Good");

                if (inputIndex >= sequence.size())
                {
                    score += static_cast<int>(sequence.size()) * 10;
                    wada->Write(score);

                    buzzer->PlayEffectTone(1650, 100);
                    buzzer->PlayEffectTone(2050, 120);

                    if (sequence.size() >= TARGET_SEQUENCE_LENGTH)
                    {
                        didWin = true;
                        gameOver = true;
                        gameOverStart = millis();
                        return;
                    }

                    roundWasSuccess = true;
                    phase = Phase::RoundPause;
                    phaseUntil = millis() + ROUND_PAUSE_MS;
                    Render("Next");
                }
            }
            else
            {
                lives--;
                buzzer->PlayEffectTone(900, 140);

                if (lives <= 0)
                {
                    didWin = false;
                    gameOver = true;
                    gameOverStart = millis();
                    return;
                }

                roundWasSuccess = false;
                phase = Phase::RoundPause;
                phaseUntil = millis() + ROUND_PAUSE_MS;
                Render("Miss");
            }
        }

        void PlayButtonTone(int buttonIndex, int durationMs)
        {
            static const uint16_t notes[8] = {
                NOTE_C4, NOTE_D4, NOTE_E4, NOTE_F4,
                NOTE_G4, NOTE_A4, NOTE_B4, NOTE_C5};

            if (buttonIndex < 0 || buttonIndex > 7)
                return;

            buzzer->PlayEffectTone(notes[buttonIndex], durationMs);
        }

        void SetOnlyLed(int idx)
        {
            for (int i = 0; i < 8; i++)
                wada->SetLed(i, i == idx);
        }

        void ClearLeds()
        {
            for (int i = 0; i < 8; i++)
                wada->SetLed(i, false);
        }

        void Render(const String &status)
        {
            lcd->Clear();
            int shownLives = lives;
            if (shownLives < 0)
                shownLives = 0;

            String top = String("Seq:") + String(sequence.size()) + " " + status;

            String hearts = "";
            for (int i = 0; i < MAX_LIVES; ++i)
                hearts += (i < shownLives) ? "<heartfilled>" : "<heart>";

            String bottom = hearts + " " + String(inputIndex) + "/" + String(sequence.size());
            String full = top + "\n" + bottom;
            Serial.println(full);
            lcd->Write(full);
        }

        bool isPlaying = false;
        bool gameOver = false;
        bool gameOverShown = false;
        bool didWin = false;

        int lives = MAX_LIVES;
        int score = 0;
        int inputIndex = 0;

        std::vector<int> sequence;

        Phase phase = Phase::Idle;
        bool roundWasSuccess = true;

        size_t showIndex = 0;
        bool showLedIsOn = false;
        unsigned long phaseUntil = 0;

        int flashLedIndex = -1;
        unsigned long flashLedUntil = 0;

        unsigned long gameOverStart = 0;

        uint64_t seed =
            std::chrono::high_resolution_clock::now().time_since_epoch().count() ^
            std::random_device{}();

        std::mt19937 generator{static_cast<uint32_t>(seed)};
        std::uniform_int_distribution<int> buttonDist{0, 7};
    };
}
