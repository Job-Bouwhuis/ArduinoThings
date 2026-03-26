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

// punctuation
#define ・ 0b10100001
#define JAP_QUOTE_BEGIN 0b10100010
#define JAP_QUOTE_END 0b10100011
#define ー 0b10100000

// vowels
#define ア 0b10110001
#define イ 0b10110010
#define ウ 0b10110011
#define エ 0b10110100
#define オ 0b10110101

// k
#define カ 0b10110110
#define キ 0b10110111
#define ク 0b10111000
#define ケ 0b10111001
#define コ 0b10111010

// s
#define サ 0b10111011
#define シ 0b10111100
#define ス 0b10111101
#define セ 0b10111110
#define ソ 0b10111111

// t
#define タ 0b11000000
#define チ 0b11000001
#define ツ 0b11000010
#define テ 0b11000011
#define ト 0b11000100

// n
#define ナ 0b11000101
#define ニ 0b11000110
#define ヌ 0b11000111
#define ネ 0b11001000
#define ノ 0b11001001

// h
#define ハ 0b11001010
#define ヒ 0b11001011
#define フ 0b11001100
#define ヘ 0b11001101
#define ホ 0b11001110

// m
#define マ 0b11001111
#define ミ 0b11010000
#define ム 0b11010001
#define メ 0b11010010
#define モ 0b11010011

// y
#define ヤ 0b11010100
#define ユ 0b11010101
#define ヨ 0b11010110

// r
#define ラ 0b11010111
#define リ 0b11011000
#define ル 0b11011001
#define レ 0b11011010
#define ロ 0b11011011

