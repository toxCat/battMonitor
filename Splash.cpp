#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "Config.h"
#include "Splash.h"

extern Adafruit_SSD1306 display;

// Placeholder: all pixels off. Swap in your own 128x32 1-bit bitmap here --
// see the format note in Splash.h.
const uint8_t kSplashBitmap[512] PROGMEM = {0};

void drawSplash() {
  display.clearDisplay();
  display.drawBitmap(0, 0, kSplashBitmap, OLED_WIDTH, OLED_HEIGHT, SSD1306_WHITE);
  display.display();
  delay(SPLASH_DURATION_MS);
  display.clearDisplay();
}
