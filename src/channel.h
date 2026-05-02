/********************************************************************************
 *  This is a battery charge monitor for lead acid batteries that:
 *   - detects if 240V is available at the input side of the charger and
 *   - measures battery voltage across 2 channels
 *
 *  Written by:  Dave Skinner
 *  Date:        16 December 2024
 *
 ********************************************************************************/

#pragma once
#ifndef _CHANNEL_H_
#define _CHANNEL_H_

#include "config.h"

enum class State
{
    Off = 0,
    Debounce = 1,
    On = 2
};

class Channel
{
protected:
    // configuration (const)
    const uint8_t _sensePin = 0;
    const float _senseThreshold = 0.0f;
    const float _senseScale = 1.0f;
    const uint8_t _outputPin = 0;
    const bool _outputInverted = false;

    // state (variables)
    State _isPresent = State::Off;
    bool _lastReading = false;
    unsigned long _firstLeadingEdgeMillis = 0;
    unsigned long _lastLeadingEdgeMillis = 0;

public:
    // constructor
    Channel(uint8_t sensePin,
            float senseThreshold,
            float senseScale,
            uint8_t outputPin,
            bool outputInverted = false);

    // constructor
    Channel(uint8_t outputPin, bool outputInverted);

    // called once on start to initialise pinouts
    virtual void Setup();

    // called to turn the LED for this channel on (may be inverted)
    void LedOn();

    // called to turn the LED for this channel off (may be inverted)
    void LedOff();

    // called to flash the LED n times (blocking)
    void Flash(unsigned long duration = 100, uint8_t flashes = 3);

    // detect whether DC is present, above the specified threshold
    bool IsDcPresent(bool ledOnIfPresent = true);

    // detect whether 50Hz AC signal is present (this is only intended to work on the mains channel)
    // - detect leading edge of when a signal first arrives
    // - measure the next leading edge and check if its between 15 and 25 milliseconds (40Hz to 66Hz)
    // - if we get successive measurements for the whole debounce time, then 50Hz is present
    bool IsAcPresent(bool ledOnIfPresent = true);

#ifdef ENABLE_CALIBRATION_MODE

    // enable calibration mode to check the primary and secondary sense channels
    // using an external power source and multimeter to apply zero to say 16V in
    // steps across the input, and read off the ADC measurement (0 to 1023). The
    // original board had full scale from approx 0 to 21.0V through the resistor
    // divider and schottky diode.
    void Calibrate();

#endif
};

#endif