// w + n
#define ワ 0b11011100
#define ヲ 0b11011101
#define ン 0b11011110

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

        void Init()
        {
            lcd.init();
            lcd.backlight();
            Clear();
            lcd.setCursor(0, 0);
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

        void WriteRaw(uint8_t b)
        {
            lcd.write(b);
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
            uint8_t outBufChar[SAFE_BUF];
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

        LiquidCrystal_I2C lcd;
    private:
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
                                uint8_t outBufType[], uint8_t outBufChar[], uint8_t outBufCustomIndex[], size_t &outLen)
        {
            outLen = 0;

            for (size_t i = 0; i < text.length();)
            {
                uint32_t code = DecodeUTF8Char(text, i);

                uint8_t mapped = MapKatakana(code);
                if (mapped != 0)
                {
                    AppendPlainCharToBuf(mapped, outBufType, outBufChar, outBufCustomIndex, outLen, SAFE_BUF);
                    continue;
                }

                if (code < 128)
                {
                    uint8_t c = (uint8_t)code;

                    if (c == '<')
                    {
                        size_t j = i;
                        String token = "";

                        while (j < text.length() && text[j] != '>')
                        {
                            token += (char)text[j];
                            ++j;
                        }

                        if (j >= text.length())
                        {
                            AppendPlainCharToBuf((uint8_t)'<', outBufType, outBufChar, outBufCustomIndex, outLen, SAFE_BUF);
                            for (size_t k = 0; k < token.length(); ++k)
                                AppendPlainCharToBuf((uint8_t)token[k], outBufType, outBufChar, outBufCustomIndex, outLen, SAFE_BUF);
                            i = j;
                            continue;
                        }

                        int regIdx = FindRegistryIndexByName(token);
                        if (regIdx < 0)
                        {
                            AppendPlainCharToBuf((uint8_t)'<', outBufType, outBufChar, outBufCustomIndex, outLen, SAFE_BUF);
                            for (size_t k = 0; k < token.length(); ++k)
                                AppendPlainCharToBuf((uint8_t)token[k], outBufType, outBufChar, outBufCustomIndex, outLen, SAFE_BUF);
                            AppendPlainCharToBuf((uint8_t)'>', outBufType, outBufChar, outBufCustomIndex, outLen, SAFE_BUF);
                        }
                        else
                        {
                            if (!IsInUniqueList((uint8_t)regIdx, uniqueIndices, uniqueCount))
                            {
                                if (uniqueCount >= MAX_CUSTOM)
                                {
                                    Serial.println("cant show more than 8 custom characters");
                                    return;
                                }
                                uniqueIndices[uniqueCount++] = (uint8_t)regIdx;
                            }

                            AppendCustomToBuf((uint8_t)regIdx, outBufType, outBufChar, outBufCustomIndex, outLen, SAFE_BUF);
                        }

                        i = j + 1;
                        continue;
                    }

                    if (c == '#')
                    {
                        if (i + 1 < text.length() && text[i + 1] == '#')
                        {
                            AppendPlainCharToBuf((uint8_t)'#', outBufType, outBufChar, outBufCustomIndex, outLen, SAFE_BUF);
                            i += 2;
                            continue;
                        }

                        size_t j = i;
                        String token = "";

                        while (j < text.length())
                        {
                            uint8_t nc = (uint8_t)text[j];
                            if ((nc >= 'a' && nc <= 'z') || (nc >= 'A' && nc <= 'Z') ||
                                (nc >= '0' && nc <= '9') || nc == '.' || nc == '_')
                            {
                                token += (char)nc;
                                ++j;
                            }
                            else
                            {
                                break;
                            }
                        }

                        bool consumed = false;

                        if (token.length() > 0 && argIndex < ARG_COUNT)
                        {
                            if (token == "num" || token == "str" || token == "bool")
                            {
                                String s = argArray[argIndex++];
                                for (size_t k = 0; k < s.length(); ++k)
                                    AppendPlainCharToBuf((uint8_t)s[k], outBufType, outBufChar, outBufCustomIndex, outLen, SAFE_BUF);
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
                        }

                        if (!consumed)
                        {
                            AppendPlainCharToBuf((uint8_t)'#', outBufType, outBufChar, outBufCustomIndex, outLen, SAFE_BUF);
                            for (size_t k = 0; k < token.length(); ++k)
                                AppendPlainCharToBuf((uint8_t)token[k], outBufType, outBufChar, outBufCustomIndex, outLen, SAFE_BUF);
                        }

                        i = j;
                        continue;
                    }

                    if (c == '\n')
                    {
                        AppendControlToBuf(CTRL_NEWLINE_TOGGLE, 0, outBufType, outBufChar, outBufCustomIndex, outLen, SAFE_BUF);
                        continue;
                    }

                    AppendPlainCharToBuf(c, outBufType, outBufChar, outBufCustomIndex, outLen, SAFE_BUF);
                    continue;
                }

                AppendPlainCharToBuf((uint8_t)'?', outBufType, outBufChar, outBufCustomIndex, outLen, SAFE_BUF);
            }
        }

        void AppendPlainCharToBuf(uint8_t c, uint8_t outType[], uint8_t outChar[], uint8_t outCustom[], size_t &outLen, size_t maxLen)
        {
            if (outLen >= maxLen)
                return;

            Serial.printf("appending: %c\n", (char)c);

            outType[outLen] = CTRL_PLAIN;
            outChar[outLen] = c;
            outCustom[outLen] = 0;
            ++outLen;
        }

        void AppendCustomToBuf(uint8_t regIdx, uint8_t outType[], uint8_t outChar[], uint8_t outCustom[], size_t &outLen, size_t maxLen)
        {
            if (outLen >= maxLen)
                return;
            outType[outLen] = CTRL_CUSTOM;
            outChar[outLen] = 0;
            outCustom[outLen] = regIdx;
            ++outLen;
        }

        void AppendControlToBuf(uint8_t ctrlType, uint8_t param, uint8_t outType[], uint8_t outChar[], uint8_t outCustom[], size_t &outLen, size_t maxLen)
        {
            if (outLen >= maxLen)
                return;
            outType[outLen] = ctrlType;
            outChar[outLen] = 0;
            outCustom[outLen] = param;
            ++outLen;
        }

        template <bool ALLOW_WRAP>
        void RenderBuffers(uint8_t outBufType[], uint8_t outBufChar[], uint8_t outBufCustomIndex[], size_t outLen,
                           uint8_t uniqueIndices[], uint8_t uniqueCount)
        {
            uint8_t currentLine = 0;
            uint8_t currentCol = 0;
            lcd.setCursor(0, currentLine);

            auto WriteByteSafely = [&](uint8_t value)
            {
                if (value >= 0x80)
                    lcd.setCursor(currentCol, currentLine);

                lcd.write(value);
            };

            struct PendingUnique
            {
                bool active = false;
                uint8_t startLine = 0;
                bool nextCleared = false;
                uint8_t *bufType = nullptr;
                uint8_t *bufChar = nullptr;
                uint8_t *bufCustom = nullptr;
            } pendingUnique;

            auto startPending = [&]()
            {
                if (pendingUnique.active)
                    return;

                pendingUnique.bufType = new uint8_t[cols];
                pendingUnique.bufChar = new uint8_t[cols];
                pendingUnique.bufCustom = new uint8_t[cols];

                for (uint8_t i = 0; i < cols; ++i)
                {
                    pendingUnique.bufType[i] = 0xFF;
                    pendingUnique.bufChar[i] = 0;
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
                        lcd.write((uint8_t)' ');
                    }
                    else if (pt == CTRL_PLAIN)
                    {
                        WriteByteSafely(outBufChar[c]);
                    }
                    else if (pt == CTRL_CUSTOM)
                    {
                        uint8_t ridx = pendingUnique.bufCustom[c];
                        uint8_t slot = (ridx < MAX_REGISTRY) ? registry[ridx].slotIndex : UNASSIGNED_SLOT;
                        if (slot == UNASSIGNED_SLOT)
                            lcd.write((uint8_t)'?');
                        else
                            lcd.write(slot);
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
                       WriteByteSafely(outBufChar[i]);
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
                            lcd.write((uint8_t)'?');
                        else
                            lcd.write(slot);
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
                        if (tt == CTRL_PLAIN || tt == CTRL_CUSTOM)
                            ++printable;
                    }

                    uint8_t startCol = 0;
                    if (printable < cols)
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

        uint32_t DecodeUTF8Char(const String &text, size_t &i)
        {
            uint8_t c = (uint8_t)text[i];

            if (c < 0x80)
            {
                ++i;
                return c;
            }

            if ((c >> 5) == 0b110)
            {
                uint8_t c1 = (uint8_t)text[i + 1];
                uint32_t result = ((uint32_t)(c & 0x1F) << 6) |
                                  (uint32_t)(c1 & 0x3F);
                i += 2;
                return result;
            }

            if ((c >> 4) == 0b1110)
            {
                uint8_t c1 = (uint8_t)text[i + 1];
                uint8_t c2 = (uint8_t)text[i + 2];
                uint32_t result = ((uint32_t)(c & 0x0F) << 12) |
                                  ((uint32_t)(c1 & 0x3F) << 6) |
                                  (uint32_t)(c2 & 0x3F);
                i += 3;
                return result;
            }

            ++i;
            return c;
        }

        uint8_t MapKatakana(uint32_t c)
        {
            switch (c)
            {
            case U'ア':
                return ア;
            case U'イ':
                return イ;
            case U'ウ':
                return ウ;
            case U'エ':
                return エ;
            case U'オ':
                return オ;

            case U'カ':
                return カ;
            case U'キ':
                return キ;
            case U'ク':
                return ク;
            case U'ケ':
                return ケ;
            case U'コ':
                return コ;

            case U'サ':
                return サ;
            case U'シ':
                return シ;
            case U'ス':
                return ス;
            case U'セ':
                return セ;
            case U'ソ':
                return ソ;

            case U'タ':
                return タ;
            case U'チ':
                return チ;
            case U'ツ':
                return ツ;
            case U'テ':
                return テ;
            case U'ト':
                return ト;

            case U'ナ':
                return ナ;
            case U'ニ':
                return ニ;
            case U'ヌ':
                return ヌ;
            case U'ネ':
                return ネ;
            case U'ノ':
                return ノ;

            case U'ハ':
                return ハ;
            case U'ヒ':
                return ヒ;
            case U'フ':
                return フ;
            case U'ヘ':
                return ヘ;
            case U'ホ':
                return ホ;

            case U'マ':
                return マ;
            case U'ミ':
                return ミ;
            case U'ム':
                return ム;
            case U'メ':
                return メ;
            case U'モ':
                return モ;

            case U'ヤ':
                return ヤ;
            case U'ユ':
                return ユ;
            case U'ヨ':
                return ヨ;

            case U'ラ':
                return ラ;
            case U'リ':
                return リ;
            case U'ル':
                return ル;
            case U'レ':
                return レ;
            case U'ロ':
                return ロ;

            case U'ワ':
                return ワ;
            case U'ヲ':
                return ヲ;
            case U'ン':
                return ン;

            case U'・':
                return ・;
            case U'「':
                return JAP_QUOTE_BEGIN;
            case U'」':
                return JAP_QUOTE_END;
            case U'ー':
                return ー;

            default:
                return 0;
            }
        }
    };
}
