/**
 * @file WS2812ComprehensiveTest.cpp
 * @brief Hardware test of the WS2812 driver on an ESP32-C6.
 *
 * Drives a WS2812B strip on ws2812_test_config::LedPins::DATA_ and checks every step's return
 * code: channel set-up through Begin(), each primary colour (sent green, red, blue on the wire),
 * brightness, an animation, and a second strip next to the first (one RMT memory block each).
 * Watch the strip: red, green, blue, white, a fade, then a rainbow.
 *
 * @copyright Copyright (c) 2024-2025 HardFOC. All rights reserved.
 */

#include "TestFramework.h"
#include "esp32_ws2812_test_config.hpp"
#include "ws2812_cpp.hpp"
#include "ws2812_effects.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <memory>

#ifdef __cplusplus
extern "C" {
#endif
#include "esp_log.h"
#ifdef __cplusplus
}
#endif

static const char* TAG = "WS2812_Test";
static TestResults g_test_results;

//=============================================================================
// TEST CONFIGURATION
//=============================================================================
static constexpr bool ENABLE_BASIC_TESTS = true;
static constexpr bool ENABLE_OUTPUT_TESTS = true;
static constexpr bool ENABLE_MULTI_STRIP_TESTS = true;

using ws2812_test_config::LedPins;
using ws2812_test_config::RMTConfig;
using ws2812_test_config::SignalTiming;
using ws2812_test_config::StripSpecs;

//=============================================================================
// SHARED TEST RESOURCES
//=============================================================================
static std::unique_ptr<WS2812Strip> g_strip;

static std::unique_ptr<WS2812Strip> make_strip(uint8_t gpio, uint32_t leds) noexcept {
  return std::make_unique<WS2812Strip>(
      static_cast<gpio_num_t>(gpio), RMTConfig::CHANNEL, leds,
      StripSpecs::IS_RGBW ? LedType::RGBW : LedType::RGB, SignalTiming::ToTicks(SignalTiming::T0H_NS),
      SignalTiming::ToTicks(SignalTiming::T1H_NS), SignalTiming::ToTicks(SignalTiming::T0L_NS),
      SignalTiming::ToTicks(SignalTiming::T1L_NS), StripSpecs::DEFAULT_BRIGHTNESS);
}

static bool fill_and_show(uint32_t rgb, uint32_t hold_ms) noexcept {
  for (uint32_t i = 0; i < g_strip->Length(); ++i) {
    g_strip->SetPixel(i, rgb);
  }
  const esp_err_t err = g_strip->Show();
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Show() failed: %s", esp_err_to_name(err));
    return false;
  }
  vTaskDelay(pdMS_TO_TICKS(hold_ms));
  return true;
}

//=============================================================================
// TEST HELPER FUNCTIONS
//=============================================================================

static bool init_test_resources() noexcept {
  g_strip = make_strip(LedPins::DATA_, StripSpecs::LED_COUNT);
  return g_strip != nullptr;
}

static void cleanup_test_resources() noexcept {
  if (g_strip) {
    (void)fill_and_show(0, 0); // leave the strip dark
  }
  g_strip.reset();
}

//=============================================================================
// TEST CASES
//=============================================================================

/** Begin() reports whether the RMT channel was created (the constructor no longer aborts). */
static bool test_basic_initialization() noexcept {
  const esp_err_t err = g_strip->Begin();
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Begin() failed: %s", esp_err_to_name(err));
    return false;
  }
  ESP_LOGI(TAG, "Strip ready: %lu LEDs on GPIO%u, reset %lu us",
           static_cast<unsigned long>(g_strip->Length()), LedPins::DATA_,
           static_cast<unsigned long>(WS2812Strip::kResetTimeUs));
  return g_strip->Length() == StripSpecs::LED_COUNT;
}

