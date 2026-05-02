# Battery Charge Indicator

Battery Charge Indicator is an ATTiny84-based embedded monitor for lead-acid battery charging systems.

The firmware determines whether charging is active by combining three inputs:

1. Mains AC detection at the charger input (nominal 240V, detected via an isolated/no-contact sensing path).
2. Primary battery DC level above a charging threshold.
3. Secondary battery/house-master channel condition indicating the system state allows charging indication.

When charging is detected, a 12V output runs a repeating PWM fade cycle to drive an external indicator LED. The board also drives onboard status LEDs for quick diagnostics.

## 1) Functionality

### High-level behavior

At runtime, the firmware continuously evaluates three channels:

- **Mains channel** (`channelMains`): checks for a stable AC waveform around 50Hz.
- **Primary channel** (`channelPrimary`): checks DC voltage against a charging threshold.
- **Secondary channel** (`channelSecondary`): checks DC state used as an interlock/condition.

Charging is considered present only when all are true:

- mains AC present,
- primary battery voltage present (above threshold),
- secondary channel **not** present.

This logic is implemented in `IsCharging()` in `src/main.cpp` with a debounce window to avoid false transitions.

### Debounce and signal conditioning

- **Charge-state debounce:** `DEBOUNCE_HYSTERISIS_MS` (default `1000 ms`) requires stable readings before state changes.
- **AC detection debounce:** mains detection validates repeated leading edges in a 50Hz timing window:
	- min interval `15 ms`
	- max interval `25 ms`
	- continuous detection timeout/hysteresis `200 ms`

### PWM output behavior

When charging starts:

- output fade cycle starts (`Fader::StartFade()`),
- brightness ramps up and down using `analogWrite()`,
- step and timing are controlled by:
	- `OUTPUT_PWM_FADE_STEP` (default `4`)
	- `OUTPUT_PWM_FADE_MILLIS` (default `10 ms`)
	- `OUTPUT_PWM_CYCLE_MILLIS` (default `3000 ms` between cycles)

When charging stops:

- fade is stopped gracefully (`Fader::StopFade()`),
- output returns to off state at the end of the current fade cycle.

### Default ATTiny84A channel configuration

Defined in `src/config.h`:

- **Primary sense pin:** `D2`, threshold `13.0V`, scale `21.01V`
- **Secondary sense pin:** `D3`, threshold `10.0V`, scale `21.01V`
- **Mains sense pin:** `D0`, digital threshold `LOW`
- **PWM output pin:** `D8`
- **Onboard LEDs:** primary `D9`, secondary `D10`, mains `D7`

### Startup behavior

On boot (normal mode), the firmware:

1. initializes sensing channels and output,
2. flashes each onboard channel/output LED (`START_FLASHES`, `START_FLASH_MS`),
3. waits `START_DELAY_MS` before entering main loop.

### Optional operating modes

Compile-time options in `src/config.h`:

- `CALIBRATION_MODE`: samples ADC channels and blinks out measured values for field calibration.
- `PIN_TEST_MODE`: basic pin/LED test routine (`src/pintest.h`) for validating board mapping and outputs.

## 2) Source Code and Build (PlatformIO)

### Repository layout

- `src/main.cpp`: application setup/loop and charging-state logic.
- `src/channel.h/.cpp`: generic channel abstraction, DC thresholding, AC edge timing.
- `src/fader.h/.cpp`: PWM fade output state machine.
- `src/config.h`: target, pins, thresholds, timing constants, feature flags.
- `src/pintest.h`: pin test helper mode.
- `platformio.ini`: build environment (`attiny84`), upload protocol (`usbasp`), CPU clock, variant.

### Toolchain

