/**
 * @file rmt_common.h
 * @brief Host stub of the RMT calls shared by TX and RX channels.
 */
#pragma once

#include "driver/rmt_types.h"

#ifdef __cplusplus
extern "C" {
#endif
esp_err_t rmt_enable(rmt_channel_handle_t channel);
esp_err_t rmt_disable(rmt_channel_handle_t channel);
esp_err_t rmt_del_channel(rmt_channel_handle_t channel);
#ifdef __cplusplus
}
#endif
