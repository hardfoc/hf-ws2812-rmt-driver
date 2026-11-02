/**
 * @file soc_caps.h
 * @brief Host stub of the ESP-IDF SoC capabilities the driver reads: those of the ESP32-C6, the
 * example target (48 RMT symbols per memory block, as on the S3, C3 and H2).
 */
#pragma once

#define SOC_RMT_MEM_WORDS_PER_CHANNEL 48
#define SOC_RMT_TX_CANDIDATES_PER_GROUP 2
