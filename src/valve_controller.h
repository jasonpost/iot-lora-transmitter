#pragma once

#include <Arduino.h>

#include "config.h"

enum class ValveState : uint8_t { CLOSED, OPEN };

struct Valve {
  ValveState state = ValveState::CLOSED;
  uint32_t closeAtMillis = 0;
  bool timerActive = false;
};

// Per-valve state, pulse sequencing via MCP23017 + TB6612FNG, and the
// adaptive-sleep timing helper. See
// iot-valve-controller-implementation-plan.md Sections 2/3.
namespace valve_controller {

extern Valve valves[NUM_VALVES];

// Safe-init sequence (Section 0 items 5 & 6):
//   1. MCP23017 STBY + boost-enable pins -> OUTPUT LOW, then all IN1/IN2
//      pins -> OUTPUT LOW (relies on the external pull-downs in pins.h for
//      the window before this runs).
//   2. Load persisted valve-open bitmask; fail-safe close any valve whose
//      bit was set (reset recovery), then clear the bitmask.
// Returns false if the MCP23017 failed to initialize -- callers should
// treat the controller as unavailable (mcp_ok=false) until a future retry
// succeeds.
bool init();

// 0-based valve index. Pulses open: boost on -> settle -> STBY/IN1/IN2
// pulse -> boost off. Does not touch the timer/persistence -- see
// openWithTimer().
void open(uint8_t valveIndex);

// Pulses close, clears the timer, and updates the persisted bitmask.
void close(uint8_t valveIndex);

// open() + arms the auto-close timer for durationS seconds and persists
// the open bit.
void openWithTimer(uint8_t valveIndex, uint32_t durationS);

// Closes any valve whose timer has expired.
void tick(uint32_t nowMillis);

// Smallest (closeAtMillis - now) in seconds across valves with
// timerActive==true, or UINT32_MAX if none -- drives the adaptive sleep
// interval (Section 6).
uint32_t secondsUntilNextEvent(uint32_t nowMillis);

}  // namespace valve_controller
