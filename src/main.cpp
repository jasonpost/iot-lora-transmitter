#include <Arduino.h>
#include <SPI.h>
#include <RadioLib.h>
#include <ArduinoJson.h>

#include "config.h"
#include "pins.h"
#include "valve_controller.h"

namespace {

constexpr uint32_t SERIAL_BAUD = 115200;
constexpr uint32_t PACKET_INTERVAL_MS = 10000;

SX1262 radio = new Module(PIN_LORA_NSS, PIN_LORA_DIO1, PIN_LORA_RST, PIN_LORA_BUSY);

bool loraReady = false;
bool mcpReady = false;
uint32_t packetSeq = 0;
uint32_t lastPacketMs = 0;

bool setupLoRa() {
  radio.setRfSwitchPins(PIN_LORA_RXEN, RADIOLIB_NC);

  int state = radio.begin(LORA_FREQUENCY_MHZ, LORA_BANDWIDTH_KHZ, LORA_SPREADING_FACTOR,
                           LORA_CODING_RATE, LORA_SYNC_WORD, LORA_OUTPUT_POWER_DBM,
                           LORA_PREAMBLE_LENGTH, LORA_TCXO_VOLTAGE);
  if (state == RADIOLIB_ERR_NONE) state = radio.setDio2AsRfSwitch(true);

  if (state != RADIOLIB_ERR_NONE) {
    Serial.print("LoRa init failed, code ");
    Serial.println(state);
    return false;
  }

  Serial.println("LoRa ready: 915.0 MHz, SF9, BW125, CR 4/7, SW 0x12");
  return true;
}

bool sendStatePacket() {
  StaticJsonDocument<128> doc;
  doc["node_id"] = NODE_ID;
  doc["type"] = "state";
  doc["seq"] = packetSeq;

  String payload;
  serializeJson(doc, payload);

  const int state = radio.transmit(payload);
  if (state != RADIOLIB_ERR_NONE) {
    Serial.print("LoRa TX failed, code ");
    Serial.println(state);
    return false;
  }

  Serial.print("LoRa TX OK: ");
  Serial.println(payload);
  ++packetSeq;
  return true;
}

// Serial test harness for bench-testing the valve controller without the
// radio link: "open <1..NUM_VALVES> <seconds>" / "close <1..NUM_VALVES>".
void handleSerialCommand(const String& line) {
  if (!mcpReady) {
    Serial.println("mcp23017 not ready, ignoring command");
    return;
  }

  int firstSpace = line.indexOf(' ');
  String cmd = firstSpace < 0 ? line : line.substring(0, firstSpace);

  if (cmd == "open") {
    int secondSpace = line.indexOf(' ', firstSpace + 1);
    if (firstSpace < 0 || secondSpace < 0) {
      Serial.println("usage: open <1-" + String(NUM_VALVES) + "> <seconds>");
      return;
    }
    int valveNum = line.substring(firstSpace + 1, secondSpace).toInt();
    long durationS = line.substring(secondSpace + 1).toInt();
    if (valveNum < 1 || valveNum > NUM_VALVES || durationS <= 0) {
      Serial.println("usage: open <1-" + String(NUM_VALVES) + "> <seconds>");
      return;
    }
    valve_controller::openWithTimer(valveNum - 1, static_cast<uint32_t>(durationS));
    Serial.println("opened valve " + String(valveNum) + " for " + String(durationS) + "s");
  } else if (cmd == "close") {
    if (firstSpace < 0) {
      Serial.println("usage: close <1-" + String(NUM_VALVES) + ">");
      return;
    }
    int valveNum = line.substring(firstSpace + 1).toInt();
    if (valveNum < 1 || valveNum > NUM_VALVES) {
      Serial.println("usage: close <1-" + String(NUM_VALVES) + ">");
      return;
    }
    valve_controller::close(valveNum - 1);
    Serial.println("closed valve " + String(valveNum));
  } else {
    Serial.println("unknown command: " + line);
  }
}

void pollSerial() {
  static String line;
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      line.trim();
      if (line.length() > 0) handleSerialCommand(line);
      line = "";
    } else {
      line += c;
    }
  }
}

} // namespace

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(1200);

  Serial.println();
  Serial.println("iot-lt-valve-controller bring-up");

  mcpReady = valve_controller::init();
  if (!mcpReady) {
    Serial.println("valve_controller::init() failed (MCP23017 not found)");
  }

  loraReady = setupLoRa();
  if (loraReady) {
    sendStatePacket();
    lastPacketMs = millis();
  }
}

void loop() {
  pollSerial();
  valve_controller::tick(millis());

  if (loraReady && millis() - lastPacketMs >= PACKET_INTERVAL_MS) {
    lastPacketMs = millis();
    sendStatePacket();
  }
}
