# battMonitor

Battery voltage/current monitor for a portable Raspberry Pi case, built
around an Arduino Micro. Two 18650 cells feed an ali-express UPS board that
outputs 5V/3A to the Pi; the Arduino independently measures the raw battery
side and drives a small OLED status display.

## Hardware

- Arduino Micro
- 0.91" SSD1306 OLED, 128x32, I2C
- [Pololu ACS709 current sensor carrier](https://www.pololu.com/product/2199)
- Resistor voltage divider across the battery (values TBD, see below)
- Two digital signals from the UPS board reporting charge state

## Wiring / pin map

| Signal                         | Arduino pin | Notes                                   |
|---------------------------------|-------------|------------------------------------------|
| OLED SDA / SCL                  | D2 / D3     | Hardware I2C on the Micro                 |
| ACS709 VIOUT                    | A0          | Analog, ratiometric around Vcc/2          |
| Battery voltage divider tap     | A1          | Analog                                    |
| UPS "charging" status           | D6          | Digital in, HIGH while charging           |
| UPS "charge full" status        | D7          | Digital in, HIGH when full                |

D2/D3 are reserved for I2C on the Micro, so the charge-status inputs use D6/D7
instead.

## Firmware

`battMonitor.ino` is the main sketch, with calibration constants split out
into `Config.h`, the state-of-charge curve in `BatteryCurve.{h,cpp}`, and the
boot splash bitmap in `Splash.{h,cpp}`.

Dependencies (install via the Arduino Library Manager):

- Adafruit GFX Library
- Adafruit SSD1306
- Adafruit BusIO (pulled in automatically)

### Calibration still needed

A few values in `Config.h` are placeholders until the physical hardware is
finalized, and firmware behavior depends on them:

- **`VOLTAGE_DIVIDER_RATIO`** — set once the divider resistors are chosen.
  Ratio is `(R1 + R2) / R2` where R1 is the battery-side leg and R2 is the
  ground-side leg. Size it so `Vbat_max / ratio` stays comfortably under the
  5V ADC reference.
- **`ACS709_ZERO_CURRENT_VOLTS`** / **`ACS709_SENSITIVITY_V_PER_A`** — depend
  on the ACS709 variant and its SEL pin wiring; measure both against a known
  load.
- **`BATTERY_CELLS_SERIES`** — set to `1` if the two 18650s are wired in
  parallel (typical for these boost-converter UPS boards, single-cell voltage
  range ~3.0-4.2V) or `2` if wired in series.

The state-of-charge percentage (to hundredths precision) comes from linear
interpolation over a typical 1S Li-ion discharge curve in `BatteryCurve.cpp`;
recalibrate that table against your actual cells if precision at low
discharge rates matters.

### ADC precision

Both analog channels are read with oversampling-and-decimation (Atmel AVR121):
16 raw 10-bit samples are summed and shifted down to yield 2 extra effective
bits (~1.22 mV/count at the pin), so the displayed voltage/current stay
meaningful to the hundredths place even after divider scaling. Tune
`ADC_EXTRA_BITS` in `Config.h` if you want more/less oversampling.

### Display states

The display is a small state machine, checked every `DISPLAY_UPDATE_MS`:

| State | Condition                                              | Shows                                  |
|-------|---------------------------------------------------------|-----------------------------------------|
| 0 — Discharging  | not charging, not full, SOC ≥ `LOW_BATTERY_PERCENT` | Voltage, SOC%, current draw (3 lines)    |
| 1 — Charging     | `PIN_CHARGING` HIGH and not full                     | SOC% (large) and "USB-C"                 |
| 2 — Full/Charged | `PIN_CHARGE_FULL` HIGH                               | "CHARGED"                                 |
| 3 — Low battery  | not charging, not full, SOC < `LOW_BATTERY_PERCENT`  | Blinking "LOW BATTERY"                    |

State 2 (full) takes priority over state 1 (charging) if both pins are
somehow HIGH at once; state 3 only applies while actually discharging.

### Boot splash

`Splash.cpp` draws a 128x32 1-bit bitmap for `SPLASH_DURATION_MS` at power-on.
It currently ships as a blank placeholder (`kSplashBitmap`, all-zero, 512
bytes) — generate your own art with a tool like
[image2cpp](https://javl.github.io/image2cpp/) (128x32, "Horizontal - 1 bit
per pixel" export mode matches `Adafruit_GFX::drawBitmap`'s layout) and drop
the resulting byte array in.
