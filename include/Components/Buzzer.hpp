#pragma once
#include <Arduino.h>
#include "Component.h"
#include <cstddef>
#include <memory>

#define ULONG_MAX 4294967295
namespace Components
{
    class Buzzer : public Component
    {
    public:
        static constexpr size_t MUSIC_VOICES = 1;
        static constexpr size_t EFFECT_VOICES = 5;
        static constexpr size_t MAX_VOICES = MUSIC_VOICES + EFFECT_VOICES;
        static constexpr byte DEFAULT_MUSIC_VOLUME = 96;
        static constexpr byte DEFAULT_EFFECT_VOLUME = 255;

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
                voices[i].active = false;
                voices[i].frequency = 0;
                voices[i].periodMicros = 0;
                voices[i].lastToggle = 0;
                voices[i].state = false;
                voices[i].endTimeMs = 0;
                voices[i].volume = 0;
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

            int mix = 0;
            bool anyActive = false;

            for (size_t i = 0; i < MAX_VOICES; ++i)
            {
                Voice &v = voices[i];

                if (!v.active)
                    continue;

                anyActive = true;

                if (v.endTimeMs != 0 && nowMillis >= v.endTimeMs)
                {
                    v.active = false;
                    v.frequency = 0;
                    v.periodMicros = 0;
                    v.state = false;
                    continue;
                }

                if (v.periodMicros > 0)
                {
                    unsigned long half = v.periodMicros >> 1;
                    while ((nowMicros - v.lastToggle) >= half)
                    {
                        v.lastToggle += half;
                        v.state = !v.state;
                    }
                }

                mix += (v.state ? (int)v.volume : -(int)v.volume);
            }

            bool newState = false;
            if (anyActive)
            {
                newState = (mix > 0);
            }
            else
            {
                newState = false;
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

            size_t idx = 0;
            Voice &v = voices[idx];

            v.active = true;
            v.frequency = frequency;
            v.periodMicros = (frequency > 0) ? (1000000UL / (unsigned long)frequency) : 0;
            v.lastToggle = (v.periodMicros > 0) ? (micros() - (v.periodMicros >> 1)) : micros();
            v.state = false;
            v.endTimeMs = (durationMs > 0) ? (millis() + durationMs) : 0;

            unsigned int scaled = (unsigned int)relativeVolume * (unsigned int)musicMasterVolume;
            v.volume = (byte)(scaled >> 8);
            if (v.volume == 0 && scaled > 0)
                v.volume = 1;
        }

        void PlayEffectTone(int frequency, unsigned long durationMs, byte relativeVolume = 255)
        {
            if (frequency <= 0)
                return;

            for (size_t i = MUSIC_VOICES; i < MAX_VOICES; ++i)
            {
                Voice &v = voices[i];
                if (!v.active)
                {
                    v.active = true;
                    v.frequency = frequency;
                    v.periodMicros = (frequency > 0) ? (1000000UL / (unsigned long)frequency) : 0;
                    v.lastToggle = (v.periodMicros > 0) ? (micros() - (v.periodMicros >> 1)) : micros();
                    v.state = false;
                    v.endTimeMs = (durationMs > 0) ? (millis() + durationMs) : 0;

                    unsigned int scaled = (unsigned int)relativeVolume * (unsigned int)effectMasterVolume;
                    v.volume = (byte)(scaled >> 8);
                    if (v.volume == 0 && scaled > 0)
                        v.volume = 1;
                    return;
                }
            }

            size_t stealIndex = MUSIC_VOICES;
            unsigned long oldestEnd = ULONG_MAX;
            for (size_t i = MUSIC_VOICES; i < MAX_VOICES; ++i)
            {
                if (voices[i].endTimeMs < oldestEnd)
                {
                    oldestEnd = voices[i].endTimeMs;
                    stealIndex = i;
                }
            }

            Voice &sv = voices[stealIndex];
            sv.active = true;
            sv.frequency = frequency;
            sv.periodMicros = (frequency > 0) ? (1000000UL / (unsigned long)frequency) : 0;
            sv.lastToggle = (sv.periodMicros > 0) ? (micros() - (sv.periodMicros >> 1)) : micros();
            sv.state = false;
            sv.endTimeMs = (durationMs > 0) ? (millis() + durationMs) : 0;

            unsigned int scaled = (unsigned int)relativeVolume * (unsigned int)effectMasterVolume;
            sv.volume = (byte)(scaled >> 8);
            if (sv.volume == 0 && scaled > 0)
                sv.volume = 1;
        }

        void StopAll()
        {
            for (size_t i = 0; i < MAX_VOICES; ++i)
                voices[i].active = false;

            StopMusicTrack();
            digitalWrite(pin, LOW);
        }

        // music track API (background)
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

            // stop music voice(s)
            for (size_t i = 0; i < MUSIC_VOICES; ++i)
            {
                voices[i].active = false;
            }
        }

        bool IsPlayingMusic() const
        {
            return activeMusicTrack && activeMusicTrack->playing;
        }

        // volume controls
        void SetMusicMasterVolume(byte volume)
        {
            musicMasterVolume = volume;
        }

        void SetEffectMasterVolume(byte volume)
        {
            effectMasterVolume = volume;
        }

        byte GetMusicMasterVolume() const { return musicMasterVolume; }
        byte GetEffectMasterVolume() const { return effectMasterVolume; }

    private:
        struct Voice
        {
            bool active;
            int frequency;
            unsigned long periodMicros;
            unsigned long lastToggle;
            bool state;
            // 0 = infinite
            unsigned long endTimeMs;
            byte volume;
        };

        Voice voices[MAX_VOICES];

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
                {
                    activeMusicTrack->index = 0;
                }
                else
                {
                    activeMusicTrack->playing = false;
                    for (size_t i = 0; i < MUSIC_VOICES; ++i)
                        voices[i].active = false;
                    return;
                }
            }

            uint16_t freq = activeMusicTrack->frequencies[activeMusicTrack->index];
            uint16_t dur = activeMusicTrack->durations[activeMusicTrack->index];

            if (dur > NOTE_GAP_MS)
                dur -= NOTE_GAP_MS;

            if (freq == 0)
            {
                // rest note
                voices[0].active = false;
            }
            else
            {
                PlayBackgroundTone((int)freq, (unsigned long)dur, 255);
            }

            activeMusicTrack->noteEndTimeMs = millis() + activeMusicTrack->durations[activeMusicTrack->index];
            activeMusicTrack->index++;
        }
    };
}