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
            return;

        unsigned long now = millis();

        if (now < activeTrack->noteEndTime)
            return;

        // debug
        Serial.printf("new: %lu - nextNoteEnd: %lu\n", now, activeTrack->noteEndTime);

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

    void Buzzer::PlayTrack(const short *frequencies,
                           const short *durations,
                           size_t length)
    {
        // stop & free previous track if any
        StopTrack();

        if (length == 0 || frequencies == nullptr || durations == nullptr)
            return;

        // allocate and copy into a new heap Track (Buzzer is friend so it can call private ctor)
        activeTrack = std::unique_ptr<Track>(new Track(frequencies, durations, length));
        // start playback immediately
        PlayNextNote();
    }

    void Buzzer::StopTrack()
    {
        if (!activeTrack)
            return;

        noTone(pin);
        // destroy the track (calls Track::~Track which deletes arrays)
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
            noTone(pin);
            activeTrack->playing = false;
            // free track once finished
            activeTrack.reset();
            return;
        }

        short freq = activeTrack->frequencies[activeTrack->index];
        short duration = activeTrack->durations[activeTrack->index];

        Serial.printf("Playing freq: %d for %dms\n", freq, duration);

        if (freq > 0)
            tone(pin, freq);
        else
            noTone(pin); // rest

        activeTrack->noteEndTime = millis() + static_cast<unsigned long>(duration);
        activeTrack->index++;
    }
}
