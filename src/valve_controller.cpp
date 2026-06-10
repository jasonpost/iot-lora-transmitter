#include "valve_controller.h"

#include "mcp23017_io.h"
#include "pins.h"
#include "state_persistence.h"

namespace valve_controller {

namespace {

struct ValvePins {
  uint8_t in1;
  uint8_t in2;
  uint8_t stby;
};

// Section 2 channel mapping. NUM_VALVES=5 would need a 5th entry here
// (GPB3/GPB4 + GPB5 STBY on a 3rd TB6612FNG) -- not yet wired in pins.h.
static_assert(NUM_VALVES <= 4, "valve pin table only covers 4 channels");

constexpr ValvePins kValvePins[4] = {
    {mcp_pins::VALVE1_IN1, mcp_pins::VALVE1_IN2, mcp_pins::TB6612_1_STBY},
    {mcp_pins::VALVE2_IN1, mcp_pins::VALVE2_IN2, mcp_pins::TB6612_1_STBY},
    {mcp_pins::VALVE3_IN1, mcp_pins::VALVE3_IN2, mcp_pins::TB6612_2_STBY},
    {mcp_pins::VALVE4_IN1, mcp_pins::VALVE4_IN2, mcp_pins::TB6612_2_STBY},
};

// Drives one TB6612 channel: boost on -> settle -> STBY/IN1/IN2 pulse ->
// boost off. `inHigh`/`inLow` select the open vs. close direction.
void pulse(uint8_t valveIndex, uint8_t inHigh, uint8_t inLow) {
  Adafruit_MCP23X17& mcp = mcp23017_io::mcp;
  const ValvePins& p = kValvePins[valveIndex];

  mcp.digitalWrite(mcp_pins::BOOST_ENABLE, HIGH);
  delay(BOOST_ENABLE_SETTLE_MS);

  mcp.digitalWrite(p.stby, HIGH);
  mcp.digitalWrite(inHigh, HIGH);
  mcp.digitalWrite(inLow, LOW);
  delay(VALVE_PULSE_MS);

  mcp.digitalWrite(p.in1, LOW);
  mcp.digitalWrite(p.in2, LOW);
  mcp.digitalWrite(p.stby, LOW);

  mcp.digitalWrite(mcp_pins::BOOST_ENABLE, LOW);
}

}  // namespace

Valve valves[NUM_VALVES];

bool init() {
  if (!mcp23017_io::init()) return false;

  Adafruit_MCP23X17& mcp = mcp23017_io::mcp;

  // Step 1 (Section 0 item 5): STBY + boost-enable pins LOW first, then
  // IN1/IN2 LOW. Combined with the external pull-downs, this guarantees no
  // spurious pulse can occur during init.
  mcp.pinMode(mcp_pins::TB6612_1_STBY, OUTPUT);
  mcp.pinMode(mcp_pins::TB6612_2_STBY, OUTPUT);
  mcp.pinMode(mcp_pins::BOOST_ENABLE, OUTPUT);
  mcp.digitalWrite(mcp_pins::TB6612_1_STBY, LOW);
  mcp.digitalWrite(mcp_pins::TB6612_2_STBY, LOW);
  mcp.digitalWrite(mcp_pins::BOOST_ENABLE, LOW);

  for (uint8_t i = 0; i < NUM_VALVES; ++i) {
    const ValvePins& p = kValvePins[i];
    mcp.pinMode(p.in1, OUTPUT);
    mcp.pinMode(p.in2, OUTPUT);
    mcp.digitalWrite(p.in1, LOW);
    mcp.digitalWrite(p.in2, LOW);
  }

  // Step 2 (Section 0 item 6): boot recovery -- fail-safe close any valve
  // that was open when the device last reset.
  state_persistence::init();
  const uint8_t mask = state_persistence::readValveBitmask();
  for (uint8_t i = 0; i < NUM_VALVES; ++i) {
    if (mask & (1u << i)) {
      close(i);
    }
  }

  return true;
}

void open(uint8_t valveIndex) {
  const ValvePins& p = kValvePins[valveIndex];
  pulse(valveIndex, p.in1, p.in2);
  valves[valveIndex].state = ValveState::OPEN;
}

void close(uint8_t valveIndex) {
  const ValvePins& p = kValvePins[valveIndex];
  pulse(valveIndex, p.in2, p.in1);
  valves[valveIndex].state = ValveState::CLOSED;
  valves[valveIndex].timerActive = false;
  state_persistence::setValveBit(valveIndex, false);
}

void openWithTimer(uint8_t valveIndex, uint32_t durationS) {
  open(valveIndex);
  valves[valveIndex].closeAtMillis = millis() + durationS * 1000UL;
  valves[valveIndex].timerActive = true;
  state_persistence::setValveBit(valveIndex, true);
}

void tick(uint32_t nowMillis) {
  for (uint8_t i = 0; i < NUM_VALVES; ++i) {
    Valve& v = valves[i];
    if (v.timerActive && static_cast<int32_t>(nowMillis - v.closeAtMillis) >= 0) {
      close(i);
    }
  }
}

uint32_t secondsUntilNextEvent(uint32_t nowMillis) {
  uint32_t best = UINT32_MAX;
  for (uint8_t i = 0; i < NUM_VALVES; ++i) {
    const Valve& v = valves[i];
    if (!v.timerActive) continue;
    const uint32_t remainingMs = v.closeAtMillis - nowMillis;  // wraps correctly if already due
    const uint32_t remainingS = remainingMs / 1000UL;
    if (remainingS < best) best = remainingS;
  }
  return best;
}

}  // namespace valve_controller
