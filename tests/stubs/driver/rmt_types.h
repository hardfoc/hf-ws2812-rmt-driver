/**
 * @file rmt_types.h
 * @brief Host stub of the ESP-IDF v5 RMT types (same names and symbol layout as ESP-IDF).
 */
#pragma once
#pragma GCC system_header

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"

/** One RMT symbol: two (level, duration) pairs, durations in resolution ticks. */
typedef union {
  struct {
    uint16_t duration0 : 15;
    uint16_t level0 : 1;
    uint16_t duration1 : 15;
    uint16_t level1 : 1;
  };
  uint32_t val;
} rmt_symbol_word_t;

typedef struct rmt_channel_t* rmt_channel_handle_t;
typedef struct rmt_encoder_t rmt_encoder_t;
typedef struct rmt_encoder_t* rmt_encoder_handle_t;

typedef enum {
  RMT_CLK_SRC_APB = 4,
  RMT_CLK_SRC_DEFAULT = RMT_CLK_SRC_APB,
} rmt_clock_source_t;

typedef struct {
  rmt_symbol_word_t* received_symbols;
  size_t num_symbols;
} rmt_rx_done_event_data_t;

typedef bool (*rmt_rx_done_callback_t)(rmt_channel_handle_t rx_chan,
                                       const rmt_rx_done_event_data_t* edata, void* user_ctx);
