#include "ws2812_control.h"
#include "driver/rmt_tx.h"
#include "driver/rmt_types.h"
#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "led_strip_encoder.h"

#if CONFIG_WS2812_LED_TYPE_RGBW
#define BYTES_PER_LED 4
#else
#define BYTES_PER_LED 3
#endif

// RMT resolution: one tick is 100 ns, the unit of the CONFIG_WS2812_T* values.
#define WS2812_RMT_RESOLUTION_HZ (10 * 1000 * 1000)

// Tag for log messages
static const char* TAG = "NeoPixel WS2812 Driver";

// Pixel bytes in wire order (G, R, B[, W]); the LED strip encoder turns them into RMT symbols and
// appends the reset code.
static uint8_t g_led_bytes[NUM_LEDS * BYTES_PER_LED];
static rmt_channel_handle_t g_handle = NULL;
static rmt_encoder_handle_t g_encoder = NULL;

// Global brightness scaling value
static uint8_t g_brightness = CONFIG_WS2812_DEFAULT_BRIGHTNESS;

static void release(void) {
  if (g_encoder) {
    rmt_del_encoder(g_encoder);
    g_encoder = NULL;
  }
  if (g_handle) {
    rmt_disable(g_handle);
    rmt_del_channel(g_handle);
    g_handle = NULL;
  }
}

esp_err_t ws2812_control_init(gpio_num_t gpio_num, int channel) {
  // ESP-IDF v5's RMT driver allocates channels itself; there is no way to request a number.
  (void)channel;
  if (g_handle != NULL) {
    return ESP_ERR_INVALID_STATE; // already initialised
  }

  rmt_tx_channel_config_t tx_cfg = {};
  tx_cfg.gpio_num = gpio_num;
  tx_cfg.clk_src = RMT_CLK_SRC_DEFAULT;
  tx_cfg.mem_block_symbols = WS2812_RMT_MEM_BLOCK_SYMBOLS;
  tx_cfg.resolution_hz = WS2812_RMT_RESOLUTION_HZ;
  tx_cfg.trans_queue_depth = 1;
  esp_err_t err = rmt_new_tx_channel(&tx_cfg, &g_handle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to create RMT channel: %s", esp_err_to_name(err));
    g_handle = NULL;
    return err;
  }

  led_strip_encoder_config_t enc_cfg = {
      .resolution = WS2812_RMT_RESOLUTION_HZ,
      .t0h_ticks = WS2812_T0H,
      .t0l_ticks = WS2812_T0L,
      .t1h_ticks = WS2812_T1H,
      .t1l_ticks = WS2812_T1L,
      .reset_us = 0, // default: 300 us
  };
  err = rmt_new_led_strip_encoder(&enc_cfg, &g_encoder);
  if (err == ESP_OK) {
    err = rmt_enable(g_handle);
  }
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to set up the LED encoder or enable the channel: %s",
             esp_err_to_name(err));
    release();
  }
  return err;
}

static uint8_t scale(uint32_t value) {
  return (uint8_t)(((value & 0xFF) * g_brightness) / 255);
}

esp_err_t ws2812_write_leds(struct led_state new_state) {
  if (g_handle == NULL || g_encoder == NULL) {
    return ESP_ERR_INVALID_STATE;
  }
  // WS2812B datasheet, "Composition of 24bit data": G7..G0, R7..R0, B7..B0; RGBW parts add W last.
  for (uint32_t led = 0; led < NUM_LEDS; led++) {
    const uint32_t color = new_state.leds[led];
    uint8_t* out = &g_led_bytes[led * BYTES_PER_LED];
    out[0] = scale(color >> 8);  // green
    out[1] = scale(color >> 16); // red
    out[2] = scale(color);       // blue
#if CONFIG_WS2812_LED_TYPE_RGBW
    out[3] = scale(color >> 24); // white
#endif
  }
  rmt_transmit_config_t tx_cfg = {};
  tx_cfg.loop_count = 0;
  ESP_RETURN_ON_ERROR(rmt_transmit(g_handle, g_encoder, g_led_bytes, sizeof(g_led_bytes), &tx_cfg),
                      TAG, "Failed to transmit LED data");
  ESP_RETURN_ON_ERROR(rmt_tx_wait_all_done(g_handle, -1), TAG, "Failed waiting for RMT done");
  return ESP_OK;
}

void ws2812_set_brightness(uint8_t brightness) {
  g_brightness = brightness;
}
