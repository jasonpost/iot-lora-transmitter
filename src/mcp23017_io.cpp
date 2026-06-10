#include "mcp23017_io.h"

#include <Wire.h>

#include "pins.h"

namespace mcp23017_io {

Adafruit_MCP23X17 mcp;

bool init() {
  Wire.setPins(PIN_I2C_SDA, PIN_I2C_SCL);
  Wire.begin();
  return mcp.begin_I2C(MCP23017_I2C_ADDR, &Wire);
}

}  // namespace mcp23017_io
