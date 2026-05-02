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
#include "fader.h"
#include "pintest.h"
#include "watchdog.h"

Channel channelPrimary(CHANNEL_PRI_SENSE_PIN, CHANNEL_PRI_THRESHOLD, CHANNEL_PRI_SCALE, CHANNEL_PRI_LED_PIN);
Channel channelSecondary(CHANNEL_SEC_SENSE_PIN, CHANNEL_SEC_THRESHOLD, CHANNEL_SEC_SCALE, CHANNEL_SEC_LED_PIN);
Channel channelMains(CHANNEL_MAINS_SENSE_PIN, CHANNEL_MAINS_THRESHOLD, CHANNEL_MAINS_SCALE, CHANNEL_MAINS_LED_PIN);
Fader channelOutput(OUTPUT_PWM_FADE_PIN);
bool charging = false;
bool IsCharging();

// --------------------------------------------------------------------------------

// run once
void setup()
{
#ifdef ENABLE_PIN_TEST_MODE
  pinTestSetup();
#else
  // setup sense channels
  channelPrimary.Setup();
  channelSecondary.Setup();
  channelMains.Setup();

  // setup the output channel
  channelOutput.Setup();

  // test the onboard leds
  channelPrimary.Flash(START_FLASH_MS, START_FLASHES);
  channelSecondary.Flash(START_FLASH_MS, START_FLASHES);
  channelMains.Flash(START_FLASH_MS, START_FLASHES);
  channelOutput.Flash(START_FLASH_MS, START_FLASHES);
  delay(START_DELAY_MS);
#endif

#ifdef ENABLE_WATCHDOG_TIMER
  indicateWatchdogReset();
  watchdogInit();

#ifdef ENABLE_WATCHDOG_LOCKUP_TEST
  // Force a one-shot lockup test: on non-watchdog boot, turn on green output LED and hang.
  // After watchdog reset, this block is skipped so normal startup can continue.
  if (!watchdogWasReset())
  {
    pinMode(OUTPUT_PWM_FADE_PIN, OUTPUT);
    digitalWrite(OUTPUT_PWM_FADE_PIN, HIGH);
    while (true)
    {
      // intentional lockup to validate watchdog reset behavior
    }
  }
#endif

#endif
}

// run repeatedly
void loop()
{
#ifdef ENABLE_WATCHDOG_TIMER
  watchdogFeed();
#endif

#if defined(ENABLE_CALIBRATION_MODE)

  // calibration mode takes readings from each ADC and blinks out the values
  channelMains.IsAcPresent();
  channelPrimary.Calibrate();
  channelSecondary.Calibrate();

#elif defined(ENABLE_PIN_TEST_MODE)

  // pin test mode is to check basic operation of the LED's and outputs
  pinTestRun();

#else

  // normal operation checks for charging present
  if (!charging && IsCharging())
  {

    // charging is detected so start pwm fade cycle
    channelOutput.StartFade();
  }
  else if (charging && !IsCharging())
  {

    // charging has dropped so stop the pwm fade cycle
    channelOutput.StopFade();
  }

  // run the PWM fade cycle
  channelOutput.RunFade();

#endif
}

// --------------------------------------------------------------------------------

// the logic for charging to be considered present is there must be:
// - 240V detected (mains channel)
// - charge voltage on the battery terminals (primary channel)
// - house master is off (secondary channel)
bool IsCharging()
{
  static bool lastDebounceState = false;
  static unsigned long lastDebounceTime = 0;

  // read the current charging state - remember you can't do this
  // all in one statement because of lazy evaluation (ffs)
  bool reading = channelMains.IsAcPresent();
  reading &= channelPrimary.IsDcPresent();
  reading &= !channelSecondary.IsDcPresent();

  // if the state changed, due to noise or actual changes in readings, reset the debounce timer
  if (reading != lastDebounceState)
  {
    lastDebounceState = reading;
    lastDebounceTime = millis();
  }

  // if the state has stable for longer than the debounce delay, take it as current
  if ((millis() - lastDebounceTime) > DEBOUNCE_HYSTERISIS_MS)
    charging = reading;

  return charging;
}
