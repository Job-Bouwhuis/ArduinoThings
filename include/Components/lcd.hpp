#pragma once

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Arduino.h>

#define __EMPTY_LINE_STRING__ "                "
#define CTRL_PLAIN 0
#define CTRL_CUSTOM 1
#define CTRL_NEWLINE_TOGGLE 2
#define CTRL_SET_CURSOR_X 3
#define CTRL_SET_CURSOR_Y 4
#define CTRL_UNIQUE 5
#define CTRL_RIGHTALIGN 6
#define MAX_CUSTOM (uint8_t)8
#define MAX_REGISTRY (uint8_t)32
#define UNASSIGNED_SLOT (uint8_t)0xFF
#define SAFE_BUF (uint8_t)128

namespace Components
{
    class Lcd
    {
    public:
        Lcd(uint8_t i2cAddress, uint8_t cols = 16, uint8_t rows = 2)
            : lcd(i2cAddress, cols, rows), cols(cols), rows(rows)
        {
            for (uint8_t i = 0; i < MAX_REGISTRY; ++i)
            {
                registry[i].valid = false;
                registry[i].slotIndex = UNASSIGNED_SLOT;
            }
            for (uint8_t i = 0; i < MAX_CUSTOM; ++i)
            {
                slotOwner[i] = 0xFF;
            }
        }

        void writeKatakanaByte(uint8_t byte)
        {
            lcd.send(byte, 1);
        }

        void Init()
        {
            lcd.init();
            lcd.backlight();
            lcd.clear();
        }

        void Backlight(bool state)
        {
            lcd.setBacklight(state);
        }

        void CursorBlink(bool state)
        {
            if (state)
                lcd.cursor_on();
            else
                lcd.cursor_off();
        }

        void Clear()
        {
            ClearLine(0);
            ClearLine(1);
        }

        void SetCursor(uint8_t col, uint8_t row)
        {
            if (col >= cols || row >= rows)
                return;
            lcd.setCursor(col, row);
        }

        bool RegisterCustom(const String &name, const byte glyph[8])
        {
            int idx = FindRegistryIndexByName(name);
            if (idx >= 0)
            {
                if (!GlyphsEqual(registry[idx].glyph, glyph))
                {
                    memcpy(registry[idx].glyph, glyph, 8);
                    if (registry[idx].slotIndex != UNASSIGNED_SLOT)
                    {
                        slotOwner[registry[idx].slotIndex] = 0xFF;
                        registry[idx].slotIndex = UNASSIGNED_SLOT;
                    }
                }
                return true;
            }

            int freeIdx = FindFreeRegistryIndex();
            if (freeIdx < 0)
                return false;

            registry[freeIdx].valid = true;
            registry[freeIdx].name = name;
            memcpy(registry[freeIdx].glyph, glyph, 8);
            registry[freeIdx].slotIndex = UNASSIGNED_SLOT;
            return true;
        }

        inline void ClearLine(byte index)
        {
            Write("#cury#str", index, __EMPTY_LINE_STRING__);
        }

        void UnregisterCustom(const String &name)
        {
            int idx = FindRegistryIndexByName(name);
            if (idx < 0)
                return;
            if (registry[idx].slotIndex != UNASSIGNED_SLOT)
            {
                uint8_t s = registry[idx].slotIndex;
                if (s < MAX_CUSTOM)
                    slotOwner[s] = 0xFF;
            }
            registry[idx].valid = false;
            registry[idx].slotIndex = UNASSIGNED_SLOT;
            registry[idx].name = "";
        }

        template <bool ALLOW_WRAP = false, typename... Args>
        void Write(const String &text, Args... args)
        {
            if (rows == 0)
                return;

            constexpr size_t ARG_COUNT = sizeof...(Args);
            String argArray[ARG_COUNT == 0 ? 1 : ARG_COUNT] = {ArgToString(args)...};
            size_t argIndex = 0;

            uint8_t uniqueIndices[MAX_CUSTOM];
            uint8_t uniqueCount = 0;

            uint8_t outBufType[SAFE_BUF];
            char outBufChar[SAFE_BUF];
            uint8_t outBufCustomIndex[SAFE_BUF];
            size_t outLen = 0;

            ParseTextToBuffers(text, argArray, ARG_COUNT, argIndex,
                                         uniqueIndices, uniqueCount,
                                         outBufType, outBufChar, outBufCustomIndex, outLen);

            if (!AllocateSlotsForUniqueRegistryList(uniqueIndices, uniqueCount))
            {
                Serial.println("cant show more than 8 custom characters");
                return;
            }

            RenderBuffers<ALLOW_WRAP>(outBufType, outBufChar, outBufCustomIndex, outLen,
                                                uniqueIndices, uniqueCount);
        }

