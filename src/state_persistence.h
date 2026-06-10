#pragma once

#include <Arduino.h>

// Persists which valves currently have an active auto-close timer, so a
// reset (brownout, watchdog, crash) while a valve is open can be detected
// on boot and fail-safe closed. See
// iot-valve-controller-implementation-plan.md Section 0 item 6.
//
// Since the protocol requires duration_s on every "open" command, all
// opens are timed -- a single-byte bitmask (bit i = valve i open) is
// sufficient, no real-time-clock/epoch sync needed.
namespace state_persistence {

// Mounts the internal LittleFS filesystem and loads the persisted bitmask.
// Returns false if the filesystem failed to mount (treated as "no
// persisted state": readValveBitmask() returns 0, setValveBit() becomes a
// RAM-only no-op write).
bool init();

// Returns the persisted bitmask, bit i set = valve i has timerActive==true.
uint8_t readValveBitmask();

// Sets or clears bit `valveIndex` and writes the bitmask to flash if it
// changed.
void setValveBit(uint8_t valveIndex, bool open);

}  // namespace state_persistence
