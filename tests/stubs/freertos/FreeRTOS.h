/**
 * @file FreeRTOS.h
 * @brief Host stub of the FreeRTOS types the driver uses.
 */
#pragma once

#include <stdint.h>

typedef uint32_t TickType_t;
typedef long BaseType_t;

#define configTICK_RATE_HZ 1000
#define portMAX_DELAY ((TickType_t)0xFFFFFFFFUL)
#define pdFALSE ((BaseType_t)0)
#define pdTRUE ((BaseType_t)1)
#define pdPASS pdTRUE
#define pdFAIL pdFALSE
#define pdMS_TO_TICKS(ms) ((TickType_t)(ms))
