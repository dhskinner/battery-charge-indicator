#include "config.h"

#pragma once
#ifndef _PINTEST_H_
#define _PINTEST_H_

/*
ATMEL ATTINY84A / ARDUINO - CLOCKWISE PINS  !!!! IMPORTANT THIS IS SET CORRECTLY IN THE BOOTLOADER !!!
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

void pinTestSetup()
{
    pinMode(10, OUTPUT);
    pinMode(9, OUTPUT);
    pinMode(8, OUTPUT);
    pinMode(7, OUTPUT);
    pinMode(6, OUTPUT);
    pinMode(5, OUTPUT);
    pinMode(4, OUTPUT);
    pinMode(3, OUTPUT);
    pinMode(2, OUTPUT);
    pinMode(1, OUTPUT);
    pinMode(0, OUTPUT);

    // set all pins to a known state
    digitalWrite(10, LOW);
    digitalWrite(9, LOW);
    digitalWrite(8, LOW);
    digitalWrite(7, LOW);
    digitalWrite(6, LOW);
    digitalWrite(5, LOW);
    digitalWrite(4, LOW);
    digitalWrite(3, LOW);
    digitalWrite(2, LOW);
    digitalWrite(1, LOW);
    digitalWrite(0, LOW);

    // set the output channel pins high
    digitalWrite(3, HIGH); // sense secondary
    digitalWrite(2, HIGH); // sense primary
    digitalWrite(0, HIGH); // sense main
}

void pinTestRun()
{
    // basic test
    // digitalWrite(LED_BUILTIN, HIGH);

    // onboard LEDS
    digitalWrite(10, HIGH);
    digitalWrite(9, HIGH);
    digitalWrite(8, HIGH);
    digitalWrite(7, HIGH);

    // digitalWrite(6, HIGH);
    // digitalWrite(5, HIGH);
    // digitalWrite(4, HIGH);
    // digitalWrite(3, HIGH);
    // digitalWrite(2, HIGH);
    // digitalWrite(1, HIGH);
    // digitalWrite(0, HIGH);
    delay(1000);

    // basic test
    // digitalWrite(LED_BUILTIN, LOW);

    // onboard LEDS
    digitalWrite(10, LOW);
    digitalWrite(9, LOW);
    digitalWrite(8, LOW);
    digitalWrite(7, LOW);

    // digitalWrite(6, LOW);
    // digitalWrite(5, LOW);
    // digitalWrite(4, LOW);
    // digitalWrite(3, LOW);
    // digitalWrite(2, LOW);
    // digitalWrite(1, LOW);
    // digitalWrite(0, LOW);
    delay(1000);
}

#endif