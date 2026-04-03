#pragma once

#include "Util/Includes.h"
#include "Tasks/ExplainTask.hpp"
#include "States/SessionResult.hpp"
#include <string>
#include <vector>
#include <random>
#include <chrono>

namespace Games
{
    class Game3 : public States::State
    {
    public:
        Game3(States::StateMachine *sm)
            : State(sm, "game3")
        {
        }

        void OnEnter() override
        {
            ResetGame();
            SetupButtons();
            isPlaying = false;

            tasks.AddTask(new Tasks::ExplainTask(
                {
                    "Avoid obstacles\nUse btn1/btn2",
                    "Btn1 = up\nBtn2 = down",
                },
                1800,
                [this]()
                {
                    isPlaying = true;
                    lastTime = millis();
                    Render();
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
                    if (score >= 25)
                    {
                        lcd->Write("You won!\nScore: #num", score);
                        buzzer->PlayEffectTone(14000, 300);
                        buzzer->PlayEffectTone(1600, 500);
                    }
                    else
                    {
                        lcd->Write("Game over!\nScore: #num", score);
                        buzzer->PlayEffectTone(1000, 300);
                        buzzer->PlayEffectTone(800, 500);
                    }
                    gameOverShown = true;
                }

                unsigned long now = millis();
                if (now - gameOverStart >= GAME_OVER_DELAY)
                {
                    lcd->Clear();
                    States::PrepareGameResult("game3", score >= 25, score);
                    stateMachine.RequestTransition("gameover");
                }

                return;
            }

            auto t = millis();
            auto delta = t - lastTime;
            time += delta;
            lastTime = t;

            Serial.printf("%d/%d\n", time, STEP_DELAY);
            while (time >= STEP_DELAY)
            {
                time = 0;
                StepGame();

                if (gameOver)
                    return;
            }
            Render();
        }

        void OnExit() override
        {
            lcd->Clear();
            wada->WriteByteAsBits(0);
        }

    private:
        static constexpr int LCD_WIDTH = 16;
        static constexpr int PLAYER_COLUMN = 7;
        static constexpr int PLAYER_CHAR = 'O';
        static constexpr int OBSTACLE_CHAR = '#';
        static constexpr int LANE_COUNT = 4;
        static constexpr int SAFE_GAP = 6;
        static constexpr int SPAWN_CHANCE_PERCENT = 35;
        static constexpr unsigned long STEP_DELAY = 500;
        static constexpr unsigned long GAME_OVER_DELAY = 2200;
        int lastSpawnedLane = -1;

        uint64_t seed =
            std::chrono::high_resolution_clock::now().time_since_epoch().count() ^
            std::random_device{}();

        std::mt19937 generator{static_cast<uint32_t>(seed)};
        std::uniform_int_distribution<int> directionDist{0, 1};
        std::uniform_int_distribution<int> spawnChanceDist{0, 99};

        struct Obstacle
        {
            int row;
            int column;
            int direction;
        };

        void ResetGame()
        {
            playerRow = 1;
            score = 0;
            gameOver = false;
            gameOverShown = false;
            obstacles.clear();

            laneDirections[0] = 0;
            laneDirections[1] = 0;
            laneDirections[2] = 0;
            laneDirections[3] = 0;
            spawnCooldowns[0] = 0;
            spawnCooldowns[1] = 0;
            spawnCooldowns[2] = 0;
            spawnCooldowns[3] = 0;

            unsigned long now = millis();
            time = 0;
            lastTime = now;
            gameOverStart = now;
        }

        void SetupButtons()
        {
            wadaButton1->OnClick.Add([this](Components::Button *btn)
                                     {
                                         if (!isPlaying || gameOver)
                                             return;

                                         if (playerRow > 0)
                                         {
                                             playerRow--;
                                             buzzer->PlayEffectTone(1200, 60);
                                         }
                                         else
                                         {
                                             buzzer->PlayEffectTone(700, 45);
                                         } });

            wadaButton2->OnClick.Add([this](Components::Button *btn)
                                     {
                                         if (!isPlaying || gameOver)
                                             return;

                                         if (playerRow < (LANE_COUNT - 1))
                                         {
                                             playerRow++;
                                             buzzer->PlayEffectTone(1200, 60);
                                         }
                                         else
                                         {
                                             buzzer->PlayEffectTone(700, 45);
                                         } });
        }

        void StepGame()
        {
            bool scoredThisTick = false;

            for (auto &obstacle : obstacles)
                obstacle.column += obstacle.direction;

            for (int row = 0; row < LANE_COUNT; row++)
                if (spawnCooldowns[row] > 0)
                    spawnCooldowns[row]--;

            bool spawnedThisTick = false;

            for (int laneIndex = 0; laneIndex < LANE_COUNT; laneIndex++)
            {
                int preferredLane = laneIndex;

                if (directionDist(generator) == 0)
                    do
                        preferredLane = generator() % LANE_COUNT;
                    while (preferredLane == playerRow);

                if (preferredLane == lastSpawnedLane && directionDist(generator) == 0)
                    preferredLane = (preferredLane + 1) % LANE_COUNT;

                if (IsLaneDangerous(playerRow))
                    for (int i = 0; i < LANE_COUNT; i++)
                        if (!IsLaneDangerous(i))
                        {
                            preferredLane = i;
                            break;
                        }

                if (!spawnedThisTick && spawnCooldowns[preferredLane] == 0)
                {
                    if (spawnChanceDist(generator) >= SPAWN_CHANCE_PERCENT)
                        continue;

                    if (TrySpawnOnLane(preferredLane))
                    {
                        lastSpawnedLane = preferredLane;
                        spawnedThisTick = true;
                        buzzer->PlayEffectTone(850, 25);
                    }
                }
            }

            for (int i = static_cast<int>(obstacles.size()) - 1; i >= 0; i--)
            {
                if (obstacles[i].column < 0 || obstacles[i].column >= LCD_WIDTH)
                {
                    obstacles.erase(obstacles.begin() + i);
                    score++;
                    scoredThisTick = true;
                    wada->Write(score);
                }
            }

            if (scoredThisTick)
                buzzer->PlayEffectTone(2000, 30);

            for (const auto &obstacle : obstacles)
            {
                if (obstacle.row == playerRow && obstacle.column == PLAYER_COLUMN)
                {
                    gameOver = true;
                    gameOverStart = millis();

                    const uint16_t sadFrequencies[] = {NOTE_A4, NOTE_F4, NOTE_D4, NOTE_B3, NOTE_C4, NOTE_D4};
                    const uint16_t sadDurations[] = {150, 120, 150, 100, 150, 200};
                    buzzer->PlayMusicTrack(sadFrequencies, sadDurations, 6);
                    return;
                }
            }

            for (int row = 0; row < LANE_COUNT; row++)
                if (!LaneHasObstacle(row))
                    laneDirections[row] = 0;
        }

        int TimeToReachPlayer(const Obstacle &obstacle) const
        {
            int distance = (obstacle.direction > 0) ? (PLAYER_COLUMN - obstacle.column) : (obstacle.column - PLAYER_COLUMN);

            if (distance < 0)
                return 0;
            return distance;
        }

        bool WouldSpawnCauseUnavoidable(int spawnRow, int spawnDir) const
        {
            int spawnColumn = (spawnDir > 0) ? 0 : (LCD_WIDTH - 1);
            int newObstacleTime = (spawnDir > 0) ? (PLAYER_COLUMN - spawnColumn) : (spawnColumn - PLAYER_COLUMN);

            for (int t = 0; t <= 6; t++)
            {
                bool lanesBlocked[LANE_COUNT] = {false};

                for (const auto &obstacle : obstacles)
                {
                    int arrivalTime = TimeToReachPlayer(obstacle) - t;
                    if (arrivalTime != 0)
                        continue;

                    lanesBlocked[obstacle.row] = true;
                }

                if (newObstacleTime - t == 0)
                    lanesBlocked[spawnRow] = true;

                bool allBlocked = true;
                for (bool blocked : lanesBlocked)
                    if (!blocked)
                    {
                        allBlocked = false;
                        break;
                    }

                if (allBlocked)
                    return true;
            }

            return false;
        }

        bool TrySpawnOnLane(int row)
        {
            int direction = laneDirections[row];

            if (direction == 0)
            {
                direction = (directionDist(generator) == 0) ? -1 : 1;
                laneDirections[row] = direction;
            }

            if (!CanSpawnOnLane(row, direction))
                return false;

            if (WouldSpawnCauseUnavoidable(row, direction))
                return false;

            Obstacle obstacle;
            obstacle.row = row;
            obstacle.direction = direction;
            obstacle.column = (direction > 0) ? 0 : (LCD_WIDTH - 1);

            obstacles.push_back(obstacle);
            spawnCooldowns[row] = SAFE_GAP;
            return true;
        }

        bool IsLaneDangerous(int row) const
        {
            for (const auto &obstacle : obstacles)
            {
                if (obstacle.row != row)
                    continue;

                if (abs(obstacle.column - PLAYER_COLUMN) <= 2)
                    return true;
            }
            return false;
        }

        bool CanSpawnOnLane(int row, int direction) const
        {
            bool foundObstacle = false;
            int nearestColumn = (direction > 0) ? LCD_WIDTH : -1;

            for (const auto &obstacle : obstacles)
            {
                if (obstacle.row != row)
                    continue;

                foundObstacle = true;

                if (direction > 0)
                {
                    if (obstacle.column < nearestColumn)
                        nearestColumn = obstacle.column;
                }
                else if (obstacle.column > nearestColumn)
                    nearestColumn = obstacle.column;
            }

            if (!foundObstacle)
                return true;

            if (direction > 0)
                return nearestColumn >= SAFE_GAP;
            else
                return nearestColumn <= (LCD_WIDTH - 1 - SAFE_GAP);
        }

        bool LaneHasObstacle(int row) const
        {
            for (const auto &obstacle : obstacles)
            {
                if (obstacle.row == row)
                    return true;
            }

            return false;
        }

        void Render()
        {
            std::string displayRow1 = "";
            std::string displayRow2 = "";

            char virtualLanes[LANE_COUNT][LCD_WIDTH];

            for (int lane = 0; lane < LANE_COUNT; lane++)
                for (int col = 0; col < LCD_WIDTH; col++)
                    virtualLanes[lane][col] = ' ';

            for (const auto &obstacle : obstacles)
                if (obstacle.column >= 0 && obstacle.column < LCD_WIDTH)
                    virtualLanes[obstacle.row][obstacle.column] = OBSTACLE_CHAR;

            // Place player
            virtualLanes[playerRow][PLAYER_COLUMN] = PLAYER_CHAR;

            for (int col = 0; col < LCD_WIDTH; col++)
            {
                char upper = virtualLanes[0][col];
                char lower = virtualLanes[1][col];
                displayRow1 += GetCombinedCharName(upper, lower);

                upper = virtualLanes[2][col];
                lower = virtualLanes[3][col];
                displayRow2 += GetCombinedCharName(upper, lower);
            }

            String renderText = String(displayRow1.c_str()) + "\n" + String(displayRow2.c_str());
            lcd->Write(renderText);
        }

        std::string GetCombinedCharName(char upper, char lower) const
        {
            if (upper == ' ' && lower == ' ')
                return "<ee>";
            else if (upper == ' ' && lower == PLAYER_CHAR)
                return "<ep>";
            else if (upper == ' ' && lower == OBSTACLE_CHAR)
                return "<eo>";
            else if (upper == PLAYER_CHAR && lower == ' ')
                return "<pe>";
            else if (upper == PLAYER_CHAR && lower == OBSTACLE_CHAR)
                return "<po>";
            else if (upper == OBSTACLE_CHAR && lower == ' ')
                return "<oe>";
            else if (upper == OBSTACLE_CHAR && lower == PLAYER_CHAR)
                return "<op>";
            else if (upper == OBSTACLE_CHAR && lower == OBSTACLE_CHAR)
                return "<oo>";

            return "<ee>";
        }

        bool gameOver = false;
        bool gameOverShown = false;
        bool isPlaying = false;

        int playerRow = 1;
        int score = 0;

        int laneDirections[LANE_COUNT] = {0, 0, 0, 0};
        int spawnCooldowns[LANE_COUNT] = {0, 0, 0, 0};

        std::vector<Obstacle> obstacles;

        unsigned long time = 0;
        unsigned long lastTime = 0;
        unsigned long gameOverStart = 0;
    };
}
