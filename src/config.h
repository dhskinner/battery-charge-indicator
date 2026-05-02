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
#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "Arduino.h"

// set the target processor - either TARGET_PROCESSOR_ATTINY85 or TARGET_PROCESSOR_ATTINY84
#define TARGET_PROCESSOR_ATTINY84

// uncomment to enable calibration mode to check the primary and secondary sense channels
// #define CALIBRATION_MODE

// uncomment to enable pin test mode to check we have the right mapping
// #define PIN_TEST_MODE

#define START_FLASHES 1              // number of times to flash each LED on startup
#define START_FLASH_MS 200           // duration to flash LEDs on startup
#define START_DELAY_MS 1000          // delay on startup
#define DEBOUNCE_HYSTERISIS_MS 1000  // milliseconds to debounce state changes
#define MAINS_50Hz_MINIMUM_MILLIS 15 // lower threshold for detecting 50Hz (15ms = 67Hz)
#define MAINS_50Hz_MAXIMUM_MILLIS 25 // upper threshold for detecting 50Hz (20ms = 40Hz)
#define MAINS_50HZ_HYSTERISIS_MS 200 // time to wait for continuous 50Hz (200msec is about 10 zero crossings at consistent 50Hz)
#define SCHOTTKY_FORWARD_VOLTS 0.13f // forward voltage drop of the schottky diode inline in pri and sec channel sense

#ifdef TARGET_PROCESSOR_ATTINY84
/*
ATMEL ATTINY84A / ARDUINO - CLOCKWISE PINS  !!!! IMPORTANt THIS IS SET CORRECTLY IN THE BOOTLOADER !!!
                                             +-\/-+
                                       VCC  1|    |14  GND
            LED 2 |            (D 10)  PB0  2|    |13  PA0  (D  0)        AREF | Sense Main
            LED 1 |            (D  9)  PB1  3|    |12  PA1  (D  1)             | Serial Tx
            RESET |            (D 11)  PB3  4|    |11  PA2  (D  2)             | Sense Primary
           Output | PWM  INT0  (D  8)  PB2  5|    |10  PA3  (D  3)             | Sense Secondary
         LED Main | PWM        (D  7)  PA7  6|    |9   PA4  (D  4)             | SCK
             MOSI | PWM        (D  6)  PA6  7|    |8   PA5  (D  5)        PWM  | MISO
                                             +----+
*/
#define CHANNEL_PRI_LED_PIN 9        // onboard yellow LED
#define CHANNEL_PRI_SENSE_PIN 2      // pin for 12V ADC channel
#define CHANNEL_PRI_THRESHOLD 13.0f  // threshold above which battery is charging e.g. ~14.0V - lead acid should be around 12.8V at rest
#define CHANNEL_PRI_SCALE 21.01f     // dynamic range of the primary sense channel - tested range for first board is 0v to 21.0V
#define CHANNEL_SEC_LED_PIN 10       // onboard yellow LED
#define CHANNEL_SEC_SENSE_PIN 3      // pin for 12V ADC channel
#define CHANNEL_SEC_THRESHOLD 10.0f  // threshold above which battery is present e.g. ~10.0V
#define CHANNEL_SEC_SCALE 21.01f     // dynamic range of the sense channel e.g. 0v to 21.0V
#define CHANNEL_MAINS_LED_PIN 7      // onboard yellow LED
#define CHANNEL_MAINS_SENSE_PIN 0    // pin for 240V no-contact sensor channel - default PB2
#define CHANNEL_MAINS_THRESHOLD LOW  // threshold to indicate signal is present (HIGH or LOW indicate digital IO)
#define CHANNEL_MAINS_SCALE 1.0f     // dynamic range of the sense channel (not used for AC sensing)
#define OUTPUT_PWM_FADE_PIN 8        // 12V PWM LED output
#define OUTPUT_PWM_FADE_STEP 4       // step for each increment of PWM brightness
#define OUTPUT_PWM_FADE_MILLIS 10    // milliseconds for each step of PWM brightness
#define OUTPUT_PWM_CYCLE_MILLIS 3000 // milliseconds between successive PWM fade cycles
#endif

