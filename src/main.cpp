#include <Arduino.h>

namespace {

constexpr uint32_t SERIAL_BAUD = 115200;
constexpr uint32_t MODEM_BAUD = 115200;
constexpr uint32_t MODEM_TIMEOUT_MS = 2000;
constexpr uint32_t PACKET_INTERVAL_MS = 10000;

// XIAO ESP32C3 default UART pins from the board variant.
constexpr uint8_t PIN_RYLR998_TX = D6;
constexpr uint8_t PIN_RYLR998_RX = D7;

constexpr uint32_t LORA_BAND_HZ = 915000000;
constexpr uint8_t LORA_SPREADING_FACTOR = 9;
constexpr uint8_t LORA_BANDWIDTH_CODE = 7;   // 125 kHz on RYLR998
constexpr uint8_t LORA_CODING_RATE_CODE = 3; // 4/7 on RYLR998
constexpr uint8_t LORA_PREAMBLE = 12;
constexpr uint16_t LORA_ADDRESS = 1;
constexpr uint8_t LORA_NETWORK_ID = 18;

const char *TEST_PAYLOAD = "iot-lora-transmitter test packet";

HardwareSerial modemSerial(1);
uint32_t packetCounter = 0;
uint32_t lastPacketMs = 0;

String readLineFromModem(uint32_t timeoutMs) {
  String response;
  const uint32_t startMs = millis();

  while (millis() - startMs < timeoutMs) {
    while (modemSerial.available() > 0) {
      const char ch = static_cast<char>(modemSerial.read());
      if (ch == '\r') {
        continue;
      }
      if (ch == '\n') {
        if (!response.isEmpty()) {
          response.trim();
          return response;
        }
        continue;
      }
      response += ch;
    }
    delay(5);
  }

  response.trim();
  return response;
}

void drainModemInput() {
  while (modemSerial.available() > 0) {
    modemSerial.read();
  }
}

bool sendCommand(const String &command, const char *expected = "+OK",
                 uint32_t timeoutMs = MODEM_TIMEOUT_MS) {
  drainModemInput();

  Serial.print("MODEM << ");
  Serial.println(command);
  modemSerial.print(command);
  modemSerial.print("\r\n");

  const uint32_t startMs = millis();
  while (millis() - startMs < timeoutMs) {
    const String line = readLineFromModem(timeoutMs);
    if (line.isEmpty()) {
      continue;
    }

    Serial.print("MODEM >> ");
    Serial.println(line);

    if (line.equals(expected)) {
      return true;
    }

    if (line.startsWith("+ERR")) {
      return false;
    }
  }

  Serial.println("MODEM >> timeout");
  return false;
}

bool setupModem() {
  Serial.println("Configuring RYLR998...");

  if (!sendCommand("AT")) {
    return false;
  }

  if (!sendCommand("AT+BAND=" + String(LORA_BAND_HZ))) {
    return false;
  }

  if (!sendCommand("AT+ADDRESS=" + String(LORA_ADDRESS))) {
    return false;
  }

  if (!sendCommand("AT+NETWORKID=" + String(LORA_NETWORK_ID))) {
    return false;
  }

  const String parameterCommand =
      "AT+PARAMETER=" + String(LORA_SPREADING_FACTOR) + "," +
      String(LORA_BANDWIDTH_CODE) + "," + String(LORA_CODING_RATE_CODE) + "," +
      String(LORA_PREAMBLE);

  if (!sendCommand(parameterCommand)) {
    return false;
  }

  return true;
}

bool sendTestPacket() {
  const String payload = String(TEST_PAYLOAD) + " #" + String(packetCounter);
  const String command = "AT+SEND=0," + String(payload.length()) + "," + payload;

  if (!sendCommand(command)) {
    return false;
  }

  Serial.print("LoRa TX OK: ");
  Serial.println(payload);
  ++packetCounter;
  return true;
}

} // namespace

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(1200);

  Serial.println();
  Serial.println("iot-lora-transmitter bring-up");
  Serial.printf("RYLR998 UART TX=%u RX=%u\n", PIN_RYLR998_TX, PIN_RYLR998_RX);

  modemSerial.begin(MODEM_BAUD, SERIAL_8N1, PIN_RYLR998_RX, PIN_RYLR998_TX);
  delay(200);

  if (!setupModem()) {
    Serial.println("RYLR998 setup failed. Check power, UART wiring, and baud rate.");
    return;
  }

  Serial.println("RYLR998 ready");
  sendTestPacket();
  lastPacketMs = millis();
}

void loop() {
  if (millis() - lastPacketMs >= PACKET_INTERVAL_MS) {
    lastPacketMs = millis();
    if (!sendTestPacket()) {
      Serial.println("LoRa TX failed");
    }
  }

  const String line = readLineFromModem(20);
  if (!line.isEmpty()) {
    Serial.print("MODEM >> ");
    Serial.println(line);
  }
}
