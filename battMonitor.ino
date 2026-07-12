// Battery charge/current monitor for a Raspberry Pi portable case.
// Target: Arduino Micro driving a 0.91" SSD1306 OLED (I2C), reading pack
// current from a Pololu ACS709 board (A0) and pack voltage from a resistor
// divider (A1), with two digital inputs reporting charger status from the
// UPS board.
//
// Dependencies (install via Arduino Library Manager):
//   - Adafruit GFX Library
//   - Adafruit SSD1306
//   - Adafruit BusIO (pulled in automatically as a dependency)

#include <math.h>
#include <string.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "Config.h"
#include "BatteryCurve.h"
#include "Splash.h"

Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET_PIN);

enum DisplayState : uint8_t {
  STATE_DISCHARGING = 0,
  STATE_CHARGING = 1,
  STATE_FULL = 2,
  STATE_LOW_BATTERY = 3,
};

void setup() {
  pinMode(PIN_CHARGING, INPUT);
  pinMode(PIN_CHARGE_FULL, INPUT);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDR)) {
    // No display to report the failure on; loop forever with the pin state
    // easy to probe with a meter/scope instead of silently misbehaving.
    while (true) {
      delay(1000);
    }
  }
  display.setTextColor(SSD1306_WHITE);
  drawSplash();
}

// Oversamples and decimates ADC_OVERSAMPLE_SAMPLES raw readings to gain
// ADC_EXTRA_BITS of effective resolution (see Config.h), returning volts.
static float readAveragedVolts(uint8_t pin) {
  uint32_t sum = 0;
  for (uint16_t i = 0; i < ADC_OVERSAMPLE_SAMPLES; i++) {
    sum += analogRead(pin);
  }
  uint32_t oversampled = sum >> ADC_EXTRA_BITS;
  return (float)oversampled * (ADC_REFERENCE_VOLTS / (float)ADC_EFFECTIVE_COUNTS);
}

static float readBatteryVoltage() {
  return readAveragedVolts(PIN_BATTERY_VOLTAGE) * VOLTAGE_DIVIDER_RATIO;
}

// Positive = discharging, negative = charging (sign depends on ACS709
// current-flow orientation as wired; flip ACS709_ZERO_CURRENT_VOLTS/polarity
// during calibration if this comes out backwards).
static float readCurrentAmps() {
  float vOut = readAveragedVolts(PIN_CURRENT_SENSE);
  return (vOut - ACS709_ZERO_CURRENT_VOLTS) / ACS709_SENSITIVITY_V_PER_A;
}

static DisplayState determineState(bool isCharging, bool isFull, float socPercent) {
  if (isFull) {
    return STATE_FULL;
  }
  if (isCharging) {
    return STATE_CHARGING;
  }
  if (socPercent < LOW_BATTERY_PERCENT) {
    return STATE_LOW_BATTERY;
  }
  return STATE_DISCHARGING;
}

// dtostrf (not snprintf's "%f") is used throughout since AVR's snprintf
// doesn't format floats without pulling in libprintf_flt.
static void formatValue(float value, uint8_t decimals, const char *suffix, char *outBuf, size_t outSize) {
  char numBuf[12];
  dtostrf(value, 1, decimals, numBuf);
  snprintf(outBuf, outSize, "%s%s", numBuf, suffix);
}

static void printCentered(const char *text, int16_t y, uint8_t size) {
  int16_t textWidth = (int16_t)strlen(text) * 6 * size;
  int16_t x = (OLED_WIDTH - textWidth) / 2;
  if (x < 0) {
    x = 0;
  }
  display.setTextSize(size);
  display.setCursor(x, y);
  display.print(text);
}

static void renderDischarging(float voltage, float socPercent, float currentAmps) {
  char buf[16];
  display.setTextSize(1);

  formatValue(voltage, 2, "V", buf, sizeof(buf));
  display.setCursor(0, 0);
  display.print(buf);

  formatValue(socPercent, 2, "%", buf, sizeof(buf));
  display.setCursor(0, 11);
  display.print(buf);

  formatValue(fabs(currentAmps), 3, "A", buf, sizeof(buf));
  display.setCursor(0, 22);
  display.print(buf);
}

static void renderCharging(float socPercent) {
  char buf[16];
  formatValue(socPercent, 2, "%", buf, sizeof(buf));
  printCentered(buf, 0, 2);
  printCentered("USB-C", 20, 1);
}

static void renderFull() {
  printCentered("CHARGED", 8, 2);
}

static void renderLowBattery(bool blinkOn) {
  if (!blinkOn) {
    return;
  }
  printCentered("LOW", 0, 2);
  printCentered("BATTERY", 16, 2);
}

void loop() {
  float batteryVoltage = readBatteryVoltage();
  float currentAmps = readCurrentAmps();
  float socPercent = voltageToPercent(batteryVoltage / BATTERY_CELLS_SERIES);

  bool isCharging = digitalRead(PIN_CHARGING) == HIGH;
  bool isFull = digitalRead(PIN_CHARGE_FULL) == HIGH;
  DisplayState state = determineState(isCharging, isFull, socPercent);
  bool blinkOn = (millis() / LOW_BATTERY_BLINK_MS) % 2 == 0;

  display.clearDisplay();

  switch (state) {
    case STATE_CHARGING:
      renderCharging(socPercent);
      break;
    case STATE_FULL:
      renderFull();
      break;
    case STATE_LOW_BATTERY:
      renderLowBattery(blinkOn);
      break;
    case STATE_DISCHARGING:
    default:
      renderDischarging(batteryVoltage, socPercent, currentAmps);
      break;
  }

  display.display();
  delay(DISPLAY_UPDATE_MS);
}
