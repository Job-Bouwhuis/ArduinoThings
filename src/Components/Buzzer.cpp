// Buzzer.cpp
#include "Components/Buzzer.h"

namespace Components
{
    Buzzer::Buzzer(byte pin)
        : pin(pin),
          currentTone(0),
          toneEndTime(0),
          activeTrack(nullptr),
          lastToggleMicros(0),
          halfPeriodMicros(0),
          pinState(false),
          volume(255)
    {
        pinMode(pin, OUTPUT);
        analogWrite(pin, 0);
    }

    void Buzzer::PlayTone(int frequency, unsigned long duration)
    {
        if (frequency <= 0 || duration == 0)
        {
            StopTone();
            return;
        }

        currentTone = frequency;
        halfPeriodMicros = (frequency > 0) ? static_cast<unsigned int>(500000UL / frequency) : 0;
        toneEndTime = millis() + duration;

        lastToggleMicros = micros();
        pinState = false;
        analogWrite(pin, 0);

        if (activeTrack && activeTrack->playing)
        {
            activeTrack->playing = false;
            activeTrack.reset();
        }
    }

    void Buzzer::StopTone()
    {
        currentTone = 0;
        toneEndTime = 0;
        halfPeriodMicros = 0;
        pinState = false;
        analogWrite(pin, 0);
    }

    void Buzzer::PlayTrack(const uint16_t (&frequencies)[],
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

    void Buzzer::StopTrack()
    {
        if (activeTrack)
        {
            activeTrack->playing = false;
            activeTrack.reset();
        }
        StopTone();
    }

    bool Buzzer::IsPlayingTrack() const
    {
        return activeTrack && activeTrack->playing;
    }

    void Buzzer::PlayNextNote()
    {
        if (!activeTrack || !activeTrack->playing)
            return;

        if (activeTrack->index >= activeTrack->length)
        {
            // finished
            StopTrack();
            return;
        }

        uint16_t freq = activeTrack->frequencies[activeTrack->index];
        uint16_t dur = activeTrack->durations[activeTrack->index];

        if (freq == 0)
        {
            currentTone = 0;
            halfPeriodMicros = 0;
            pinState = false;
            analogWrite(pin, 0);
        }
        else
        {
            currentTone = freq;
            halfPeriodMicros = static_cast<unsigned int>(500000UL / currentTone);
            lastToggleMicros = micros();
            pinState = false;
            analogWrite(pin, 0);
        }

        activeTrack->noteEndTime = millis() + dur;
    }

    void Buzzer::Tick()
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

            if (currentTone > 0 && halfPeriodMicros > 0)
            {
                unsigned long nowMicros = micros();
                if ((nowMicros - lastToggleMicros) >= halfPeriodMicros)
                {
                    lastToggleMicros += halfPeriodMicros;
                    pinState = !pinState;
                    if (pinState && volume > 0)
                        analogWrite(pin, volume);
                    else
                        analogWrite(pin, 0);
                }
            }
            else
            {
                analogWrite(pin, 0);
                pinState = false;
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

            if (halfPeriodMicros > 0)
            {
                unsigned long nowMicros = micros();
                if ((nowMicros - lastToggleMicros) >= halfPeriodMicros)
                {
                    lastToggleMicros += halfPeriodMicros;
                    pinState = !pinState;
                    if (pinState && volume > 0)
                        analogWrite(pin, volume);
                    else
                        analogWrite(pin, 0);
                }
            }
        }
        else
        {
            if (pinState)
            {
                pinState = false;
                analogWrite(pin, 0);
            }
        }
    }
}