#pragma once

#include <Arduino.h>

namespace States
{
    struct SessionResultData
    {
        bool wonGames[4] = {false, false, false, false};
        String restartState = "mainmenu";
        bool lastGameWon = false;
        bool globalSessionWin = false;
        bool timeUp = false;
        int lastScore = 0;
    };

    inline SessionResultData &GetSessionResultData()
    {
        static SessionResultData data;
        return data;
    }

    inline int GetGameIndex(const String &gameName)
    {
        if (gameName == "game1")
            return 0;
        if (gameName == "game2")
            return 1;
        if (gameName == "game3")
            return 2;
        if (gameName == "game4")
            return 3;
        return -1;
    }

    inline void ResetSessionWins()
    {
        SessionResultData &data = GetSessionResultData();
        for (int i = 0; i < 4; ++i)
            data.wonGames[i] = false;
        data.globalSessionWin = false;
    }

    inline bool HasWonAllGamesInSession()
    {
        SessionResultData &data = GetSessionResultData();
        for (int i = 0; i < 4; ++i)
            if (!data.wonGames[i])
                return false;
        return true;
    }

    inline void PrepareGameResult(const String &gameName, bool won, int score)
    {
        SessionResultData &data = GetSessionResultData();
        data.restartState = gameName;
        data.lastGameWon = won;
        data.timeUp = false;
        data.lastScore = score;

        if (won)
        {
            const int gameIdx = GetGameIndex(gameName);
            if (gameIdx >= 0)
                data.wonGames[gameIdx] = true;
        }

        data.globalSessionWin = HasWonAllGamesInSession();
    }

    inline void PrepareTimeUpResult(const String &restartState)
    {
        SessionResultData &data = GetSessionResultData();
        data.restartState = restartState;
        data.lastGameWon = false;
        data.globalSessionWin = false;
        data.timeUp = true;
        data.lastScore = 0;
    }
}
