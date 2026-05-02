/********************************************************************************
 *  This is a battery charge monitor for lead acid batteries that:
 *   - detects if 240V is available at the input side of the charger and
 *   - measures battery voltage across 2 channels
 *
 *  Written by:  Dave Skinner
 *  Date:        16 December 2024
 *
 ********************************************************************************/

#include "channel.h"

Channel::Channel(uint8_t sensePin,
                 float senseThreshold,
                 float senseScale,
                 uint8_t outputPin,
                 bool outputInverted)
    : _sensePin(sensePin),
      _senseThreshold(senseThreshold),
      _senseScale(senseScale),
      _outputPin(outputPin),
      _outputInverted(outputInverted) {
      };

Channel::Channel(uint8_t outputPin, bool outputInverted)
    : _outputPin(outputPin),
      _outputInverted(outputInverted) {};

void Channel::Setup()
{
    pinMode(_sensePin, INPUT);
    pinMode(_outputPin, OUTPUT);
    LedOff();
}

void Channel::LedOn()
{
    digitalWrite(_outputPin, _outputInverted ? LOW : HIGH);
}

void Channel::LedOff()
{
    digitalWrite(_outputPin, _outputInverted ? HIGH : LOW);
}

bool Channel::IsDcPresent(bool ledOnIfPresent)
{
    if (_senseThreshold == HIGH || _senseThreshold == LOW)
    {
        // check for changes in signal presence using digital IO
        _isPresent = digitalRead(_sensePin) == _senseThreshold ? State::On : State::Off;
    }
    else
    {
        // check for changes in signal presence using analog measurement
        int senseValue = analogRead(_sensePin);

        // convert the analog reading (from 0 to 1023 where 1023 = VRef) to a voltage
        float voltage = (float)senseValue * ((_senseScale - SCHOTTKY_FORWARD_VOLTS) / 1023.0) + SCHOTTKY_FORWARD_VOLTS;
        _isPresent = voltage >= _senseThreshold ? State::On : State::Off;
        // TODO
        // this was the threshold for the first board primary channel at around 12v
        // present = senseValue >= 572 ? State::On : State::Off;
    }

    // if voltage is present turn on the LED
    if (ledOnIfPresent && _isPresent == State::On)
        LedOn();
    else
        LedOff();

    return _isPresent == State::On;
};

bool Channel::IsAcPresent(bool ledOnIfPresent)
{
    // read the current pin state
    bool reading = digitalRead(_sensePin) == _senseThreshold ? true : false;
    unsigned long now = millis();
    bool acPresent = false;

    // is this a leading edge?
    if (_lastReading == false && reading == true)
    {
        // is this leading edge about 50Hz from the previous?
        unsigned long elapsed = now - _lastLeadingEdgeMillis;
        _lastLeadingEdgeMillis = now;
        acPresent = (elapsed >= MAINS_50Hz_MINIMUM_MILLIS && elapsed <= MAINS_50Hz_MAXIMUM_MILLIS);
    }
    _lastReading = reading;

    // use a baby fsm to debounce the signal
    switch (_isPresent)
    {
    case State::On:

        // AC is present turn on the LED
        if (ledOnIfPresent)
            LedOn();

        // check if leading edges have dropped out
        if (now - _lastLeadingEdgeMillis >= MAINS_50HZ_HYSTERISIS_MS)
            _isPresent = State::Off;

        break;

    case State::Debounce:

        if (now - _firstLeadingEdgeMillis >= MAINS_50HZ_HYSTERISIS_MS)
            _isPresent = State::On;

        break;

    case State::Off:
    default:

        // turn off the LED
        if (ledOnIfPresent)
            LedOff();

        // if 50Hz is initially detected, start the debounce timer
        if (acPresent)
        {
            _firstLeadingEdgeMillis = now;
            _isPresent = State::Debounce;
        }
    }

    return _isPresent == State::On;
}

void Channel::Flash(unsigned long duration, uint8_t flashes)
{
    for (int i = 0; i < flashes; i++)
    {
        LedOn();
        delay(duration);
        LedOff();
        delay(duration);
    }
};

#ifdef CALIBRATION_MODE

// enable calibration mode to check the primary and secondary sense channels
// using an external power source and multimeter to apply zero to say 16V in
// steps across the input, and read off the ADC measurement (0 to 1023). The
// original board had full scale from approx 0 to 21.0V through the resistor
// divider and schottky diode.
void Channel::Calibrate()
{
    // take some samples
    int reading = 0;
    int samples = 5;
    for (int i = 0; i < samples; i++)
    {
        reading += analogRead(_sensePin);
        delay(50);
    }
    reading /= samples;

    // check for zero
    if (reading == 0)
    {
        LedOn();
        delay(START_DELAY_MS);
        LedOff();
        delay(START_FLASH_MS);
    }

    // find the highest order of magnitude
    int magnitude = 10000;
    while (magnitude > reading)
        magnitude /= 10;

    // blink the adc value 0 to 1023
    while (magnitude > 0)
    {
        // get the most significant digit
        int digit = (int)reading / magnitude;

        // blink out the digits, or a long flash for zero
        if (digit == 0)
        {
            LedOn();
            delay(START_DELAY_MS);
            LedOff();
            delay(START_FLASH_MS);
        }
        else
        {
            for (int i = 0; i < digit; i++)
            {
                LedOn();
                delay(START_FLASH_MS);
                LedOff();
                delay(START_FLASH_MS);
            }
        }
        // reduce the reading by an order of magnitude
        reading = reading % magnitude;
        magnitude /= 10;
        delay(START_DELAY_MS);
    }
    delay(START_DELAY_MS);
}

#endif