    private:
        LiquidCrystal_I2C lcd;
        byte cols;
        byte rows;

        struct RegistryEntry
        {
            String name;
            uint8_t glyph[8];
            bool valid;
            uint8_t slotIndex;
        };

        RegistryEntry registry[MAX_REGISTRY];
        uint8_t slotOwner[MAX_CUSTOM];

        String ArgToString(const String &v) { return v; }
        String ArgToString(const char *v) { return v ? String(v) : String(""); }
        String ArgToString(bool v) { return v ? String("true") : String("false"); }
        template <typename T>
        String ArgToString(T v) { return String(v); }

        short FindRegistryIndexByName(const String &name)
        {
            for (uint8_t i = 0; i < MAX_REGISTRY; ++i)
            {
                if (registry[i].valid && registry[i].name == name)
                    return i;
            }
            return -1;
        }

        int FindFreeRegistryIndex()
        {
            for (uint8_t i = 0; i < MAX_REGISTRY; ++i)
            {
                if (!registry[i].valid)
                    return i;
            }
            return -1;
        }

        bool GlyphsEqual(const uint8_t a[8], const uint8_t b[8])
        {
            for (uint8_t i = 0; i < 8; ++i)
                if (a[i] != b[i])
                    return false;
            return true;
        }

        bool IsInUniqueList(uint8_t value, uint8_t list[], uint8_t count)
        {
            for (uint8_t i = 0; i < count; ++i)
                if (list[i] == value)
                    return true;
            return false;
        }

        void AppendPlainChar(char c, uint8_t outType[], char outChar[], uint8_t outCustom[], size_t &outLen, size_t maxLen)
        {
            if (outLen >= maxLen)
                return;
            outType[outLen] = 0;
            outChar[outLen] = c;
            outCustom[outLen] = 0;
            ++outLen;
        }

        void AppendCustom(uint8_t regIdx, uint8_t outType[], char outChar[], uint8_t outCustom[], size_t &outLen, size_t maxLen)
        {
            if (outLen >= maxLen)
                return;
            outType[outLen] = 1;
            outChar[outLen] = 0;
            outCustom[outLen] = regIdx;
            ++outLen;
        }

        bool AllocateSlotsForUniqueRegistryList(uint8_t uniqueIndices[], uint8_t uniqueCount)
        {
            for (uint8_t ui = 0; ui < uniqueCount; ++ui)
            {
                uint8_t ridx = uniqueIndices[ui];
                if (registry[ridx].slotIndex != UNASSIGNED_SLOT)
                    continue;

                bool found = false;

                for (uint8_t s = 0; s < MAX_CUSTOM; ++s)
                {
                    if (slotOwner[s] == 0xFF)
                        continue;

                    uint8_t ownerIdx = slotOwner[s];

                    if (ownerIdx >= MAX_REGISTRY)
                        continue;

                    if (!IsInUniqueList(ownerIdx, uniqueIndices, uniqueCount))
                        continue;

                    if (GlyphsEqual(registry[ownerIdx].glyph, registry[ridx].glyph))
                    {
                        registry[ridx].slotIndex = s;
                        found = true;
                        break;
                    }
                }

                if (found)
                    continue;
            }

            for (uint8_t ui = 0; ui < uniqueCount; ++ui)
            {
                uint8_t ridx = uniqueIndices[ui];
                if (registry[ridx].slotIndex != UNASSIGNED_SLOT)
                    continue;

                int freeSlot = FindFreeSlot();
                if (freeSlot >= 0)
                {
                    uint8_t s = (uint8_t)freeSlot;
                    lcd.createChar(s, registry[ridx].glyph);
                    slotOwner[s] = ridx;
                    registry[ridx].slotIndex = s;
                    continue;
                }

                int replaceSlot = FindReplaceableSlot(uniqueIndices, uniqueCount);
                if (replaceSlot >= 0)
                {
                    byte prevOwner = slotOwner[replaceSlot];
                    if (prevOwner < MAX_REGISTRY)
                        registry[prevOwner].slotIndex = UNASSIGNED_SLOT;

                    lcd.createChar(replaceSlot, registry[ridx].glyph);
                    lcd.setCursor(0, 0);
                    slotOwner[replaceSlot] = ridx;
                    registry[ridx].slotIndex = replaceSlot;
                    continue;
                }

                return false;
            }

            for (uint8_t ui = 0; ui < uniqueCount; ++ui)
            {
                uint8_t ridx = uniqueIndices[ui];
                if (registry[ridx].slotIndex == UNASSIGNED_SLOT)
                    return false;
            }

            return true;
        }

