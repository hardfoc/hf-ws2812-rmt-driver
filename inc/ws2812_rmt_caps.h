#pragma once
/**
 * @file ws2812_rmt_caps.h
 * @brief RMT resources the driver requests per strip (shared by the C and C++ APIs).
 *
 * @copyright Copyright (c) 2024-2025 HardFOC. All rights reserved.
 */

#include "soc/soc_caps.h"

/**
 * RMT channel memory requested per strip, in symbols: one memory block of the target
 * (SOC_RMT_MEM_WORDS_PER_CHANNEL: 64 on ESP32 and ESP32-S2, 48 on ESP32-S3, C3, C6 and H2).
 * Asking for more than one block makes the channel take its neighbour's block too, and the next
 * strip then finds no free channel. Define it before including the driver to override.
 */
#ifndef WS2812_RMT_MEM_BLOCK_SYMBOLS
#define WS2812_RMT_MEM_BLOCK_SYMBOLS SOC_RMT_MEM_WORDS_PER_CHANNEL
#endif
