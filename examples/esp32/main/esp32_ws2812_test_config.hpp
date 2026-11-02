/**
 * @file esp32_ws2812_test_config.hpp
 * @brief Hardware configuration for WS2812 driver on ESP32-C6
 *
 * This file contains the actual hardware configuration that is used by the HAL
 * and example applications. Modify these values to match your hardware setup.
 *
 * @note The WS2812 driver uses Kconfig for default values. This test config
 * provides compile-time constants that can override Kconfig defaults or be
 * used when Kconfig is not available.
 *
 * @copyright Copyright (c) 2024-2025 HardFOC. All rights reserved.
 */

#pragma once

#include <cstdint>

//==============================================================================
// COMPILE-TIME CONFIGURATION FLAGS
//==============================================================================

/**
 * @brief Enable detailed RMT/LED transaction logging
 *
 * @details
 * When enabled (set to 1), the WS2812 driver will log detailed
 * information about each LED update including:
 * - RMT timing parameters
 * - Pixel data buffer contents
 * - Effect animation state
 *
 * When disabled (set to 0), only basic error logging is performed.
 *
 * Default: 0 (disabled) - Set to 1 to enable for debugging
 */
#ifndef ESP32_WS2812_ENABLE_DETAILED_LOGGING
#define ESP32_WS2812_ENABLE_DETAILED_LOGGING 0
#endif

namespace ws2812_test_config {

/**
 * @brief LED Strip Pin Configuration for ESP32-C6
 *
 * GPIO pin connected to the WS2812 data input.
 */
struct LedPins {
    static constexpr uint8_t DATA_ = 8;           ///< GPIO8 - WS2812 data pin
    static constexpr uint8_t DATA2_ = 4;          ///< GPIO4 - optional second strip (coexistence test)
};

/**
 * @brief RMT Channel Configuration
 *
 * RMT (Remote Control Transceiver) channel settings for signal generation.
 */
struct RMTConfig {
    /// Passed to WS2812Strip for compatibility only: ESP-IDF v5 picks a free channel itself.
    static constexpr int CHANNEL = -1;
};

/**
 * @brief LED Strip Specifications
 *
 * Physical properties of the WS2812 LED strip.
 */
struct StripSpecs {
    static constexpr uint32_t LED_COUNT = 16;           ///< Number of LEDs in the strip
    static constexpr uint8_t DEFAULT_BRIGHTNESS = 50;   ///< Default brightness (0-255)
    static constexpr bool IS_RGBW = false;               ///< true for RGBW strips, false for RGB
};

/**
 * @brief WS2812 Signal Timing Parameters (nanoseconds)
 *
 * WS2812B datasheet, "Data transfer time": T0H 0.4 us, T1H 0.8 us, T0L 0.85 us, T1L 0.45 us, each
 * +-150 ns; RES above 50 us (280 us on WS2812B-V5). The driver adds the reset itself
 * (WS2812Strip::kResetTimeUs).
 */
struct SignalTiming {
    static constexpr uint16_t T0H_NS = 400;     ///< Logic 0, high time (ns)
    static constexpr uint16_t T0L_NS = 850;     ///< Logic 0, low time (ns)
    static constexpr uint16_t T1H_NS = 800;     ///< Logic 1, high time (ns)
    static constexpr uint16_t T1L_NS = 450;     ///< Logic 1, low time (ns)
    static constexpr uint16_t RESET_US = 280;   ///< Reset/latch time (us), minimum

