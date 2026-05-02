/********************************************************************************
 *  This is a battery charge monitor for lead acid batteries that:
 *   - detects if 240V is available at the input side of the charger and
 *   - measures battery voltage across 2 channels
 *
 *  Written by:  Dave Skinner
 *  Date:        16 December 2024
 *
 ********************************************************************************

To burn the bootloader to a fresh Attiny:

  1) connect the ICSP header from USBAsp to the target board

      ATTiny84A Expander Daughter Board (purple)
      -----------------------------------------
                  -----
      pin 1 MISO |*    | pin 2 5V+
      pin 3 SCK  |     | pin 4 MOSI
      pin 5 RST  |     | pin 6 GND
                  -----

  2) Setup USBAsp board:
      - jumper the "slow clock" header
      - select "5V" on the target voltage selection switch
      - jumper the 'target power' header

      Refer to here for a guide to USBAsp https://www.freetronics.com.au/pages/usbasp-icsp-programmer-quickstart-guide

  3) setup the following options (this is easiest in the Arduino IDE):
      - Board:              Attiny 24/44/84a (no bootloader)
      - B.O.D. Level:       Disabled (brownout detection)
      - Chip:               Attiny84(a)
      - Clock Source:       8MHz (internal)
      - Save EEPROM:        EEPROM not retained
      - LTO:                Enabled
      - millis()/micros():  Enabled
      - tinyNeoPixel:       Port A
      - Pin mapping:        Clockwise (!! IMPORTANT !!)
      - Programmer:         USBAsp (ATTiny Core)

  4) Burn bootloader and check all is ok i.e. 2 bytes written

  5) On the USBAsp board, remove the "Slow Clock" jumper

  6) For a simple proof-of-life test either:
      - run the Arduino 'Blink' sketch using pin 7,8,9 or 10 (see "pintest.h" for LED pinouts)
      - uncomment #define PIN_TEST_MODE below to see all four LED's blinking at the same time
      - run this sample in the Arduino IDE:

          int led_pin[4] = {7, 8, 9, 10}, led_index = 0;
          void setup() {
            for (; led_index < 4; led_index++) {
              pinMode(led_pin[led_index], OUTPUT);
              digitalWrite(led_pin[led_index], LOW);
          }}
          void loop() {
            led_index = led_index >= 3 ? 0 : led_index + 1;
            digitalWrite(led_pin[led_index], HIGH); delay(100);
            digitalWrite(led_pin[led_index], LOW); delay(100);
          }

  7) Upload the production program code as normal using Arduino IDE or PlatformIO and USBAsp connected via ICSP
     For PlatformIO:
        - go to the PlatformIO tab in VSCode and 'Open" this project
        - when config is complete, go to the PIO menu bar to the left of the VSCode working area
        - the menu for "Project Tasks" should be visible - hit "Upload"

  If the Attiny is bricked refer to https://github.com/tsaarni/avr-high-voltage-serial-programming
*/

// Uncomment the following line for a simple test program that cycles a couple of LED's:
// #define PIN_TEST_MODE

#include "channel.h"
#include "fader.h"
#include "pintest.h"

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
#ifdef PIN_TEST_MODE
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
}

// run repeatedly
void loop()
{
#if defined(CALIBRATION_MODE)

  // calibration mode takes readings from each ADC and blinks out the values
  channelMains.IsAcPresent();
  channelPrimary.Calibrate();
  channelSecondary.Calibrate();

#elif defined(PIN_TEST_MODE)

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
