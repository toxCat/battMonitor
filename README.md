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
into `Config.h` and the state-of-charge curve in `BatteryCurve.{h,cpp}`.

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
- **`BATTERY_CAPACITY_MAH`** — total pack capacity, used only for the
  charge-time estimate.

The state-of-charge percentage (to hundredths precision) comes from linear
interpolation over a typical 1S Li-ion discharge curve in `BatteryCurve.cpp`;
recalibrate that table against your actual cells if precision at low
discharge rates matters.

### Display layout (128x32)

```
12.34V  1.234A
SOC 87.65%
CHARGING ETA 1h23m   (or FULL / LOW BATTERY, blinking, when applicable)
```

Charge-time-remaining and the low-battery alarm are best-effort/low-priority
features: the ETA is a simple linear estimate from present charge current and
remaining capacity, and the low-battery text blinks once state of charge
drops below `LOW_BATTERY_PERCENT` while not charging.
