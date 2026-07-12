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
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "Config.h"
#include "BatteryCurve.h"

Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET_PIN);

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
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.display();
}

// Averages several ADC samples to cut down on switching-noise jitter.
static float readAveragedVolts(uint8_t pin) {
  uint32_t sum = 0;
  for (uint8_t i = 0; i < ADC_OVERSAMPLE_COUNT; i++) {
    sum += analogRead(pin);
  }
  float counts = (float)sum / ADC_OVERSAMPLE_COUNT;
  return counts * (ADC_REFERENCE_VOLTS / ADC_MAX_COUNTS);
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

// Formats whole minutes as "Hh MMm" into buf (must be at least 8 bytes).
static void formatDuration(uint32_t minutes, char *buf) {
  uint32_t hours = minutes / 60;
  uint32_t mins = minutes % 60;
  snprintf(buf, 8, "%luh%02lum", (unsigned long)hours, (unsigned long)mins);
}

void loop() {
  float batteryVoltage = readBatteryVoltage();
  float currentAmps = readCurrentAmps();
  float socPercent = voltageToPercent(batteryVoltage / BATTERY_CELLS_SERIES);

  bool isCharging = digitalRead(PIN_CHARGING) == HIGH;
  bool isFull = digitalRead(PIN_CHARGE_FULL) == HIGH;
  bool lowBattery = !isCharging && socPercent <= LOW_BATTERY_PERCENT;
  bool blinkOn = (millis() / 500) % 2 == 0;

  display.clearDisplay();
  display.setTextSize(1);

  display.setCursor(0, 0);
  display.print(batteryVoltage, 2);
  display.print("V  ");
  display.print(fabs(currentAmps), 3);
  display.print("A");

  display.setCursor(0, 11);
  display.print("SOC ");
  display.print(socPercent, 2);
  display.print("%");

  display.setCursor(0, 22);
  if (isFull) {
    display.print("FULL");
  } else if (isCharging) {
    display.print("CHARGING");
    if (currentAmps < -CHARGE_CURRENT_MIN_A) {
      float remainingMah = BATTERY_CAPACITY_MAH * (1.0 - socPercent / 100.0);
      float chargeCurrentMa = -currentAmps * 1000.0;
      uint32_t etaMinutes = (uint32_t)((remainingMah / chargeCurrentMa) * 60.0);
      char etaBuf[8];
      formatDuration(etaMinutes, etaBuf);
      display.print(" ETA ");
      display.print(etaBuf);
    }
  } else if (lowBattery) {
    if (blinkOn) {
      display.print("LOW BATTERY");
    }
  }

  display.display();
  delay(DISPLAY_UPDATE_MS);
}
