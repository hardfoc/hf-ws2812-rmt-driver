/**
 * @file gpio.h
 * @brief Host stub of the GPIO number type.
 */
#pragma once

typedef enum {
  GPIO_NUM_NC = -1,
  GPIO_NUM_0 = 0,
  GPIO_NUM_2 = 2,
  GPIO_NUM_8 = 8,
  GPIO_NUM_18 = 18,
  GPIO_NUM_MAX = 64,
} gpio_num_t;