#ifdef TARGET_PROCESSOR_ATTINY85
/*
ATMEL ATTINY85 PINOUTS - NEED CHECKING AGAINST PHYSICAL HARDWARE
                                             +-\/-+
  Sense Secondary | Ain0       (D  5)  PB5  1|    |8   VCC
  Sense Primary   | Ain3       (D  3)  PB3  2|    |7   PB2  (D  2)  INT0  Ain1 | Sense Mains
  Output PWM      | PWM  Ain2  (D  4)  PB4  3|    |6   PB1  (D  1)  PWM (OC0B) | LED 2
                  | OC1B               GND  4|    |5   PB0  (D  0)  PWM (OC0A) | LED 1
                                             +----+
*/
#define CHANNEL_PRI_LED_PIN PB0      // onboard yellow LED
#define CHANNEL_PRI_SENSE_PIN PB3    // pin for 12V ADC channel
#define CHANNEL_PRI_THRESHOLD 13.0f  // threshold above which battery is charging e.g. ~14.0V - lead acid should be around 12.8V at rest
#define CHANNEL_PRI_SCALE 21.01f     // dynamic range of the primary sense channel - tested range for first board is 0v to 21.0V
#define CHANNEL_SEC_LED_PIN PB1      // onboard yellow LED
#define CHANNEL_SEC_SENSE_PIN PB5    // pin for 12V ADC channel
#define CHANNEL_SEC_THRESHOLD 11.0f  // threshold above which battery is present e.g. ~10.0V
#define CHANNEL_SEC_SCALE 21.01f     // dynamic range of the sense channel e.g. 0v to 16.0V
#define CHANNEL_MAINS_LED_PIN PB4    // 12V PWM LED output - default PB4
#define CHANNEL_MAINS_SENSE_PIN PB2  // pin for 240V no-contact sensor channel - default PB2
#define CHANNEL_MAINS_THRESHOLD LOW  // threshold to indicate signal is present (HIGH or LOW indicate digital IO)
#define CHANNEL_MAINS_PWM_FADE 6     // step for each increment of PWM brightness
#define CHANNEL_MAINS_PWM_CYCLE 3000 // milliseconds between successive PWM fade cycles
#endif

#if defined(PINMAPPING_CW)
#warning "This is the CLOCKWISE pin mapping - make sure you're using the pinout diagram with the pins in clockwise order"
// ATMEL ATTINY84A / ARDUINO
//                           +-\/-+
//                     VCC  1|    |14  GND
//             (D 10)  PB0  2|    |13  PA0  (D  0)        AREF
//             (D  9)  PB1  3|    |12  PA1  (D  1)
//             (D 11)  PB3  4|    |11  PA2  (D  2)
//  PWM  INT0  (D  8)  PB2  5|    |10  PA3  (D  3)
//  PWM        (D  7)  PA7  6|    |9   PA4  (D  4)
//  PWM        (D  6)  PA6  7|    |8   PA5  (D  5)        PWM
//                           +----+
#elif defined(PINMAPPING_CCW)
#warning "This is the COUNTERCLOCKWISE pin mapping - make sure you're using the pinout diagram with the pins in counter clockwise order"
// ATMEL ATTINY84A / ARDUINO
//                           +-\/-+
//                     VCC  1|    |14  GND
//             (D  0)  PB0  2|    |13  PA0  (D 10)        AREF
//             (D  1)  PB1  3|    |12  PA1  (D  9)
//             (D 11)  PB3  4|    |11  PA2  (D  8)
//  PWM  INT0  (D  2)  PB2  5|    |10  PA3  (D  7)
//  PWM        (D  3)  PA7  6|    |9   PA4  (D  6)
//  PWM        (D  4)  PA6  7|    |8   PA5  (D  5)        PWM
//                           +----+
#else
#error "Pin mapping is undefined - ensure your inclusion path has 'pins_arduino.h"
#endif

#endif
