/**
 * @file esp_err.h
 * @brief Host stub of ESP-IDF's error codes.
 */
#pragma once

#include <stdlib.h>

typedef int esp_err_t;

#define ESP_OK 0
#define ESP_FAIL -1
#define ESP_ERR_NO_MEM 0x101
#define ESP_ERR_INVALID_ARG 0x102
#define ESP_ERR_INVALID_STATE 0x103
#define ESP_ERR_NOT_FOUND 0x105
#define ESP_ERR_TIMEOUT 0x107

#ifdef __cplusplus
extern "C" {
#endif
const char* esp_err_to_name(esp_err_t code);
#ifdef __cplusplus
}
#endif

/** Like ESP-IDF: abort when the expression is not ESP_OK. */
#define ESP_ERROR_CHECK(x)                                                                         \
  do {                                                                                             \
    esp_err_t esp_stub_rc_ = (x);                                                                  \
    if (esp_stub_rc_ != ESP_OK) {                                                                  \
      abort();                                                                                     \
    }                                                                                              \
  } while (0)
