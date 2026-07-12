#include "BatteryCurve.h"
#include <stddef.h>

struct CurvePoint {
  float volts;
  float percent;
};

// Descending by voltage. Approximate 1S Li-ion discharge curve at low/moderate
// discharge rates.
static const CurvePoint kCurve[] = {
  {4.20, 100.0}, {4.15, 95.0},  {4.11, 90.0},  {4.08, 85.0},  {4.02, 80.0},
  {3.98, 75.0},  {3.95, 70.0},  {3.91, 65.0},  {3.87, 60.0},  {3.85, 55.0},
  {3.84, 50.0},  {3.82, 45.0},  {3.80, 40.0},  {3.79, 35.0},  {3.77, 30.0},
  {3.75, 25.0},  {3.73, 20.0},  {3.71, 15.0},  {3.69, 10.0},  {3.61, 5.0},
  {3.27, 0.0},
};
static const size_t kCurveLen = sizeof(kCurve) / sizeof(kCurve[0]);

float voltageToPercent(float cellVoltage) {
  if (cellVoltage >= kCurve[0].volts) {
    return 100.0;
  }
  if (cellVoltage <= kCurve[kCurveLen - 1].volts) {
    return 0.0;
  }

  for (size_t i = 0; i < kCurveLen - 1; i++) {
    const CurvePoint &hi = kCurve[i];
    const CurvePoint &lo = kCurve[i + 1];
    if (cellVoltage <= hi.volts && cellVoltage >= lo.volts) {
      float span = hi.volts - lo.volts;
      float frac = (cellVoltage - lo.volts) / span;
      return lo.percent + frac * (hi.percent - lo.percent);
    }
  }
  return 0.0;
}
