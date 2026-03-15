#pragma once
#include <Arduino.h>
#include "Component.h"
#include <cstddef>
#include <memory>

#define ULONG_MAX 4294967295
#define MUSIC_VOICES 1
#define EFFECT_VOICES 5
#define MAX_VOICES (MUSIC_VOICES + EFFECT_VOICES)
#define DEFAULT_MUSIC_VOLUME 96
#define DEFAULT_EFFECT_VOLUME 255

namespace Components
{
    class Buzzer : public Component
    {
    public:
        Buzzer(byte pin)
            : pin(pin),
              musicMasterVolume(DEFAULT_MUSIC_VOLUME),
              effectMasterVolume(DEFAULT_EFFECT_VOLUME)
        {
            pinMode(pin, OUTPUT);
            portOut = portOutputRegister(digitalPinToPort(pin));
            pinMask = digitalPinToBitMask(pin);
            *portOut &= ~pinMask;
            outputState = false;

            for (size_t i = 0; i < MAX_VOICES; ++i)
            {
                tones[i].active = false;
                tones[i].frequency = 0;
                tones[i].periodMicros = 0;
                tones[i].lastToggle = 0;
                tones[i].state = false;
                tones[i].endTimeMs = 0;
                tones[i].startTimeMs = 0;
                tones[i].volume = 0;
            }
        }

        void Tick() override
        {
            unsigned long nowMicros = micros();
            unsigned long nowMillis = millis();

            if (activeMusicTrack && activeMusicTrack->playing)
            {
                if (nowMillis >= activeMusicTrack->noteEndTimeMs)
                {
                    PlayNextMusicNote();
                }
            }

            // expire tones
            for (size_t i = 0; i < MAX_VOICES; ++i)
            {
                Tone &t = tones[i];
                if (!t.active)
                    continue;

                if (t.endTimeMs != 0 && nowMillis >= t.endTimeMs)
                {
                    t.active = false;
                    t.frequency = 0;
                    t.periodMicros = 0;
                    t.state = false;
                }
            }

            // select single tone to play (effects override background)
            size_t selectedIndex = MAX_VOICES;
            for (size_t i = MUSIC_VOICES; i < MAX_VOICES; ++i)
            {
                Tone &t = tones[i];
                if (!t.active)
                    continue;
                if (selectedIndex == MAX_VOICES)
                    selectedIndex = i;
                else
                {
                    Tone &best = tones[selectedIndex];
                    if (t.volume > best.volume ||
                        (t.volume == best.volume && t.startTimeMs > best.startTimeMs))
                    {
                        selectedIndex = i;
                    }
                }
            }

            // fallback to music tone
            if (selectedIndex == MAX_VOICES)
            {
                if (tones[0].active)
                    selectedIndex = 0;
            }

            bool newState = false;

            if (selectedIndex != MAX_VOICES)
            {
                Tone &t = tones[selectedIndex];

                if (t.periodMicros > 0)
                {
                    unsigned long half = t.periodMicros >> 1;
                    while ((nowMicros - t.lastToggle) >= half)
                    {
                        t.lastToggle += half;
                        t.state = !t.state;
                    }
                }

                newState = (t.state && t.volume > 0);
            }

            if (newState != outputState)
            {
                if (newState)
                    *portOut |= pinMask;
                else
                    *portOut &= ~pinMask;

                outputState = newState;
            }
        }

        void PlayBackgroundTone(int frequency, unsigned long durationMs, byte relativeVolume = 255)
        {
            if (frequency <= 0)
                return;

            Tone &t = tones[0]; // background tone is always index 0
            t.active = true;
            t.frequency = frequency;
            t.periodMicros = (frequency > 0) ? (1000000UL / (unsigned long)frequency) : 0;
            t.lastToggle = (t.periodMicros > 0) ? (micros() - (t.periodMicros >> 1)) : micros();
            t.state = false;
            t.endTimeMs = (durationMs > 0) ? (millis() + durationMs) : 0;
            t.startTimeMs = millis();

            unsigned int scaled = (unsigned int)relativeVolume * (unsigned int)musicMasterVolume;
            t.volume = (byte)(scaled >> 8);
            if (t.volume == 0 && scaled > 0)
                t.volume = 1;
        }

