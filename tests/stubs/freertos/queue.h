/**
 * @file queue.h
 * @brief Host stub of the FreeRTOS queue calls used by ws2812::RmtRx.
 */
#pragma once

#include "freertos/FreeRTOS.h"

typedef struct QueueDefinition* QueueHandle_t;

#ifdef __cplusplus
extern "C" {
#endif
QueueHandle_t xQueueCreate(uint32_t length, uint32_t item_size);
BaseType_t xQueueReceive(QueueHandle_t queue, void* item, TickType_t wait);
BaseType_t xQueueSendFromISR(QueueHandle_t queue, const void* item, BaseType_t* woken);
void vQueueDelete(QueueHandle_t queue);
#ifdef __cplusplus
}
#endif
