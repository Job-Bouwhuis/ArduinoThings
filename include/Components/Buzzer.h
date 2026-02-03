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
        void PlayTrack(const short *frequencies,
                       const short *durations,
                       size_t length);
        void StopTrack();
        bool IsPlayingTrack() const;

    private:
        byte pin;
        int currentTone;
        unsigned long toneEndTime;

        void PlayNextNote();

        struct Track
        {
        public:
            short *frequencies;
            short *durations;
            size_t length;

            size_t index;
            unsigned long noteEndTime;
            bool playing;

            Track(const short *freqs, const short *durs, size_t len)
                : frequencies(nullptr), durations(nullptr),
                  length(len), index(0), noteEndTime(0), playing(true)
            {
                if (length > 0)
                {
                    frequencies = new short[length];
                    durations = new short[length];
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
