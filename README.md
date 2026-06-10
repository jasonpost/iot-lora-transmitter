# iot-lt-valve-controller

A small PlatformIO/Arduino project for transmitting and receiving packets over LoRa from a Seeed XIAO nRF52840 & Wio-SX1262 Kit. The controller controls Hunter irrigation valves. It specifically controls Hunter DC Latching solenoids that open and close the water valves.

Each controller needs to control up to 4 or 5 valves.

This controller communicates with our Heltec Lora Gateway. The Heltec Lora Gateway then communicates to Home Assistant via MQTT.

## Hardware

- Seeed XIAO nRF52840 & Wio-SX1262 Kit
- Power: TP4056 with 2 18650 batteries wired in parallel
- GPIO expansion Module 16-Bit I2C MCP23017
- Dual H-Bridge Driver TB6612FNG from Pololu

### Available Additional Parts Inventory
- 3.3V Buck Regulator D24V3F3
- DC-DC Boost Converter 2-24V > 5-28V MT3608
- Noyito MOSFET Switch Modules Logic Power Switch
- N-Channel Logic MOSFET 30V 5.8A
- N-Channel Logic MOSFET 30V 5A
- P-Channel Logic MOSFET 30V 4A
- Small Signal N-Channel MOSFET BSS138

NOTE: There are more parts in inventory that are not listed. These parts seemed relevant.

## Behavior

The firmware:

- starts a USB serial console at `115200`
- drives the SX1262 radio on the Wio-SX1262 kit directly over SPI via [RadioLib](https://github.com/jgromes/RadioLib)
- sends a compact JSON state packet (`node_id`, `type`, `seq`) on a periodic interval, matching the payload format expected by `iot-lora-gateway`

Current LoRa settings in [`src/main.cpp`](src/main.cpp) (must match `iot-lora-gateway`):

- Frequency: `915.0 MHz`
- Bandwidth: `125 kHz`
- Spreading factor: `9`
- Coding rate: `4/7`
- Sync word: `0x12`
- Output power: `14 dBm`

The radio pin mapping in `src/main.cpp` follows the Wio-SX1262 for XIAO (nRF52840 kit, SKU 102010710) schematic.

The node reports as `node_id: "valve-controller-ferns"` to `iot-lora-gateway`.

## Build
TP
TODO

## Upload

TODO

## Serial Monitor

```bash
pio device monitor -b 115200
```

## Project Files

- [`platformio.ini`](platformio.ini) for the PlatformIO environment
- [`src/main.cpp`](src/main.cpp) for the firmware entry point