    /// RMT ticks of 100 ns (the driver runs the RMT at 10 MHz), rounded to the nearest tick.
    static constexpr uint16_t ToTicks(uint16_t ns) { return static_cast<uint16_t>((ns + 50) / 100); }
};

/**
 * @brief Supply Voltage Specifications (volts)
 *
 * VDD: LED strip supply voltage
 */
struct SupplyVoltage {
    static constexpr float VDD_MIN = 3.5f;     ///< Minimum VDD voltage (V)
    static constexpr float VDD_NOM = 5.0f;     ///< Nominal VDD voltage (V)
    static constexpr float VDD_MAX = 5.3f;     ///< Maximum VDD voltage (V)
};

/**
 * @brief Current Consumption Estimates
 *
 * Per-LED current draw for power budget calculations.
 */
struct PowerSpecs {
    static constexpr float CURRENT_PER_LED_MA = 60.0f;   ///< Max current per LED at full white (mA)
    static constexpr float IDLE_CURRENT_MA = 1.0f;        ///< Current per LED when off (mA)
};

/**
 * @brief Temperature Specifications (celsius)
 *
 * Operating temperature range for WS2812B LEDs.
 */
struct Temperature {
    static constexpr int16_t OPERATING_MIN = -25;    ///< Minimum operating temperature (°C)
    static constexpr int16_t OPERATING_MAX = 80;     ///< Maximum operating temperature (°C)
    static constexpr int16_t WARNING_THRESHOLD = 70; ///< Temperature warning threshold (°C)
};

/**
 * @brief Timing Parameters
 *
 * Application-level timing for LED updates.
 */
struct Timing {
    static constexpr uint16_t REFRESH_INTERVAL_MS = 16;   ///< LED refresh interval (~60fps) (ms)
    static constexpr uint16_t EFFECT_STEP_MS = 20;        ///< Effect animation step (ms)
    static constexpr uint16_t POWER_ON_DELAY_MS = 10;     ///< Power-on stabilization delay (ms)
};

/**
 * @brief Diagnostic Thresholds
 *
 * Thresholds for monitoring.
 */
struct Diagnostics {
    static constexpr uint16_t POLL_INTERVAL_MS = 1000;    ///< Diagnostic polling interval (ms)
    static constexpr uint8_t MAX_RETRY_COUNT = 3;          ///< Maximum RMT init retries
};

/**
 * @brief Test Configuration
 *
 * Default parameters for testing.
 */
struct TestConfig {
    static constexpr uint16_t TEST_DURATION_MS = 5000;       ///< Test duration (ms)
    static constexpr uint16_t COLOR_CYCLE_DELAY_MS = 50;    ///< Color cycle step delay (ms)
    static constexpr uint8_t TEST_BRIGHTNESS = 64;           ///< Brightness level for tests (0-255)
};

/**
 * @brief Application-specific Configuration
 *
 * Configuration values that can be adjusted per application.
 */
struct AppConfig {
    // Logging
    static constexpr bool ENABLE_DEBUG_LOGGING = true;     ///< Enable detailed debug logs
    static constexpr bool ENABLE_RMT_LOGGING = false;      ///< Enable RMT transaction logs

    // Performance
    static constexpr bool ENABLE_PERFORMANCE_MONITORING = true;  ///< Enable performance metrics
    static constexpr uint16_t STATS_REPORT_INTERVAL_MS = 10000;  ///< Statistics reporting interval

    // Error handling
    static constexpr bool ENABLE_AUTO_RECOVERY = true;     ///< Enable automatic error recovery
    static constexpr uint8_t MAX_ERROR_COUNT = 10;         ///< Maximum errors before failsafe
};

} // namespace ws2812_test_config

/**
 * @brief Hardware configuration validation
 *
 * Compile-time checks to ensure configuration is valid.
 */
static_assert(ws2812_test_config::StripSpecs::LED_COUNT > 0,
              "Number of LEDs must be greater than 0");

static_assert(ws2812_test_config::StripSpecs::DEFAULT_BRIGHTNESS <= 255,
              "Brightness must be 0-255");

static_assert(ws2812_test_config::SignalTiming::T0H_NS + ws2812_test_config::SignalTiming::T0L_NS > 0,
              "Signal timing values must be positive");

static_assert(ws2812_test_config::SignalTiming::T1H_NS + ws2812_test_config::SignalTiming::T1L_NS > 0,
              "Signal timing values must be positive");

/**
 * @brief Helper macro for compile-time GPIO pin validation
 */
#define WS2812_VALIDATE_GPIO(pin) \
    static_assert((pin) >= 0 && (pin) < 30, "Invalid GPIO pin number for ESP32-C6")
