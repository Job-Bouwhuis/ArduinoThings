#pragma once
#include <WString.h>
#include "Arduino.h"
#include <type_traits>
#include <sstream>
#include <string>

namespace Util
{
    class ConsoleProvider
    {
    public:
        template <typename... Args>
        void Write(Args &&...args)
        {
            Serial.print(ConcatToString(std::forward<Args>(args)...));
        }

        template <typename... Args>
        void WriteLine(Args &&...args)
        {
            Serial.println(ConcatToString(std::forward<Args>(args)...));
        }

        void WriteLine()
        {
            Serial.println();
        }

        String ReadLine()
        {
            while (!Serial.available())
                asm volatile("nop");

            return Serial.readString();
        }

    private:
        template <typename T>
        String ToStringGeneric(const T &value)
        {
            using bare_t = std::decay_t<T>;

            if constexpr (std::is_same_v<bare_t, String>)
                return value;
            else if constexpr (std::is_same_v<bare_t, const char *> || std::is_same_v<bare_t, char *>)
                return String(value);
            else if constexpr (std::is_same_v<bare_t, std::string>)
                return String(value.c_str());
            else if constexpr (std::is_same_v<bare_t, bool>)
                return value ? "true" : "false";
            else if constexpr (std::is_arithmetic_v<bare_t>)
                return String(value);
            else if constexpr (has_toString<bare_t>::value)
            {
                auto tmp = value.toString();
                if constexpr (std::is_same_v<decltype(tmp), String>)
                    return tmp;
                else
                    return String(tmp.c_str());
            }
            else if constexpr (has_ostream_insertion<bare_t>::value)
            {
                std::ostringstream oss;
                oss << value;
                std::string s = oss.str();
                return String(s.c_str());
            }
            else
            {
                return String("[unprintable]");
            }
        }

        template <typename... Args>
        String ConcatToString(Args &&...args)
        {
            String out;
            ((out += ToStringGeneric(std::forward<Args>(args))), ...);
            return out;
        }

        template <typename, typename = void>
        struct has_toString : std::false_type
        {
        };

        template <typename T>
        struct has_toString<T, std::void_t<decltype(std::declval<T>().toString())>> : std::true_type
        {
        };

        template <typename, typename = void>
        struct has_ostream_insertion : std::false_type
        {
        };

        template <typename T>
        struct has_ostream_insertion<T, std::void_t<decltype(std::declval<std::ostream &>() << std::declval<T>())>> : std::true_type
        {
        };
    };
    inline ConsoleProvider ___SerialConsole___;
}
#define Console Util::___SerialConsole___