- [PlatformIO](https://platformio.org/) with VS Code, or PlatformIO Core CLI.
- AVR target from `platformio.ini`:
	- platform: `atmelavr`
	- board: `attiny84`
	- framework: `arduino`
	- upload protocol: `usbasp`
	- MCU: `attiny84`
	- frequency: `8000000L`
	- variant: `tinyX4_reverse`

### Build and upload using VS Code

1. Open this folder in VS Code.
2. Ensure PlatformIO extension is installed.
3. Open the PlatformIO panel.
4. Under **Project Tasks -> attiny84**:
	 - run **Build** to compile,
	 - run **Upload** to flash via USBasp.

### Build and upload using CLI

From the project root:

```bash
pio run
pio run -t upload
```

### Bootloader/fuse notes for fresh ATTiny84A devices

The source comments in `src/main.cpp` include detailed instructions for preparing a fresh ATTiny84A with USBasp, including:

- core board options,
- expected fuse values,
- pin mapping requirement (**Clockwise** mapping),
- proof-of-life LED checks,
- recovery pointer for high-voltage serial programming if needed.

`platformio.ini` also records expected signature/fuse values for reference.

### Calibration

Calibration of the 12V sense lines is performed manually and adjusted in source code. The input voltage range is approximately 0VDC to +20VDC, and ADC response is reasonably linear, as follows:

![Calibration Reference](design/calibration.png)

- `design/calibration.xlsx`: calibration data workbook.
- `design/calibration.png`: calibration reference graphic.

### Brown-out Detection (BOD)

Brown-out Detection is an ATTiny hardware protection feature that keeps the MCU in reset when supply voltage drops below a configured threshold.

What it does:

1. Prevents unstable code execution during low-voltage events.
2. Reduces risk of corrupted state, bad ADC readings, and EEPROM write issues.
3. Restarts cleanly when Vcc returns above the threshold.

Trade-off:

1. Enabling BOD increases power consumption slightly.
2. Higher BOD thresholds can cause more frequent resets on brief supply dips.

How to configure BOD:

Reliability is the priority, so enable BOD (commonly around 2.7V for 8MHz AVR operation):
1. BOD is configured in fuse bits, not in normal C/C++ source code.
2. To set these manually use the Arduino IDE board options plus Burn Bootloader (with USBasp connected).

USBasp workflow (safe and repeatable):

1. Connect USBasp to ICSP and power target correctly.
2. In Arduino IDE, select ATTiny84A board/core options, then set B.O.D. Level 4.3V.
3. Run Burn Bootloader to write the fuse values.
4. Upload firmware from PlatformIO as usual (`pio run -t upload`).
5. Optionally verify written fuses with avrdude before/after changes:

```bash
avrdude -c usbasp -p t84 -U efuse:r:-:h -U hfuse:r:-:h -U lfuse:r:-:h
```

Notes:

1. Keep pin mapping/clock settings aligned with your board core configuration when burning fuses.
2. If the MCU appears unresponsive after incorrect fuse settings, use HV rescue tooling as referenced in the source comments.

### Notes

- This firmware is currently configured for `TARGET_PROCESSOR_ATTINY84`.
- If changing pin mapping, ensure the selected core variant and hardware mapping are consistent before flashing.

## 3) Hardware Design

Hardware design assets are in `design/`. 

Electronics were authored using **EasyEDA** in `design/EasyEDA/`. 

Mechanical assets are in `design/Solidworks/`.

This project has three PCB designs:

1. Main Board
2. ATTiny84A Expander Board
3. LED Board

### PCB 1: Main Board

The Main Board is the core controller board containing the ATTiny85, charging logic, sense interfaces, and PWM output stage. 

Originally this was designed for ATTiny85 but was upgraded to ATTiny84A (see the next section)

#### Schematic

![Main Board Schematic Page 1](design/Battery%20Charge%20Indicator%20Main%20Board%201.svg)

![Main Board Schematic Page 2](design/Battery%20Charge%20Indicator%20Main%20Board%202.svg)

#### PCB

<img src="design/Battery%20Charge%20Indicator%20Main%20Board.png" alt="Main Board 3D" width="25%" />

##### PCB Front (Top)

![Main Board Top](design/Battery%20Charge%20Indicator%20Main%20Board%20Top.svg)

##### PCB Back (Bottom)

![Main Board Bottom](design/Battery%20Charge%20Indicator%20Main%20Board%20Bottom.svg)

### PCB 2: ATTiny84A Expander Board

The ATTiny84A Expander Board (daughter board) supports the microcontroller integration/programming workflow and related signal breakout. This was added to replace the original ATTiny85 and provide a little more flexibility.

#### Schematic

![ATTiny84A Expander Schematic](design/Battery%20Charge%20Indicator%20ATTiny84%20Schematic.svg)

#### PCB

<img src="design/Battery%20Charge%20Indicator%20ATTiny84.png" alt="ATTiny84A Expander 3D" width="25%" />

##### PCB Front (Top)

![ATTiny84A Expander Top](design/Battery%20Charge%20Indicator%20ATTiny84%20Top.svg)

##### PCB Back (Bottom)

![ATTiny84A Expander Bottom](design/Battery%20Charge%20Indicator%20ATTiny84%20Bottom.svg)

### PCB 3: LED Board

The LED Board provides the remote/visible charge indication hardware driven by the PWM output.

#### Schematic

![LED Board Schematic](design/Battery%20Charge%20Indicator%20LED%20Schematic.svg)

#### PCB

<img src="design/Battery%20Charge%20Indicator%20LED.png" alt="LED Board 3D" width="25%" />

##### PCB Front (Top)

![LED Board Top](design/Battery%20Charge%20Indicator%20LED%20Top.svg)

##### PCB Back (Bottom)

![LED Board Bottom](design/Battery%20Charge%20Indicator%20LED%20Bottom.svg)
