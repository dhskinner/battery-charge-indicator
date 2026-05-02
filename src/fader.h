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
#ifndef _FADER_H_
#define _FADER_H_

#include "channel.h"

class Fader : public Channel
{
protected:
    // state (variables)
    bool _runFade = false;
    int _pwmBrightness = 0;
    int _pwmFadeAmount = 0;
    unsigned long _pwmStepMillis = 0;
    unsigned long _pwmCycleMillis = 0;

public:
    // constructor
    Fader(uint8_t outputPin, bool outputInverted = false);

    // called once on start to initialise pinouts
    void Setup() override;

    // called to restart a fade cycle (from LED fully off)
    void StartFade();

    // called to gracefully stop fading at the next cycle (until LED fully off)
    void StopFade();

    // called frequently when LED fade is required (non blocking)
    void RunFade();

private:
    // change the visibility of some methods to hide them in the child
    // class - this is fairly contrived, and breaks casting the child
    // back to the parent class, but its just an example of whats do-able
    using Channel::IsAcPresent;
    using Channel::IsDcPresent;
};

#endif