        void PlayEffectTone(int frequency, unsigned long durationMs, byte relativeVolume = 255)
        {
            if (frequency <= 0)
                return;

            // find free effect slot
            for (size_t i = MUSIC_VOICES; i < MAX_VOICES; ++i)
            {
                Tone &t = tones[i];
                if (!t.active)
                {
                    t.active = true;
                    t.frequency = frequency;
                    t.periodMicros = (frequency > 0) ? (1000000UL / (unsigned long)frequency) : 0;
                    t.lastToggle = (t.periodMicros > 0) ? (micros() - (t.periodMicros >> 1)) : micros();
                    t.state = false;
                    t.endTimeMs = (durationMs > 0) ? (millis() + durationMs) : 0;
                    t.startTimeMs = millis();

                    unsigned int scaled = (unsigned int)relativeVolume * (unsigned int)effectMasterVolume;
                    t.volume = (byte)(scaled >> 8);
                    if (t.volume == 0 && scaled > 0)
                        t.volume = 1;
                    return;
                }
            }

            // steal oldest effect if needed
            size_t stealIndex = MUSIC_VOICES;
            unsigned long oldestEnd = ULONG_MAX;
            for (size_t i = MUSIC_VOICES; i < MAX_VOICES; ++i)
            {
                if (tones[i].endTimeMs < oldestEnd)
                {
                    oldestEnd = tones[i].endTimeMs;
                    stealIndex = i;
                }
            }

            Tone &t = tones[stealIndex];
            t.active = true;
            t.frequency = frequency;
            t.periodMicros = (frequency > 0) ? (1000000UL / (unsigned long)frequency) : 0;
            t.lastToggle = (t.periodMicros > 0) ? (micros() - (t.periodMicros >> 1)) : micros();
            t.state = false;
            t.endTimeMs = (durationMs > 0) ? (millis() + durationMs) : 0;
            t.startTimeMs = millis();

            unsigned int scaled = (unsigned int)relativeVolume * (unsigned int)effectMasterVolume;
            t.volume = (byte)(scaled >> 8);
            if (t.volume == 0 && scaled > 0)
                t.volume = 1;
        }

        void StopAll()
        {
            for (size_t i = 0; i < MAX_VOICES; ++i)
                tones[i].active = false;

            StopMusicTrack();
            digitalWrite(pin, LOW);
        }

        void PlayMusicTrack(const uint16_t (&frequencies)[], const uint16_t (&durations)[], size_t length, bool loop = false)
        {
            StopMusicTrack();
            activeMusicTrack = std::make_unique<Track>(frequencies, durations, length, loop);
            if (activeMusicTrack->length > 0)
            {
                activeMusicTrack->index = 0;
                PlayNextMusicNote();
            }
        }

        void StopMusicTrack()
        {
            if (activeMusicTrack)
            {
                activeMusicTrack->playing = false;
                activeMusicTrack.reset();
            }

            for (size_t i = 0; i < MUSIC_VOICES; ++i)
            {
                tones[i].active = false;
            }
        }

        bool IsPlayingMusic() const
        {
            return activeMusicTrack && activeMusicTrack->playing;
        }

        void SetMusicMasterVolume(byte volume) { musicMasterVolume = volume; }
        void SetEffectMasterVolume(byte volume) { effectMasterVolume = volume; }
        byte GetMusicMasterVolume() const { return musicMasterVolume; }
        byte GetEffectMasterVolume() const { return effectMasterVolume; }

    private:
        struct Tone
        {
            bool active;
            int frequency;
            unsigned long periodMicros;
            unsigned long lastToggle;
            bool state;
            unsigned long endTimeMs;
            unsigned long startTimeMs;
            byte volume;
        };

        Tone tones[MAX_VOICES];

        byte pin;
        byte musicMasterVolume;
        byte effectMasterVolume;
        static constexpr uint16_t NOTE_GAP_MS = 14;

        volatile uint32_t *portOut;
        uint8_t pinMask;
        bool outputState = false;

        struct Track
        {
            uint16_t *frequencies;
            uint16_t *durations;
            size_t length;
            size_t index;
            unsigned long noteEndTimeMs;
            bool playing;
            bool loop;

            Track(const uint16_t (&freqs)[], const uint16_t (&durs)[], size_t len, bool loopArg)
                : frequencies(nullptr), durations(nullptr),
                  length(len), index(0), noteEndTimeMs(0), playing(true), loop(loopArg)
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

        std::unique_ptr<Track> activeMusicTrack;

        void PlayNextMusicNote()
        {
            if (!activeMusicTrack || !activeMusicTrack->playing)
                return;

            if (activeMusicTrack->index >= activeMusicTrack->length)
            {
                if (activeMusicTrack->loop)
                    activeMusicTrack->index = 0;
                else
                {
                    activeMusicTrack->playing = false;
                    for (size_t i = 0; i < MUSIC_VOICES; ++i)
                        tones[i].active = false;
                    return;
                }
            }

            uint16_t freq = activeMusicTrack->frequencies[activeMusicTrack->index];
            uint16_t dur = activeMusicTrack->durations[activeMusicTrack->index];

            if (dur > NOTE_GAP_MS)
                dur -= NOTE_GAP_MS;

            if (freq == 0)
                tones[0].active = false;
            else
                PlayBackgroundTone((int)freq, (unsigned long)dur, 255);

            activeMusicTrack->noteEndTimeMs = millis() + activeMusicTrack->durations[activeMusicTrack->index];
            activeMusicTrack->index++;
        }
    };
}