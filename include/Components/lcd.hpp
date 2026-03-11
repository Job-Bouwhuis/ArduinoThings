#pragma once

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Arduino.h>

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

        void graph(byte row, byte col, byte lengthInCharacters, byte pixelColEnd)
        {
        }

        void Clear()
        {
            lcd.clear();
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
            String s = "";
            for (int i = 0; i < cols; i++)
                s += ' ';
            Write(s, index);
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

            const size_t SAFE_BUF = 128;
            uint8_t outBufType[SAFE_BUF];
            char outBufChar[SAFE_BUF];
            uint8_t outBufCustomIndex[SAFE_BUF];
            size_t outLen = 0;

            auto AppendPlain = [&](char ch)
            {
                if (outLen >= SAFE_BUF)
                    return;
                outBufType[outLen] = 0;
                outBufChar[outLen] = ch;
                outBufCustomIndex[outLen] = 0;
                ++outLen;
            };

            auto AppendCustomBuf = [&](uint8_t regIdx)
            {
                if (outLen >= SAFE_BUF)
                    return;
                outBufType[outLen] = 1;
                outBufChar[outLen] = 0;
                outBufCustomIndex[outLen] = regIdx;
                ++outLen;
            };

            auto AppendControlBuf = [&](uint8_t ctrlType, uint8_t param)
            {
                if (outLen >= SAFE_BUF)
                    return;
                outBufType[outLen] = ctrlType;
                outBufChar[outLen] = 0;
                outBufCustomIndex[outLen] = param;
                ++outLen;
            };

            for (size_t i = 0; i < text.length();)
            {
                char c = text[i];

                if (c == '<')
                {
                    size_t j = i + 1;
                    String token = "";
                    while (j < text.length() && text[j] != '>')
                    {
                        token += text[j++];
                    }

                    if (j >= text.length())
                    {
                        AppendPlain('<');
                        for (size_t k = 0; k < token.length(); ++k)
                            AppendPlain(token[k]);
                        i = j;
                        continue;
                    }

                    int regIdx = FindRegistryIndexByName(token);
                    if (regIdx < 0)
                    {
                        AppendPlain('<');
                        for (size_t k = 0; k < token.length(); ++k)
                            AppendPlain(token[k]);
                        AppendPlain('>');
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
                        AppendCustomBuf((uint8_t)regIdx);
                    }
                    i = j + 1;
                    continue;
                }

                if (c == '#')
                {
                    if (i + 1 < text.length() && text[i + 1] == '#')
                    {
                        AppendPlain('#');
                        i += 2;
                        continue;
                    }

                    if (i + 5 < text.length() && text.substring(i + 1, i + 5) == "curx" && text[i + 5] == '(')
                    {
                        size_t j = i + 6;
                        String num = "";
                        while (j < text.length() && text[j] >= '0' && text[j] <= '9')
                            num += text[j++];
                        if (j < text.length() && text[j] == ')' && num.length() > 0)
                        {
                            int val = num.toInt();
                            if (val < 0)
                                val = 0;
                            if (val > 255)
                                val = 255;
                            AppendControlBuf(3, (uint8_t)val);
                            i = j + 1;
                            continue;
                        }
                        AppendPlain('#');
                        ++i;
                        continue;
                    }

                    if (i + 5 < text.length() && text.substring(i + 1, i + 5) == "cury" && text[i + 5] == '(')
                    {
                        size_t j = i + 6;
                        String num = "";
                        while (j < text.length() && text[j] >= '0' && text[j] <= '9')
                            num += text[j++];
                        if (j < text.length() && text[j] == ')' && num.length() > 0)
                        {
                            int val = num.toInt();
                            if (val < 0)
                                val = 0;
                            if (val > 255)
                                val = 255;
                            AppendControlBuf(4, (uint8_t)val);
                            i = j + 1;
                            continue;
                        }
                        AppendPlain('#');
                        ++i;
                        continue;
                    }

                    size_t j = i + 1;
                    String token = "";
                    while (j < text.length())
                    {
                        char nc = text[j];
                        if ((nc >= 'a' && nc <= 'z') || (nc >= 'A' && nc <= 'Z') || (nc >= '0' && nc <= '9') || nc == '.' || nc == '_')
                        {
                            token += nc;
                            ++j;
                        }
                        else
                            break;
                    }

                    bool consumed = false;
                    if (token.length() > 0 && argIndex < ARG_COUNT)
                    {
                        if (token == "num" || token == "str" || token == "bool")
                        {
                            String s = argArray[argIndex++];
                            for (size_t k = 0; k < s.length(); ++k)
                                AppendPlain(s[k]);
                            consumed = true;
                        }
                        else if (token.startsWith("dec"))
                        {
                            // dec, dec.2, dec._2
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
                                AppendPlain(formatted[k]);
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
                                AppendPlain(binStr[k]);
                            consumed = true;
                        }
                    }

                    if (!consumed)
                    {
                        AppendPlain('#');
                        for (size_t k = 0; k < token.length(); ++k)
                            AppendPlain(token[k]);
                    }

                    i = i + 1 + token.length();
                    continue;
                }

                if (c == '\n')
                {
                    AppendControlBuf(2, 0);
                    ++i;
                    continue;
                }

                AppendPlain(c);
                ++i;
            }

            if (!AllocateSlotsForUniqueRegistryList(uniqueIndices, uniqueCount))
            {
                Serial.println("cant show more than 8 custom characters");
                return;
            }

            uint8_t currentLine = 0;
            uint8_t currentCol = 0;
            lcd.setCursor(0, currentLine);

            for (size_t i = 0; i < outLen; ++i)
            {
                uint8_t t = outBufType[i];

                if (t == 0)
                {
                    if (currentCol >= cols)
                    {
                        if constexpr (ALLOW_WRAP)
                        {
                            if (currentLine == 0 && rows > 1)
                            {
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
                    lcd.write(outBufChar[i]);
                    ++currentCol;
                }
                else if (t == 1)
                {
                    if (currentCol >= cols)
                    {
                        if constexpr (ALLOW_WRAP)
                        {
                            if (currentLine == 0 && rows > 1)
                            {
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
                    uint8_t slot = registry[regIdx].slotIndex;
                    if (slot == UNASSIGNED_SLOT)
                        lcd.write('?');
                    else
                        lcd.write((uint8_t)slot);
                    ++currentCol;
                }
                else if (t == 2)
                {
                    if (rows > 1 && currentLine == 0)
                        currentLine = 1;
                    else
                        currentLine = 0;
                    currentCol = 0;
                    lcd.setCursor(0, currentLine);
                }
                else if (t == 3)
                {
                    uint8_t val = outBufCustomIndex[i];
                    if (val >= cols)
                        val = cols - 1;
                    currentCol = val;
                    lcd.setCursor(currentCol, currentLine);
                }
                else if (t == 4)
                {
                    uint8_t val = outBufCustomIndex[i];
                    if (val >= rows)
                        val = rows - 1;
                    currentLine = val;
                    lcd.setCursor(currentCol, currentLine);
                }
            }
        }

    private:
        LiquidCrystal_I2C lcd;
        byte cols;
        byte rows;
        byte blankGlyph[8] = {
            0b00000,
            0b00000,
            0b00000,
            0b00000,
            0b00000,
            0b00000,
            0b00000,
            0b00000};

        static const uint8_t MAX_CUSTOM = 8;
        static const uint8_t MAX_REGISTRY = 32;
        static const uint8_t UNASSIGNED_SLOT = 0xFF;

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

        int FindRegistryIndexByName(const String &name)
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

        void PrintGlyph(const byte glyph[8])
        {
            Serial.println("{");
            for (uint8_t i = 0; i < 8; ++i)
            {
                Serial.print("  ");
                for (uint8_t j = 0; j < 5; ++j)
                {
                    Serial.print((glyph[i] >> (4 - j)) & 1);
                    if (j < 4)
                        Serial.print(", ");
                }
                if (i < 7)
                    Serial.println(",");
                else
                    Serial.println();
            }
            Serial.println("}");
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