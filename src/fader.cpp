/********************************************************************************
 *  This is a battery charge monitor for lead acid batteries that:
 *   - detects if 240V is available at the input side of the charger and
 *   - measures battery voltage across 2 channels
 *
 *  Written by:  Dave Skinner
 *  Date:        16 December 2024
 *
 ********************************************************************************/

#include "fader.h"

Fader::Fader(uint8_t outputPin, bool outputInverted)
    : Channel(outputPin, outputInverted) {};

void Fader::Setup()
{
    pinMode(_outputPin, OUTPUT);
    LedOff();
}

void Fader::StartFade()
{
    _runFade = true;
    _pwmFadeAmount = OUTPUT_PWM_FADE_STEP;
    _pwmBrightness = _pwmFadeAmount;
    _pwmCycleMillis = millis() + OUTPUT_PWM_CYCLE_MILLIS;
    _pwmStepMillis = millis() + OUTPUT_PWM_FADE_MILLIS;
    analogWrite(_outputPin, _outputInverted ? 255 - _pwmBrightness : _pwmBrightness);
};

void Fader::StopFade()
{
    _runFade = false;
}

void Fader::RunFade()
{
    if (_pwmBrightness > 0)
    {
        if (millis() >= _pwmStepMillis)
        {
            // set the brightness (note PWM brightness is inverted)
            _pwmBrightness += _pwmFadeAmount;
            _pwmStepMillis += OUTPUT_PWM_FADE_MILLIS;
            analogWrite(_outputPin, _outputInverted ? 255 - _pwmBrightness : _pwmBrightness);

            // reverse at the top of the cycle
            if (_pwmBrightness >= 255)
                _pwmFadeAmount = -_pwmFadeAmount;
        }
    }
    else
    {
        // at the bottom of the fade, wait until the next cycle
        LedOff();
        if (_runFade && millis() >= _pwmCycleMillis)
            StartFade();
    }
};
