#pragma once

#include <stdint.h>

// 128x32, 1-bit-per-pixel, MSB-first, row-major, rows padded to a whole byte
// (matches Adafruit_GFX::drawBitmap's expected layout, and the "Horizontal"
// export mode of tools like https://javl.github.io/image2cpp/). Currently a
// blank placeholder -- replace the contents of kSplashBitmap in Splash.cpp
// with your own generated array, keeping the same size (512 bytes).
extern const uint8_t kSplashBitmap[512] PROGMEM;

void drawSplash();
