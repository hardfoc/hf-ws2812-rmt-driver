/**
 * @file quickstart.cpp
 * @brief The Quickstart page's example, compiled by the host build against the ESP-IDF stubs (never
 * run).
 */

// --8<-- [start:quickstart]
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ws2812_cpp.hpp"

// WS2812B bit timing in RMT ticks of 100 ns (the driver runs the RMT at 10 MHz).
constexpr uint16_t kT0H = 4, kT1H = 8, kT0L = 9, kT1L = 5;

extern "C" void app_main() {
  // 8 LEDs on GPIO8, brightness 32 of 255. The constructor claims an RMT TX channel.
  WS2812Strip strip(GPIO_NUM_8, -1, 8, LedType::RGB, kT0H, kT1H, kT0L, kT1L, 32);
  if (strip.Begin() != ESP_OK) { // no free RMT channel, or a bad GPIO
    ESP_LOGE("leds", "RMT channel set-up failed");
    return;
  }

  for (uint32_t i = 0;; i = (i + 1) % strip.Length()) {
    strip.SetPixel(i, 0xFF0000);  // 0xRRGGBB: one red LED walks along the strip
    if (strip.Show() != ESP_OK) { // blocks until the frame is on the wire
      ESP_LOGE("leds", "RMT transmit failed");
    }
    strip.SetPixel(i, 0);
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}
// --8<-- [end:quickstart]
