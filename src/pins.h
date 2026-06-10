#pragma once

#include <Arduino.h>

// All pin assignments for the project live here so they can be changed
// cheaply once verified on the bench. See
// iot-valve-controller-implementation-plan.md Section 2 for the full
// rationale and wiring notes.

// ---- Radio (Wio-SX1262 for XIAO), per
// https://files.seeedstudio.com/products/SenseCAP/Wio_SX1262/Wio-SX1262%20for%20XIAO%20V1.0_SCH.pdf
constexpr uint8_t PIN_LORA_NSS = D4;
constexpr uint8_t PIN_LORA_RST = D2;
constexpr uint8_t PIN_LORA_BUSY = D3;
constexpr uint8_t PIN_LORA_DIO1 = D1;
constexpr uint8_t PIN_LORA_RXEN = D5;
// SPI bus (D8/D9/D10 = SCK/MISO/MOSI) used implicitly by RadioLib's default SPI.

// ---- I2C bus for MCP23017 (remapped off the radio's D4/D5) ----
// OPEN ITEM (Section 0 item 1 addendum): bench-verify D6/D7 are actually
// free on the Wio-SX1262 carrier before relying on this remap.
constexpr uint8_t PIN_I2C_SDA = D6;
constexpr uint8_t PIN_I2C_SCL = D7;
constexpr uint8_t MCP23017_I2C_ADDR = 0x20;

// ---- Battery voltage sense (built into the XIAO nRF52840 itself) ----
// PIN_VBAT (P0.31) / VBAT_ENABLE (P0.14) are provided by the Adafruit nRF52
// core's board variant (xiaoble_adafruit). No external divider or extra
// free pin needed. VBAT_ENABLE must be driven LOW to enable the onboard
// ~2.96 divider before reading PIN_VBAT, and must NOT be driven HIGH while
// charging (P0.31 over-voltage risk).

// ---- MCP23017 pin assignments for valve control ----
// MCP23017 pin numbering (Adafruit_MCP23X17): GPA0-7 = 0-7, GPB0-7 = 8-15.
// Each valve uses 2 pins (TB6612 IN1/IN2 for one H-bridge channel).
//   GPA0/GPA1 -> Valve1 IN1/IN2  (TB6612 #1 channel A)
//   GPA2/GPA3 -> Valve2 IN1/IN2  (TB6612 #1 channel B)
//   GPA4/GPA5 -> Valve3 IN1/IN2  (TB6612 #2 channel A)
//   GPA6/GPA7 -> Valve4 IN1/IN2  (TB6612 #2 channel B)
//   GPB0 -> TB6612 #1 STBY (active HIGH)
//   GPB1 -> TB6612 #2 STBY
//   GPB2 -> Boost converter enable (MT3608 gate via N-MOSFET low-side switch)
//           HIGH = ~9V to TB6612 VM, LOW = boost off (default/sleep)
//   GPB3..GPB7 -> spare (status LED, flow sensor, future Valve5, etc.)
//
// To extend to NUM_VALVES=5: add a 3rd TB6612FNG, wire its channel A to
// GPB3/GPB4 (Valve5 IN1/IN2) and its STBY to GPB5.
//
// CRITICAL (Section 0 item 5): add 10x 10kOhm external pull-down resistors
// -- one on each TB6612 STBY line (GPB0/GPB1) and one on each IN1/IN2 line
// (GPA0-GPA7) -- so the MCP23017's high-Z power-on/reset state defaults
// STBY and IN1/IN2 LOW (driver disabled / short-brake), preventing any
// spurious valve pulse before valveControllerInit() runs.
namespace mcp_pins {

constexpr uint8_t VALVE1_IN1 = 0;  // GPA0
constexpr uint8_t VALVE1_IN2 = 1;  // GPA1
constexpr uint8_t VALVE2_IN1 = 2;  // GPA2
constexpr uint8_t VALVE2_IN2 = 3;  // GPA3
constexpr uint8_t VALVE3_IN1 = 4;  // GPA4
constexpr uint8_t VALVE3_IN2 = 5;  // GPA5
constexpr uint8_t VALVE4_IN1 = 6;  // GPA6
constexpr uint8_t VALVE4_IN2 = 7;  // GPA7

constexpr uint8_t TB6612_1_STBY = 8;   // GPB0
constexpr uint8_t TB6612_2_STBY = 9;   // GPB1
constexpr uint8_t BOOST_ENABLE = 10;   // GPB2

}  // namespace mcp_pins
