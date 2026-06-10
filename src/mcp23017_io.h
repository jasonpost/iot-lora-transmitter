#pragma once

#include <Adafruit_MCP23X17.h>

// Thin wrapper around the MCP23017 driver: owns the I2C remap (off the
// radio's D4/D5, see pins.h) and the Adafruit_MCP23X17 instance that
// valve_controller drives directly.
namespace mcp23017_io {

extern Adafruit_MCP23X17 mcp;

// Remaps Wire to PIN_I2C_SDA/PIN_I2C_SCL and initializes the MCP23017.
// Returns false if the device doesn't respond at MCP23017_I2C_ADDR.
bool init();

}  // namespace mcp23017_io