        void ParseTextToBuffers(const String &text, String argArray[], size_t ARG_COUNT, size_t &argIndex,
                                uint8_t uniqueIndices[], uint8_t &uniqueCount,
                                uint8_t outBufType[], char outBufChar[], uint8_t outBufCustomIndex[], size_t &outLen)
        {
            outLen = 0;

            for (size_t i = 0; i < text.length();)
            {
                char c = text[i];

                if (c == '<')
                {
                    size_t j = i + 1;
                    String token = "";
                    while (j < text.length() && text[j] != '>')
                        token += text[j++];

                    if (j >= text.length())
                    {
                        AppendPlainCharToBuf('<', outBufType, outBufChar, outBufCustomIndex, outLen, SAFE_BUF);
                        for (size_t k = 0; k < token.length(); ++k)
                            AppendPlainCharToBuf(token[k], outBufType, outBufChar, outBufCustomIndex, outLen, SAFE_BUF);
                        i = j;
                        continue;
                    }

                    int regIdx = FindRegistryIndexByName(token);
                    if (regIdx < 0)
                    {
                        AppendPlainCharToBuf('<', outBufType, outBufChar, outBufCustomIndex, outLen, SAFE_BUF);
                        for (size_t k = 0; k < token.length(); ++k)
                            AppendPlainCharToBuf(token[k], outBufType, outBufChar, outBufCustomIndex, outLen, SAFE_BUF);
                        AppendPlainCharToBuf('>', outBufType, outBufChar, outBufCustomIndex, outLen, SAFE_BUF);
                    }
                    else
                    {
                        if (!IsInUniqueList(regIdx, uniqueIndices, uniqueCount))
                        {
                            if (uniqueCount >= MAX_CUSTOM)
                            {
                                Serial.println("cant show more than 8 custom characters");
                                return;
                            }
                            uniqueIndices[uniqueCount++] = regIdx;
                        }
                        AppendCustomToBuf(regIdx, outBufType, outBufChar, outBufCustomIndex, outLen, SAFE_BUF);
                    }
                    i = j + 1;
                    continue;
                }

                if (c == '#')
                {
                    if (i + 1 < text.length() && text[i + 1] == '#')
                    {
                        AppendPlainCharToBuf('#', outBufType, outBufChar, outBufCustomIndex, outLen, SAFE_BUF);
                        i += 2;
                        continue;
                    }

                    size_t j = i + 1;
                    String token = "";

                    while (j < text.length())
                    {
                        char nc = text[j];
                        if ((nc >= 'a' && nc <= 'z') || (nc >= 'A' && nc <= 'Z') ||
                            (nc >= '0' && nc <= '9') || nc == '.' || nc == '_')
                        {
                            token += nc;
                            ++j;
                        }
                        else
                            break;
                    }

                    if (j + 1 < text.length() && text[j] == '\\' && text[j + 1] == '#')
                        j += 2;

                    bool consumed = false;

                    if (token.length() > 0 && argIndex < ARG_COUNT)
                    {
                        if (token == "num" || token == "str" || token == "bool")
                        {
                            String s = argArray[argIndex++];
                            for (size_t k = 0; k < s.length(); ++k)
                                AppendPlainCharToBuf(s[k], outBufType, outBufChar, outBufCustomIndex, outLen, SAFE_BUF);
                            consumed = true;
                        }
                        else if (token == "curx" || token == "cury")
                        {
                            int val = argArray[argIndex++].toInt();
                            if (token == "curx")
                                AppendControlToBuf(CTRL_SET_CURSOR_X, (uint8_t)val, outBufType, outBufChar, outBufCustomIndex, outLen, SAFE_BUF);
                            else
                                AppendControlToBuf(CTRL_SET_CURSOR_Y, (uint8_t)val, outBufType, outBufChar, outBufCustomIndex, outLen, SAFE_BUF);
                            consumed = true;
                        }
                        else if (token == "unique")
                        {
                            AppendControlToBuf(CTRL_UNIQUE, 0, outBufType, outBufChar, outBufCustomIndex, outLen, SAFE_BUF);
                            consumed = true;
                        }
                        else if (token == "rightalign")
                        {
                            AppendControlToBuf(CTRL_RIGHTALIGN, 0, outBufType, outBufChar, outBufCustomIndex, outLen, SAFE_BUF);
                            consumed = true;
                        }
                        else if (token.startsWith("dec"))
                        {
                            int precision = 0;
                            bool suppressLeadingZero = false;
                            int dotIndex = token.indexOf('.');
                            if (dotIndex >= 0)
                            {
                                String afterDot = token.substring(dotIndex + 1);
                                if (afterDot.startsWith("_"))
                                {
                                    suppressLeadingZero = true;
                                    afterDot = afterDot.substring(1);
                                }
                                precision = afterDot.length();
                            }

                            double val = argArray[argIndex++].toFloat();
                            String formatted = String(val, precision);
                            if (suppressLeadingZero && formatted.startsWith("0"))
                                formatted = formatted.substring(1);
                            for (size_t k = 0; k < formatted.length(); ++k)
                                AppendPlainCharToBuf(formatted[k], outBufType, outBufChar, outBufCustomIndex, outLen, SAFE_BUF);
                            consumed = true;
                        }
                        else if (token == "bin")
                        {
                            long val = argArray[argIndex++].toInt();
                            String binStr = "";
                            for (int b = sizeof(long) * 8 - 1; b >= 0; --b)
                                binStr += ((val >> b) & 1) ? '1' : '0';
                            int firstOne = binStr.indexOf('1');
                            if (firstOne >= 0)
                                binStr = binStr.substring(firstOne);
                            else
                                binStr = "0";
                            for (size_t k = 0; k < binStr.length(); ++k)
                                AppendPlainCharToBuf(binStr[k], outBufType, outBufChar, outBufCustomIndex, outLen, SAFE_BUF);
                            consumed = true;
                        }
                    }

                    if (!consumed)
                    {
                        AppendPlainCharToBuf('#', outBufType, outBufChar, outBufCustomIndex, outLen, SAFE_BUF);
                        for (size_t k = 0; k < token.length(); ++k)
                            AppendPlainCharToBuf(token[k], outBufType, outBufChar, outBufCustomIndex, outLen, SAFE_BUF);
                    }

                    i = j;
                    continue;
                }

                if (c == '\n')
                {
                    AppendControlToBuf(CTRL_NEWLINE_TOGGLE, 0, outBufType, outBufChar, outBufCustomIndex, outLen, SAFE_BUF);
                    ++i;
                    continue;
                }

                AppendPlainCharToBuf(c, outBufType, outBufChar, outBufCustomIndex, outLen, SAFE_BUF);
                ++i;
            }
        }

