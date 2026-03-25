#pragma once

#include "Util/Includes.h"
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
            Render();
        }

        void Tick() override
        {
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
                    replayState->SetReplayState("game3");
                    stateMachine.RequestTransition("replay");
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
        static constexpr int LANE_COUNT = 2;
        static constexpr int SAFE_GAP = 4;
        static constexpr unsigned long STEP_DELAY = 500;
        static constexpr unsigned long GAME_OVER_DELAY = 2200;
        int lastSpawnedLane = -1;

        uint64_t seed =
            std::chrono::high_resolution_clock::now().time_since_epoch().count() ^
            std::random_device{}();

        std::mt19937 generator{static_cast<uint32_t>(seed)};
        std::uniform_int_distribution<int> directionDist{0, 1};

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
            spawnCooldowns[0] = 0;
            spawnCooldowns[1] = 0;

            unsigned long now = millis();
            time = 0;
            lastTime = now;
            gameOverStart = now;
        }

        void SetupButtons()
        {
            wadaButton1->OnClick.Add([this](Components::Button *btn)
                                     {
                                         if (gameOver)
                                             return;

                                         playerRow = (playerRow == 0) ? 1 : 0;
                                         buzzer->PlayEffectTone(1100, 60); });
        }

        void StepGame()
        {
            for (auto &obstacle : obstacles)
            {
                obstacle.column += obstacle.direction;
            }

            for (int row = 0; row < LANE_COUNT; row++)
            {
                if (spawnCooldowns[row] > 0)
                    spawnCooldowns[row]--;
            }

            bool spawnedThisTick = false; // new flag

            for (int laneIndex = 0; laneIndex < LANE_COUNT; laneIndex++)
            {
                int preferredLane = laneIndex;

                if (directionDist(generator) == 0)
                    preferredLane = 1 - playerRow;

                if (preferredLane == lastSpawnedLane && directionDist(generator) == 0)
                    preferredLane = 1 - preferredLane;

                if (IsLaneDangerous(playerRow))
                    preferredLane = 1 - playerRow;

                if (!spawnedThisTick && spawnCooldowns[preferredLane] == 0)
                {
                    if (TrySpawnOnLane(preferredLane))
                    {
                        lastSpawnedLane = preferredLane;
                        spawnedThisTick = true;
                    }
                }
            }

            for (int i = static_cast<int>(obstacles.size()) - 1; i >= 0; i--)
            {
                if (obstacles[i].column < 0 || obstacles[i].column >= LCD_WIDTH)
                {
                    obstacles.erase(obstacles.begin() + i);
                    score++;
                    wada->Write(score);
                }
            }

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
            {
                if (!LaneHasObstacle(row))
                    laneDirections[row] = 0;
            }
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
                bool topBlocked = false;
                bool bottomBlocked = false;

                for (const auto &obstacle : obstacles)
                {
                    int arrivalTime = TimeToReachPlayer(obstacle) - t;
                    if (arrivalTime != 0)
                        continue;

                    if (obstacle.row == 0)
                        topBlocked = true;
                    else
                        bottomBlocked = true;
                }

                if (newObstacleTime - t == 0)
                {
                    if (spawnRow == 0)
                        topBlocked = true;
                    else
                        bottomBlocked = true;
                }

                if (topBlocked && bottomBlocked)
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
                else
                {
                    if (obstacle.column > nearestColumn)
                        nearestColumn = obstacle.column;
                }
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
            std::string topRow(LCD_WIDTH, ' ');
            std::string bottomRow(LCD_WIDTH, ' ');

            for (const auto &obstacle : obstacles)
            {
                if (obstacle.column < 0 || obstacle.column >= LCD_WIDTH)
                    continue;

                if (obstacle.row == 0)
                    topRow[obstacle.column] = OBSTACLE_CHAR;
                else
                    bottomRow[obstacle.column] = OBSTACLE_CHAR;
            }

            if (playerRow == 0)
                topRow[PLAYER_COLUMN] = PLAYER_CHAR;
            else
                bottomRow[PLAYER_COLUMN] = PLAYER_CHAR;

            lcd->Write("#unique#str\n#str", topRow.c_str(), bottomRow.c_str());
        }

        bool gameOver = false;
        bool gameOverShown = false;

        int playerRow = 1;
        int score = 0;

        int laneDirections[LANE_COUNT] = {0, 0};
        int spawnCooldowns[LANE_COUNT] = {0, 0};

        std::vector<Obstacle> obstacles;

        unsigned long time = 0;
        unsigned long lastTime = 0;
        unsigned long gameOverStart = 0;
    };
}