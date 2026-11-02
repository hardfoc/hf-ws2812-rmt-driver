/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "led_strip_encoder.h"
#include "esp_check.h"

static const char* TAG = "led_encoder";

typedef struct {
  rmt_encoder_t base;
  rmt_encoder_t* bytes_encoder;
  rmt_encoder_t* copy_encoder;
  int state;
  rmt_symbol_word_t reset_code;
} rmt_led_strip_encoder_t;

static size_t rmt_encode_led_strip(rmt_encoder_t* encoder, rmt_channel_handle_t channel,
                                   const void* primary_data, size_t data_size,
                                   rmt_encode_state_t* ret_state) {
  rmt_led_strip_encoder_t* led_encoder = __containerof(encoder, rmt_led_strip_encoder_t, base);
  rmt_encoder_handle_t bytes_encoder = led_encoder->bytes_encoder;
  rmt_encoder_handle_t copy_encoder = led_encoder->copy_encoder;
  rmt_encode_state_t session_state = RMT_ENCODING_RESET;
  rmt_encode_state_t state = RMT_ENCODING_RESET;
  size_t encoded_symbols = 0;
  switch (led_encoder->state) {
  case 0: // send RGB data
    encoded_symbols +=
        bytes_encoder->encode(bytes_encoder, channel, primary_data, data_size, &session_state);
    if (session_state & RMT_ENCODING_COMPLETE) {
      led_encoder->state = 1; // switch to next state when current encoding session finished
    }
    if (session_state & RMT_ENCODING_MEM_FULL) {
      state |= RMT_ENCODING_MEM_FULL;
      goto out; // yield if there's no free space for encoding artifacts
    }
  // fall-through
  case 1: // send reset code
    encoded_symbols += copy_encoder->encode(copy_encoder, channel, &led_encoder->reset_code,
                                            sizeof(led_encoder->reset_code), &session_state);
    if (session_state & RMT_ENCODING_COMPLETE) {
      led_encoder->state = RMT_ENCODING_RESET; // back to the initial encoding session
      state |= RMT_ENCODING_COMPLETE;
    }
    if (session_state & RMT_ENCODING_MEM_FULL) {
      state |= RMT_ENCODING_MEM_FULL;
      goto out; // yield if there's no free space for encoding artifacts
    }
  }
out:
  *ret_state = state;
  return encoded_symbols;
}

static esp_err_t rmt_del_led_strip_encoder(rmt_encoder_t* encoder) {
  rmt_led_strip_encoder_t* led_encoder = __containerof(encoder, rmt_led_strip_encoder_t, base);
  rmt_del_encoder(led_encoder->bytes_encoder);
  rmt_del_encoder(led_encoder->copy_encoder);
  free(led_encoder);
  return ESP_OK;
}

static esp_err_t rmt_led_strip_encoder_reset(rmt_encoder_t* encoder) {
  rmt_led_strip_encoder_t* led_encoder = __containerof(encoder, rmt_led_strip_encoder_t, base);
  rmt_encoder_reset(led_encoder->bytes_encoder);
  rmt_encoder_reset(led_encoder->copy_encoder);
  led_encoder->state = RMT_ENCODING_RESET;
  return ESP_OK;
}

esp_err_t rmt_new_led_strip_encoder(const led_strip_encoder_config_t* config,
                                    rmt_encoder_handle_t* ret_encoder) {
  esp_err_t ret = ESP_OK;
  rmt_led_strip_encoder_t* led_encoder = NULL;
  ESP_GOTO_ON_FALSE(config && ret_encoder && config->resolution >= 1000000, ESP_ERR_INVALID_ARG,
                    err, TAG, "invalid argument");
  led_encoder = rmt_alloc_encoder_mem(sizeof(rmt_led_strip_encoder_t));
  ESP_GOTO_ON_FALSE(led_encoder, ESP_ERR_NO_MEM, err, TAG, "no mem for led strip encoder");
  led_encoder->base.encode = rmt_encode_led_strip;
  led_encoder->base.del = rmt_del_led_strip_encoder;
  led_encoder->base.reset = rmt_led_strip_encoder_reset;
  // WS2812B datasheet, "Data transfer time" (each +-150 ns) unless the caller gives ticks.
  const uint32_t ticks_per_us = config->resolution / 1000000;
#define LED_NS_TO_TICKS(ns)                                                                        \
  ((uint16_t)(((uint64_t)(ns) * config->resolution + 500000000ULL) / 1000000000ULL))
  rmt_bytes_encoder_config_t bytes_encoder_config = {
      .bit0 =
          {
              .duration0 = config->t0h_ticks ? config->t0h_ticks : LED_NS_TO_TICKS(400),
              .level0 = 1,
              .duration1 = config->t0l_ticks ? config->t0l_ticks : LED_NS_TO_TICKS(850),
              .level1 = 0,
          },
      .bit1 =
          {
              .duration0 = config->t1h_ticks ? config->t1h_ticks : LED_NS_TO_TICKS(800),
              .level0 = 1,
              .duration1 = config->t1l_ticks ? config->t1l_ticks : LED_NS_TO_TICKS(450),
              .level1 = 0,
          },
      .flags.msb_first = 1 // WS2812 transfer bit order: G7...G0R7...R0B7...B0
  };
#undef LED_NS_TO_TICKS
  ESP_GOTO_ON_ERROR(rmt_new_bytes_encoder(&bytes_encoder_config, &led_encoder->bytes_encoder), err,
                    TAG, "create bytes encoder failed");
  rmt_copy_encoder_config_t copy_encoder_config = {};
  ESP_GOTO_ON_ERROR(rmt_new_copy_encoder(&copy_encoder_config, &led_encoder->copy_encoder), err,
                    TAG, "create copy encoder failed");

  // Reset code (RES): line low for more than 50 us on WS2812B, at least 280 us on WS2812B-V5. Split
  // over the two halves of one symbol (15-bit durations).
  const uint32_t reset_us = config->reset_us ? config->reset_us : 300;
  uint32_t reset_ticks = ticks_per_us * reset_us / 2;
  ESP_GOTO_ON_FALSE(reset_ticks > 0 && reset_ticks <= 0x7FFF, ESP_ERR_INVALID_ARG, err, TAG,
                    "reset time does not fit one RMT symbol");
  led_encoder->reset_code = (rmt_symbol_word_t){
      .level0 = 0,
      .duration0 = reset_ticks,
      .level1 = 0,
      .duration1 = reset_ticks,
  };
  *ret_encoder = &led_encoder->base;
  return ESP_OK;
err:
  if (led_encoder) {
    if (led_encoder->bytes_encoder) {
      rmt_del_encoder(led_encoder->bytes_encoder);
    }
    if (led_encoder->copy_encoder) {
      rmt_del_encoder(led_encoder->copy_encoder);
    }
    free(led_encoder);
  }
  return ret;
}
