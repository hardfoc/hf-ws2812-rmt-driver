/**
 * @file esp_log.h
 * @brief Host stub of ESP-IDF logging: the arguments are evaluated, nothing is printed.
 */
#pragma once

static inline void esp_stub_log(const char* tag, const char* format, ...) {
  (void)tag;
  (void)format;
}

#define ESP_LOGE(tag, ...) esp_stub_log(tag, __VA_ARGS__)
#define ESP_LOGW(tag, ...) esp_stub_log(tag, __VA_ARGS__)
#define ESP_LOGI(tag, ...) esp_stub_log(tag, __VA_ARGS__)
#define ESP_LOGD(tag, ...) esp_stub_log(tag, __VA_ARGS__)
#define ESP_LOGV(tag, ...) esp_stub_log(tag, __VA_ARGS__)
