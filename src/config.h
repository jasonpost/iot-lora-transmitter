#pragma once

#include <Arduino.h>

// Tunable constants: behavior, protocol, and timing. "What's wired where"
// lives in pins.h instead. See iot-valve-controller-implementation-plan.md
// Section 10 for the full rationale.

constexpr const char* NODE_ID = "valve-controller-ferns";
constexpr uint8_t NUM_VALVES = 4;  // 2x TB6612FNG; bump to 5 with a 3rd TB6612FNG

// ---- LoRa (must match iot-lora-gateway) ----
constexpr float LORA_FREQUENCY_MHZ = 915.0;
constexpr float LORA_BANDWIDTH_KHZ = 125.0;
constexpr uint8_t LORA_SPREADING_FACTOR = 9;
constexpr uint8_t LORA_CODING_RATE = 7;  // 4/7
constexpr uint8_t LORA_SYNC_WORD = 0x12;
constexpr int8_t LORA_OUTPUT_POWER_DBM = 14;
constexpr uint16_t LORA_PREAMBLE_LENGTH = 8;
constexpr float LORA_TCXO_VOLTAGE = 1.8;  // Wio-SX1262 module TCXO

// ---- Timing ----
constexpr uint32_t IDLE_WAKE_INTERVAL_S = 300;  // 5 min; also the cap on the adaptive interval
constexpr uint32_t MIN_WAKE_INTERVAL_S = 5;     // floor near a scheduled close
constexpr uint32_t RX_WINDOW_MS = 1500;
constexpr uint32_t WAKE_JITTER_MAX_S = 3;       // anti-collision jitter
constexpr uint32_t VALVE_PULSE_MS = 250;        // Hunter 458200 datasheet pulse width
constexpr uint32_t BOOST_ENABLE_SETTLE_MS = 30; // OPEN ITEM: tune on bench
constexpr uint32_t MAX_VALVE_DURATION_S = 86400; // sanity cap on open commands

// ---- Solenoid supply (MT3608 boost converter) ----
constexpr float SOLENOID_SUPPLY_TARGET_V = 9.0f;  // Hunter 458200: 6-9V range, top end

// ---- Battery (onboard PIN_VBAT/VBAT_ENABLE divider) ----
constexpr float BATTERY_DIVIDER_RATIO = 2.96f;  // (1M + 510R) / 510R
constexpr float BATTERY_ADC_REF_V = 3.0f;       // OPEN ITEM: verify analogReference()
constexpr float BATTERY_CAL_OFFSET_V = 0.0f;    // bench-calibrated
