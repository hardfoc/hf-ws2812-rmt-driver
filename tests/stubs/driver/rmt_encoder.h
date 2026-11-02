/**
 * @file rmt_encoder.h
 * @brief Host stub of the ESP-IDF v5 RMT encoder API.
 */
#pragma once

#include "driver/rmt_types.h"

typedef enum {
  RMT_ENCODING_RESET = 0,
  RMT_ENCODING_COMPLETE = (1 << 0),
  RMT_ENCODING_MEM_FULL = (1 << 1),
} rmt_encode_state_t;

struct rmt_encoder_t {
  size_t (*encode)(rmt_encoder_t* encoder, rmt_channel_handle_t tx_channel,
                   const void* primary_data, size_t data_size, rmt_encode_state_t* ret_state);
  esp_err_t (*reset)(rmt_encoder_t* encoder);
  esp_err_t (*del)(rmt_encoder_t* encoder);
};

typedef struct {
  rmt_symbol_word_t bit0;
  rmt_symbol_word_t bit1;
  struct {
    uint32_t msb_first : 1;
  } flags;
} rmt_bytes_encoder_config_t;

typedef struct {
  int reserved;
} rmt_copy_encoder_config_t;

#ifdef __cplusplus
extern "C" {
#endif
esp_err_t rmt_new_bytes_encoder(const rmt_bytes_encoder_config_t* config,
                                rmt_encoder_handle_t* ret_encoder);
esp_err_t rmt_new_copy_encoder(const rmt_copy_encoder_config_t* config,
                               rmt_encoder_handle_t* ret_encoder);
esp_err_t rmt_del_encoder(rmt_encoder_handle_t encoder);
esp_err_t rmt_encoder_reset(rmt_encoder_handle_t encoder);
void* rmt_alloc_encoder_mem(size_t size);
#ifdef __cplusplus
}
#endif

#ifndef __containerof
#define __containerof(ptr, type, member) ((type*)((char*)(ptr) - offsetof(type, member)))
#endif
