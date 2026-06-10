#include "state_persistence.h"

#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>

using namespace Adafruit_LittleFS_Namespace;

namespace state_persistence {

namespace {

constexpr const char* kStateFile = "/valve_state.bin";

uint8_t cachedBitmask = 0;
bool fsOk = false;

}  // namespace

bool init() {
  fsOk = InternalFS.begin();
  if (!fsOk) {
    cachedBitmask = 0;
    return false;
  }

  File f = InternalFS.open(kStateFile, FILE_O_READ);
  if (f) {
    uint8_t b = 0;
    if (f.read(&b, 1) == 1) {
      cachedBitmask = b;
    }
    f.close();
  } else {
    cachedBitmask = 0;
  }
  return true;
}

uint8_t readValveBitmask() { return cachedBitmask; }

void setValveBit(uint8_t valveIndex, bool open) {
  uint8_t newMask = cachedBitmask;
  if (open) {
    newMask |= static_cast<uint8_t>(1u << valveIndex);
  } else {
    newMask &= static_cast<uint8_t>(~(1u << valveIndex));
  }

  if (newMask == cachedBitmask) return;
  cachedBitmask = newMask;

  if (!fsOk) return;

  File f = InternalFS.open(kStateFile, FILE_O_WRITE);
  if (!f) return;
  f.write(&cachedBitmask, 1);
  f.close();
}

}  // namespace state_persistence