/** Pixels are 0xRRGGBB; the driver sends them green first. Each colour must look right. */
static bool test_primary_colours() noexcept {
  ESP_LOGI(TAG, "Expect: red, green, blue, white, off (1 s each)");
  return fill_and_show(0xFF0000, 1000) && fill_and_show(0x00FF00, 1000) &&
         fill_and_show(0x0000FF, 1000) && fill_and_show(0xFFFFFF, 1000) && fill_and_show(0, 500);
}

/** One lit LED walks the strip: every frame is latched by the driver's reset pulse. */
static bool test_walking_pixel() noexcept {
  for (uint32_t i = 0; i < g_strip->Length(); ++i) {
    g_strip->SetPixel(i, 0x00FF00);
    if (g_strip->Show() != ESP_OK) {
      return false;
    }
    g_strip->SetPixel(i, 0);
  }
  return fill_and_show(0, 100);
}

static bool test_brightness() noexcept {
  for (int level = 0; level <= 255; level += 15) {
    g_strip->SetBrightness(static_cast<uint8_t>(level));
    if (!fill_and_show(0xFFFFFF, 40)) {
      return false;
    }
  }
  g_strip->SetBrightness(StripSpecs::DEFAULT_BRIGHTNESS);
  return fill_and_show(0, 100);
}

static bool test_rainbow_animation() noexcept {
  WS2812Animator anim(*g_strip);
  anim.SetEffect(WS2812Animator::Effect::Rainbow);
  for (int frame = 0; frame < 150; ++frame) {
    anim.Tick();
    vTaskDelay(pdMS_TO_TICKS(ws2812_test_config::Timing::EFFECT_STEP_MS));
  }
  return fill_and_show(0, 100);
}

/** A second strip gets its own channel: each strip takes one RMT memory block of the target. */
static bool test_second_strip() noexcept {
  auto second = make_strip(LedPins::DATA2_, 8);
  const esp_err_t err = second->Begin();
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Second strip Begin() failed: %s", esp_err_to_name(err));
    return false;
  }
  second->SetPixel(0, 0x0000FF);
  return second->Show() == ESP_OK && g_strip->Show() == ESP_OK;
}

//=============================================================================
// MAIN TEST RUNNER
//=============================================================================

extern "C" void app_main() {
  ESP_LOGI(TAG, "ESP32-C6 WS2812 test suite, driver %s", GetWS2812DriverVersion());

  vTaskDelay(pdMS_TO_TICKS(1000));

  print_test_section_status(TAG, "WS2812");

  if (!init_test_resources()) {
    ESP_LOGE(TAG, "Failed to initialize test resources");
    return;
  }

  RUN_TEST_SECTION_IF_ENABLED_WITH_PATTERN(
      ENABLE_BASIC_TESTS, "WS2812 BASIC TESTS", 5,
      RUN_TEST_IN_TASK("basic_initialization", test_basic_initialization, 8192, 1);
      flip_test_progress_indicator(););

  RUN_TEST_SECTION_IF_ENABLED_WITH_PATTERN(
      ENABLE_OUTPUT_TESTS, "WS2812 OUTPUT TESTS", 5,
      RUN_TEST_IN_TASK("primary_colours", test_primary_colours, 8192, 1);
      flip_test_progress_indicator();
      RUN_TEST_IN_TASK("walking_pixel", test_walking_pixel, 8192, 1);
      flip_test_progress_indicator();
      RUN_TEST_IN_TASK("brightness", test_brightness, 8192, 1);
      flip_test_progress_indicator();
      RUN_TEST_IN_TASK("rainbow_animation", test_rainbow_animation, 8192, 1);
      flip_test_progress_indicator(););

  RUN_TEST_SECTION_IF_ENABLED_WITH_PATTERN(
      ENABLE_MULTI_STRIP_TESTS, "WS2812 MULTI-STRIP TESTS", 5,
      RUN_TEST_IN_TASK("second_strip", test_second_strip, 8192, 1);
      flip_test_progress_indicator(););

  cleanup_test_resources();

  print_test_summary(g_test_results, "WS2812", TAG);

  output_section_indicator(5);

  cleanup_test_progress_indicator();

  while (true) {
    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}
