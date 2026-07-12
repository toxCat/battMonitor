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
// ADC
// ---------------------------------------------------------------------------
#define ADC_REFERENCE_VOLTS   5.0
#define ADC_MAX_COUNTS        1023.0
#define ADC_OVERSAMPLE_COUNT  16   // averaged per reading to reduce noise

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
// (R1 = battery-side leg, R2 = ground-side leg). Placeholder until the
// resistor pair is chosen -- update once the divider is built and measured.
// ---------------------------------------------------------------------------
#define VOLTAGE_DIVIDER_RATIO   2.0

// ---------------------------------------------------------------------------
// Battery pack
// BATTERY_CELLS_SERIES: 1 if the two 18650s are wired in parallel (typical for
// these boost-converter UPS HATs, single-cell voltage range), 2 if wired in
// series. The SOC curve is applied per-cell (Vbat / BATTERY_CELLS_SERIES).
// ---------------------------------------------------------------------------
#define BATTERY_CELLS_SERIES   1
#define BATTERY_CAPACITY_MAH   6800   // total pack capacity, adjust to your cells

// ---------------------------------------------------------------------------
// Alarms / behavior
// ---------------------------------------------------------------------------
#define LOW_BATTERY_PERCENT     15.0
#define CHARGE_CURRENT_MIN_A    0.02   // below this, ETA isn't estimated
#define DISPLAY_UPDATE_MS       500
