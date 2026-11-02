/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include "driver/rmt_encoder.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Type of led strip encoder configuration
 *
 * Fields left at 0 take the WS2812B datasheet values ("Data transfer time" and RES):
 * T0H 400 ns, T0L 850 ns, T1H 800 ns, T1L 450 ns and a 300 us reset.
 */
typedef struct {
  uint32_t resolution; /*!< Encoder resolution, in Hz */
  uint16_t t0h_ticks;  /*!< 0 bit high time in resolution ticks (0: 400 ns) */
  uint16_t t0l_ticks;  /*!< 0 bit low time in resolution ticks (0: 850 ns) */
  uint16_t t1h_ticks;  /*!< 1 bit high time in resolution ticks (0: 800 ns) */
  uint16_t t1l_ticks;  /*!< 1 bit low time in resolution ticks (0: 450 ns) */
  uint32_t reset_us;   /*!< Reset (latch) time after the data, in us (0: 300 us) */
} led_strip_encoder_config_t;

/**
 * @brief Create RMT encoder for encoding LED strip pixels into RMT symbols
 *
 * The encoder takes bytes in wire order (G, R, B for a WS2812B; G, R, B, W for an RGBW part), sends
 * them high bit first and appends the reset code.
 *
 * @param[in] config Encoder configuration
 * @param[out] ret_encoder Returned encoder handle
 * @return
 *      - ESP_ERR_INVALID_ARG for any invalid arguments
 *      - ESP_ERR_NO_MEM out of memory when creating led strip encoder
 *      - ESP_OK if creating encoder successfully
 */
esp_err_t rmt_new_led_strip_encoder(const led_strip_encoder_config_t* config,
                                    rmt_encoder_handle_t* ret_encoder);

#ifdef __cplusplus
}
#endif
