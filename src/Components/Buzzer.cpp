#include "Components/Buzzer.h"

namespace Components
{
    Buzzer::Buzzer(byte pin)
        : pin(pin), currentTone(0), toneEndTime(0), activeTrack(nullptr)
    {
        pinMode(pin, OUTPUT);
    }

    void Buzzer::Tick()
    {
        if (!activeTrack || !activeTrack->playing)
        {
            noTone(pin);
            return;
        }

        unsigned long now = millis();

        if (now < activeTrack->noteEndTime)
            return;

        PlayNextNote();
    }

    void Buzzer::PlayTone(int frequency, unsigned long duration)
    {
        if (frequency <= 0)
            return;

        tone(pin, frequency);
        currentTone = frequency;
        toneEndTime = millis() + duration;
    }

    void Buzzer::StopTone()
    {
        noTone(pin);
        currentTone = 0;
        toneEndTime = 0;
    }

    void Buzzer::PlayTrack(const uint16_t (&frequencies)[],
                           const uint16_t (&durations)[],
                           size_t length)
    {
        StopTrack();

        if (length == 0)
            return;

        activeTrack = std::make_unique<Track>(frequencies, durations, length);
        activeTrack->index = 0;
        activeTrack->playing = true;

        PlayNextNote();
    }

    void Buzzer::StopTrack()
    {
        if (!activeTrack)
            return;

        noTone(pin);
        // destroy current track object
        activeTrack.reset();
    }

    bool Buzzer::IsPlayingTrack() const
    {
        return activeTrack && activeTrack->playing;
    }

    void Buzzer::PlayNextNote()
    {
        if (!activeTrack)
            return;

        if (activeTrack->index >= activeTrack->length)
        {
            Serial.println("Track complete!");
            noTone(pin);
            StopTrack();
            return;
        }

        const uint16_t freq = activeTrack->frequencies[activeTrack->index];
        const uint16_t dur = activeTrack->durations[activeTrack->index];

        if (freq > 0)
        {

            if (dur == 0)
            {
                noTone(pin);
                Serial.printf("No duration note: %d. playing for 1ms", freq);
                activeTrack->noteEndTime = millis() + 1;
            }
            else
            {
                Serial.printf("Freq: %d for %dms\n", freq, dur);
                tone(pin, freq, dur);
                activeTrack->noteEndTime = millis() + dur;
            }
        }
        else
        {
            noTone(pin);
            activeTrack->noteEndTime = millis() + dur;
            Serial.printf("Restnote for %dms\n", dur);
        }

        ++activeTrack->index;
    }
}
