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
        Buzzer(byte pin)
            : pin(pin),
              currentTone(0),
              toneEndTime(0),
              lastToggleMicros(0),
              halfPeriodMicros(0),
              pinState(false),
              volume(255),
              activeTrack(nullptr)
        {
            pinMode(pin, OUTPUT);
            digitalWrite(pin, LOW);
            noTone(pin);
        }

        inline void Tick() override
        {
            unsigned long nowMillis = millis();

            if (activeTrack && activeTrack->playing)
            {
                if (nowMillis >= activeTrack->noteEndTime)
                {
                    activeTrack->index++;

                    if (activeTrack->index >= activeTrack->length)
                    {
                        StopTrack();
                        return;
                    }

                    PlayNextNote();
                }

                return;
            }

            if (currentTone > 0)
            {
                if (nowMillis >= toneEndTime)
                {
                    StopTone();
                    return;
                }
            }
        }

        inline void PlayTone(int frequency, unsigned long duration)
        {
            if (frequency <= 0 || duration == 0)
            {
                StopTone();
                return;
            }

            currentTone = frequency;
            toneEndTime = millis() + duration;

            tone(pin, frequency);

            if (activeTrack && activeTrack->playing)
            {
                activeTrack->playing = false;
                activeTrack.reset();
            }
        }

        inline void StopTone()
        {
            currentTone = 0;
            toneEndTime = 0;

            noTone(pin);
            digitalWrite(pin, LOW);
        }

        inline void PlayTrack(const uint16_t (&frequencies)[],
                              const uint16_t (&durations)[],
                              size_t length)
        {
            StopTone();

            activeTrack = std::make_unique<Track>(frequencies, durations, length);

            if (activeTrack->length == 0)
            {
                activeTrack->playing = false;
                activeTrack.reset();
                return;
            }

            activeTrack->index = 0;
            activeTrack->playing = true;

            PlayNextNote();
        }

        inline void StopTrack()
        {
            if (activeTrack)
            {
                activeTrack->playing = false;
                activeTrack.reset();
            }

            StopTone();
        }

        inline bool IsPlayingTrack() const
        {
            return activeTrack && activeTrack->playing;
        }

        inline void SetVolume(byte vol)
        {
            volume = vol;
        }

        inline byte GetVolume() const
        {
            return volume;
        }

    private:
        byte pin;
        int currentTone;
        unsigned long toneEndTime;
        unsigned long lastToggleMicros;
        unsigned int halfPeriodMicros;
        bool pinState;
        byte volume;

        void PlayNextNote()
        {
            if (!activeTrack || !activeTrack->playing)
                return;

            if (activeTrack->index >= activeTrack->length)
            {
                StopTrack();
                return;
            }

            uint16_t freq = activeTrack->frequencies[activeTrack->index];
            uint16_t dur = activeTrack->durations[activeTrack->index];

            if (freq == 0)
            {
                currentTone = 0;
                noTone(pin);
            }
            else
            {
                currentTone = freq;
                tone(pin, freq);
            }

            activeTrack->noteEndTime = millis() + dur;
        }

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
                : frequencies(nullptr),
                  durations(nullptr),
                  length(len),
                  index(0),
                  noteEndTime(0),
                  playing(true)
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