        void AppendPlainCharToBuf(char c, uint8_t outType[], char outChar[], uint8_t outCustom[], size_t &outLen, size_t maxLen)
        {
            if (outLen >= maxLen)
                return;
            outType[outLen] = CTRL_PLAIN;
            outChar[outLen] = c;
            outCustom[outLen] = 0;
            ++outLen;
        }

        void AppendCustomToBuf(uint8_t regIdx, uint8_t outType[], char outChar[], uint8_t outCustom[], size_t &outLen, size_t maxLen)
        {
            if (outLen >= maxLen)
                return;
            outType[outLen] = CTRL_CUSTOM;
            outChar[outLen] = 0;
            outCustom[outLen] = regIdx;
            ++outLen;
        }

        void AppendControlToBuf(uint8_t ctrlType, uint8_t param, uint8_t outType[], char outChar[], uint8_t outCustom[], size_t &outLen, size_t maxLen)
        {
            if (outLen >= maxLen)
                return;
            outType[outLen] = ctrlType;
            outChar[outLen] = 0;
            outCustom[outLen] = param;
            ++outLen;
        }

        template <bool ALLOW_WRAP>
        void RenderBuffers(uint8_t outBufType[], char outBufChar[], uint8_t outBufCustomIndex[], size_t outLen,
                           uint8_t uniqueIndices[], uint8_t uniqueCount)
        {
            uint8_t currentLine = 0;
            uint8_t currentCol = 0;
            lcd.setCursor(0, currentLine);

            struct PendingUnique
            {
                bool active = false;
                uint8_t startLine = 0;
                bool nextCleared = false;
                uint8_t *bufType = nullptr;
                char *bufChar = nullptr;
                uint8_t *bufCustom = nullptr; 
            } pendingUnique;
            pendingUnique.active = false;

            auto startPending = [&]()
            {
                if (pendingUnique.active)
                    return;

                pendingUnique.bufType = new uint8_t[cols];
                pendingUnique.bufChar = new char[cols];
                pendingUnique.bufCustom = new uint8_t[cols];

                for (uint8_t i = 0; i < cols; ++i)
                {
                    pendingUnique.bufType[i] = 0xFF;
                    pendingUnique.bufChar[i] = ' ';
                    pendingUnique.bufCustom[i] = 0xFF;
                }

                pendingUnique.active = true;
                pendingUnique.startLine = currentLine;
                pendingUnique.nextCleared = false;
            };

            auto flushPending = [&]()
            {
                if (!pendingUnique.active)
                    return;
                uint8_t line = pendingUnique.startLine;
                lcd.setCursor(0, line);
                for (uint8_t c = 0; c < cols; ++c)
                {
                    uint8_t pt = pendingUnique.bufType[c];
                    if (pt == 0xFF)
                    {
                        lcd.write(' ');
                    }
                    else if (pt == CTRL_PLAIN)
                    {
                        lcd.write((uint8_t)pendingUnique.bufChar[c]);
                    }
                    else if (pt == CTRL_CUSTOM)
                    {
                        uint8_t ridx = pendingUnique.bufCustom[c];
                        uint8_t slot = (ridx < MAX_REGISTRY) ? registry[ridx].slotIndex : UNASSIGNED_SLOT;
                        if (slot == UNASSIGNED_SLOT)
                            lcd.write('?');
                        else
                            lcd.write((uint8_t)slot);
                    }
                }
                delete[] pendingUnique.bufType;
                delete[] pendingUnique.bufChar;
                delete[] pendingUnique.bufCustom;
                pendingUnique.bufType = nullptr;
                pendingUnique.bufChar = nullptr;
                pendingUnique.bufCustom = nullptr;
                pendingUnique.active = false;
            };

            for (size_t i = 0; i < outLen; ++i)
            {
                uint8_t t = outBufType[i];

                if (t == CTRL_PLAIN)
                {
                    if (currentCol >= cols)
                    {
                        if constexpr (ALLOW_WRAP)
                        {
                            if (currentLine == 0 && rows > 1)
                            {
                                if (pendingUnique.active && !pendingUnique.nextCleared && pendingUnique.startLine != 1)
                                {
                                    flushPending();
                                    pendingUnique.nextCleared = true;
                                }
                                currentLine = 1;
                                currentCol = 0;
                                lcd.setCursor(0, currentLine);
                            }
                            else
                            {
                                break;
                            }
                        }
                        else
                        {
                            break;
                        }
                    }

                    if (pendingUnique.active && pendingUnique.startLine == currentLine)
                    {
                        pendingUnique.bufType[currentCol] = CTRL_PLAIN;
                        pendingUnique.bufChar[currentCol] = outBufChar[i];
                        pendingUnique.bufCustom[currentCol] = 0xFF;
                    }
                    else
                    {
                        lcd.write(outBufChar[i]);
                    }
                    ++currentCol;
                }
                else if (t == CTRL_CUSTOM)
                {
                    if (currentCol >= cols)
                    {
                        if constexpr (ALLOW_WRAP)
                        {
                            if (currentLine == 0 && rows > 1)
                            {
                                if (pendingUnique.active && !pendingUnique.nextCleared && pendingUnique.startLine != 1)
                                {
                                    flushPending();
                                    pendingUnique.nextCleared = true;
                                }
                                currentLine = 1;
                                currentCol = 0;
                                lcd.setCursor(0, currentLine);
                            }
                            else
                            {
                                break;
                            }
                        }
                        else
                        {
                            break;
                        }
                    }

                    uint8_t regIdx = outBufCustomIndex[i];
                    if (pendingUnique.active && pendingUnique.startLine == currentLine)
                    {
                        pendingUnique.bufType[currentCol] = CTRL_CUSTOM;
                        pendingUnique.bufChar[currentCol] = 0;
                        pendingUnique.bufCustom[currentCol] = regIdx;
                    }
                    else
                    {
                        uint8_t slot = registry[regIdx].slotIndex;
                        if (slot == UNASSIGNED_SLOT)
                            lcd.write('?');
                        else
                            lcd.write((uint8_t)slot);
                    }
                    ++currentCol;
                }
                else if (t == CTRL_NEWLINE_TOGGLE)
                {
                    if (pendingUnique.active && pendingUnique.startLine == currentLine)
                        flushPending();

                    if (rows > 1 && currentLine == 0)
                        currentLine = 1;
                    else
                        currentLine = 0;
                    currentCol = 0;
                    lcd.setCursor(0, currentLine);
                }
                else if (t == CTRL_SET_CURSOR_X)
                {
                    uint8_t val = outBufCustomIndex[i];
                    if (val >= cols)
                        val = cols - 1;
                    currentCol = val;
                    if (!(pendingUnique.active && pendingUnique.startLine == currentLine))
                        lcd.setCursor(currentCol, currentLine);
                }
                else if (t == CTRL_SET_CURSOR_Y)
                {
                    uint8_t val = outBufCustomIndex[i];
                    if (val >= rows)
                        val = rows - 1;

                    if (pendingUnique.active)
                        pendingUnique.startLine = val;

                    currentLine = val;
                    if (!(pendingUnique.active && pendingUnique.startLine == currentLine))
                        lcd.setCursor(currentCol, currentLine);
                }
                else if (t == CTRL_UNIQUE)
                {
                    startPending();
                    currentCol = 0;
                }
                else if (t == CTRL_RIGHTALIGN)
                {
                    size_t printable = 0;
                    for (size_t j = i + 1; j < outLen; ++j)
                    {
                        uint8_t tt = outBufType[j];
                        if (tt == CTRL_NEWLINE_TOGGLE)
                            break;
                        if (tt == CTRL_PLAIN)
                            ++printable;
                        else if (tt == CTRL_CUSTOM)
                            ++printable;
                        else if (tt == CTRL_RIGHTALIGN || tt == CTRL_UNIQUE || tt == CTRL_SET_CURSOR_X || tt == CTRL_SET_CURSOR_Y)
                            continue;
                    }

                    uint8_t startCol = 0;
                    if (printable >= cols)
                        startCol = 0;
                    else
                        startCol = (uint8_t)(cols - printable);

                    currentCol = startCol;
                    if (!(pendingUnique.active && pendingUnique.startLine == currentLine))
                        lcd.setCursor(currentCol, currentLine);
                }
            }

            if (pendingUnique.active)
                flushPending();
        }

        int FindFreeSlot()
        {
            for (uint8_t s = 0; s < MAX_CUSTOM; ++s)
            {
                if (slotOwner[s] == 0xFF)
                    return (int)s;
            }
            return -1;
        }

        int FindReplaceableSlot(uint8_t requiredList[], uint8_t requiredCount)
        {
            for (uint8_t s = 0; s < MAX_CUSTOM; ++s)
            {
                if (slotOwner[s] == 0xFF)
                    continue;
                uint8_t owner = slotOwner[s];
                if (!IsInUniqueList(owner, requiredList, requiredCount))
                {
                    return s;
                }
            }
            return -1;
        }
    };
}
