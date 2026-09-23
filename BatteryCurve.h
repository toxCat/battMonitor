#pragma once

// Converts an open-circuit-ish single-cell Li-ion voltage to state-of-charge
// percent (0-100), linearly interpolating between measured curve points so
// callers get hundredths-place precision instead of a coarse step function.
// The table is a typical discharge curve; recalibrate against your specific
// cells if precision matters at low discharge currents.
float voltageToPercent(float cellVoltage);
