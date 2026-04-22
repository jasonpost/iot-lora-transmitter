# iot-lora-transmitter

A small PlatformIO/Arduino project for transmitting periodic test packets over LoRa from a Seeed XIAO ESP32C3 through an RYLR998 modem.

## Hardware

- Seeed XIAO ESP32C3
- Reyax RYLR998 LoRa modem
- UART wiring:
  - XIAO `D6` -> RYLR998 RX
  - XIAO `D7` -> RYLR998 TX

## Behavior

The firmware:

- starts a USB serial console at `115200`
- configures the modem with AT commands
- sets the LoRa band to `915000000`
- sends a test packet immediately on boot
- sends another test packet every 10 seconds

Current LoRa settings in [`src/main.cpp`](src/main.cpp):

- Address: `1`
- Network ID: `18`
- Spreading factor: `9`
- Bandwidth code: `7`
- Coding rate code: `3`
- Preamble: `12`

## Build

```bash
pio run -e seeed_xiao_esp32c3
```

## Upload

```bash
pio run -e seeed_xiao_esp32c3 --target upload
```

## Serial Monitor

```bash
pio device monitor -b 115200
```

## Project Files

- [`platformio.ini`](platformio.ini) for the PlatformIO environment
- [`src/main.cpp`](src/main.cpp) for the firmware entry point
- [`boards/heltec_wifi_lora_32_v4.json`](boards/heltec_wifi_lora_32_v4.json) is included in the repo but not used by the active `platformio.ini` environment
