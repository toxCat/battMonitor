#pragma once

// ---------------------------------------------------------------------------
// OLED display (0.91" SSD1306, I2C)
// ---------------------------------------------------------------------------
#define OLED_WIDTH      128
#define OLED_HEIGHT     32
#define OLED_I2C_ADDR   0x3C
#define OLED_RESET_PIN  -1   // shares Arduino reset, no dedicated pin

// ---------------------------------------------------------------------------
// Analog inputs
// ---------------------------------------------------------------------------
#define PIN_CURRENT_SENSE     A0   // ACS709 VIOUT
#define PIN_BATTERY_VOLTAGE   A1   // resistor divider midpoint

// ---------------------------------------------------------------------------
// Charge-status GPIOs from the UPS board (both active HIGH per board behavior)
// D2/D3 are reserved for I2C (SDA/SCL) on the Micro, so avoid them here.
// ---------------------------------------------------------------------------
#define PIN_CHARGING        6   // HIGH while the charger is actively charging
#define PIN_CHARGE_FULL     7   // HIGH once the UPS board reports full charge

// ---------------------------------------------------------------------------
// ADC oversampling
// Standard oversample-and-decimate technique (Atmel AVR121): summing
// 4^ADC_EXTRA_BITS raw 10-bit samples and shifting right by ADC_EXTRA_BITS
// yields ADC_EXTRA_BITS additional effective bits of resolution. With 2 extra
// bits that's 12-bit effective resolution, ~1.22 mV/count at the pin -- well
// under a hundredth of a volt even after divider scaling.
// ---------------------------------------------------------------------------
#define ADC_REFERENCE_VOLTS     5.0
#define ADC_EXTRA_BITS          2
#define ADC_OVERSAMPLE_SAMPLES  (1UL << (2 * ADC_EXTRA_BITS))        // 16
#define ADC_EFFECTIVE_COUNTS    (1024UL << ADC_EXTRA_BITS)           // 4096

// ---------------------------------------------------------------------------
// ACS709 current sensor calibration
// Output is ratiometric: VIOUT = Vcc/2 at zero current, with a sensitivity
// (mV per A) set by the module's SEL pin/version. Recalibrate both constants
// against a known load before trusting the current reading.
// ---------------------------------------------------------------------------
#define ACS709_ZERO_CURRENT_VOLTS    2.50
#define ACS709_SENSITIVITY_V_PER_A   0.066   // e.g. 66 mV/A, 35A range, SEL=high

// ---------------------------------------------------------------------------
// Battery voltage divider
// Vbat = Vadc * VOLTAGE_DIVIDER_RATIO, ratio = (R1 + R2) / R2
// (R1 = battery-side leg, R2 = ground-side leg). Measured divider: 33k top
// (R1) / 100k bottom (R2) -> ratio = (33k + 100k) / 100k = 1.33. At the
// pack's 4.2V full point that puts Vadc at ~3.16V, comfortably under the 5V
// ADC reference.
// ---------------------------------------------------------------------------
#define VOLTAGE_DIVIDER_RATIO   1.33

// ---------------------------------------------------------------------------
// Battery pack
// Two 18650s wired in parallel (confirmed): single-cell voltage range, full
// at 4.20V and empty at 3.10V (the UPS board's cutoff) -- see BatteryCurve.cpp.
// BATTERY_CELLS_SERIES divides Vbat down to a per-cell voltage before it's
// run through the SOC curve; set to 2 if the pack is ever rewired in series.
// ---------------------------------------------------------------------------
#define BATTERY_CELLS_SERIES   1

// ---------------------------------------------------------------------------
// Alarms / behavior
// ---------------------------------------------------------------------------
#define LOW_BATTERY_PERCENT     5.0
#define DISPLAY_UPDATE_MS       500
#define SPLASH_DURATION_MS      1500
#define LOW_BATTERY_BLINK_MS    500
