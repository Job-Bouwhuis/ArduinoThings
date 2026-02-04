#pragma once
#include <Arduino.h>
#include "Component.h"
#include <cstddef>
#include <memory>

namespace Components
{
    class Buzzer : public Component
    {
    public:
        Buzzer(byte pin);
        void Tick() override;

        // Single tone interface
        void PlayTone(int frequency, unsigned long duration);
        void StopTone();

        // Track interface (copies data to buzzer-owned heap)
        void PlayTrack(const uint16_t (&frequencies)[],
                       const uint16_t (&durations)[],
                       size_t length);
        void StopTrack();
        bool IsPlayingTrack() const;

        void SetVolume(byte volume); // 0–255
        byte GetVolume() const;

    private:
        byte pin;
        int currentTone;
        unsigned long toneEndTime;
        unsigned long lastToggleMicros;
        unsigned int halfPeriodMicros;
        bool pinState;
        byte volume;

        void PlayNextNote();

        struct Track
        {
        public:
            uint16_t *frequencies;
            uint16_t *durations;
            size_t length;

            size_t index;
            unsigned long noteEndTime;
            bool playing;

            Track(const uint16_t (&freqs)[], const uint16_t (&durs)[], size_t len)
                : frequencies(nullptr), durations(nullptr),
                  length(len), index(0), noteEndTime(0), playing(true)
            {
                if (length > 0)
                {
                    frequencies = new uint16_t[length];
                    durations = new uint16_t[length];
                    for (size_t i = 0; i < length; ++i)
                    {
                        frequencies[i] = freqs[i];
                        durations[i] = durs[i];
                    }
                }
            }

            Track(const Track &) = delete;
            Track &operator=(const Track &) = delete;

            ~Track()
            {
                delete[] frequencies;
                delete[] durations;
            }
        };

        std::unique_ptr<Track> activeTrack;
    };
}
