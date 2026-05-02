#pragma once
#ifndef _WATCHDOG_SUPPORT_H_
#define _WATCHDOG_SUPPORT_H_

#include "config.h"

#ifdef ENABLE_WATCHDOG_TIMER

// Captured very early at startup from MCUSR to retain reset cause information.
uint8_t g_watchdogResetCause __attribute__((section(".noinit")));

// Run before setup() to avoid watchdog reset loops after watchdog-caused resets.
void watchdogEarlyInit(void) __attribute__((used)) __attribute__((naked)) __attribute__((section(".init3")));

void watchdogEarlyInit(void)
{
    // Important: clear WDRF before disabling WDT, otherwise WDE can remain forced.
    g_watchdogResetCause = MCUSR;
    MCUSR = 0;
    wdt_disable();
}

static inline void watchdogInit()
{
    wdt_enable(WATCHDOG_TIMEOUT);
    wdt_reset();
}

static inline void watchdogFeed()
{
    wdt_reset();
}

static inline bool watchdogWasReset()
{
    return (g_watchdogResetCause & _BV(WDRF)) != 0;
}

static inline void indicateWatchdogReset()
{
    if (!watchdogWasReset())
        return;

    // Briefly flash all three onboard channel LEDs to indicate a watchdog recovery reset.
    pinMode(CHANNEL_PRI_LED_PIN, OUTPUT);
    pinMode(CHANNEL_SEC_LED_PIN, OUTPUT);
    pinMode(CHANNEL_MAINS_LED_PIN, OUTPUT);

    for (uint8_t i = 0; i < WATCHDOG_RESET_FLASHES; i++)
    {
        digitalWrite(CHANNEL_PRI_LED_PIN, HIGH);
        digitalWrite(CHANNEL_SEC_LED_PIN, HIGH);
        digitalWrite(CHANNEL_MAINS_LED_PIN, HIGH);
        watchdogFeed();
        delay(WATCHDOG_RESET_FLASH_MS);

        digitalWrite(CHANNEL_PRI_LED_PIN, LOW);
        digitalWrite(CHANNEL_SEC_LED_PIN, LOW);
        digitalWrite(CHANNEL_MAINS_LED_PIN, LOW);
        watchdogFeed();
        delay(WATCHDOG_RESET_FLASH_MS);
    }
}

#else

static inline void watchdogInit() {}
static inline void watchdogFeed() {}
static inline bool watchdogWasReset() { return false; }
static inline void indicateWatchdogReset() {}

#endif

#endif
