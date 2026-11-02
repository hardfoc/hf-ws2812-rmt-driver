#pragma once
/**
 * @file ws2812_control.h
 * @brief ESP-IDF RMT backend for WS2812 / NeoPixel LED chains.
 *
 * This is **not** a portable CRTP chip driver. Public headers include
 * ESP-IDF (`driver/rmt_*.h`, `sdkconfig.h`). Hosts that are not ESP32
 * do not compile this tree.
 *
 * @copyright Copyright (c) 2024-2025 HardFOC. All rights reserved.
 */
#include "driver/gpio.h"
#include "driver/rmt_tx.h"
#include "driver/rmt_types.h"
#include "esp_err.h"
#include "led_strip_encoder.h"
#include "sdkconfig.h"
#include "ws2812_rmt_caps.h"
#include <stdint.h>

/**
 * @name Driver constants
 * @{*/

/** Number of LEDs that will be driven by the component. */
#define NUM_LEDS CONFIG_WS2812_NUM_LEDS

#if CONFIG_WS2812_LED_TYPE_RGB
#define WS2812_BITS_PER_LED 24
#elif CONFIG_WS2812_LED_TYPE_RGBW
#define WS2812_BITS_PER_LED 32
#endif

/** Number of RMT items required to represent the LED chain. */
#define WS2812_LED_BUFFER_ITEMS (NUM_LEDS * WS2812_BITS_PER_LED)

/** Timing for a '0' bit high pulse in RMT ticks */
#define WS2812_T0H CONFIG_WS2812_T0H
/** Timing for a '1' bit high pulse in RMT ticks */
#define WS2812_T1H CONFIG_WS2812_T1H
/** Low time for any bit in RMT ticks */
#define WS2812_T0L CONFIG_WS2812_T0L
#define WS2812_T1L CONFIG_WS2812_T1L

/** Default global brightness scaling */
#define WS2812_DEFAULT_BRIGHTNESS CONFIG_WS2812_DEFAULT_BRIGHTNESS

/**@}*/

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Desired colors for a chain of LEDs.
 *
 * Each element of @ref leds describes one LED. The lower three bytes contain
 * the RGB value in the order R (byte 2), G (byte 1) and B (byte 0). When the
 * driver is configured for RGBW LEDs the most significant byte contains the
 * white channel value. ws2812_write_leds() sends them in the order the LED
 * reads them: green, red, blue, then white.
 */
struct led_state {
  /** Array of packed color values for each LED. */
  uint32_t leds[NUM_LEDS];
};

/**
 * @brief Initialise the RMT peripheral for WS2812 output.
 *
 * This function must be called once before any LED data is transmitted.
 * On failure everything it allocated is released, so it can be called again.
 *
 * @param gpio_num GPIO connected to the LED strip.
 * @param channel  Unused: ESP-IDF v5's RMT driver picks a free channel itself.
 * @return ESP_OK on success, ESP_ERR_INVALID_STATE if already initialised, or
 *         an error code from the ESP-IDF drivers.
 */
esp_err_t ws2812_control_init(gpio_num_t gpio_num, int channel);

/**
 * @brief Send LED colour data.
 *
 * Sends every LED's colour (green, red, blue[, white], scaled by the
 * brightness) with the Kconfig bit timing, then the reset code that latches
 * the frame. Blocks the calling task until the frame has been transmitted.
 *
 * @param new_state Desired LED colours.
 * @return ESP_OK on success, ESP_ERR_INVALID_STATE before ws2812_control_init(),
 *         or an ESP-IDF error code.
 */
esp_err_t ws2812_write_leds(struct led_state new_state);

/**
 * @brief Set global brightness for subsequent transmissions.
 *
 * All LED values are scaled by this factor before being sent. 255 means
 * full brightness.
 *
 * @param brightness Brightness level 0-255.
 */
void ws2812_set_brightness(uint8_t brightness);

#ifdef __cplusplus
}
#endif
