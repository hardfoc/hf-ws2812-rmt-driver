/**
 * @file api_tasks.cpp
 * @brief The API guide's examples, task by task, compiled by the host build against the ESP-IDF
 * stubs (never run).
 */
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ws2812_multi_animator.hpp"

#include <cstdio>

namespace {
constexpr uint32_t Grb(uint8_t r, uint8_t g, uint8_t b) {
  return (uint32_t{g} << 16) | (uint32_t{r} << 8) | b;
}
} // namespace

// --8<-- [start:create]
// WS2812B timing in 100 ns ticks: T0H 400 ns, T1H 800 ns, T0L 900 ns, T1L 500 ns.
WS2812Strip make_status_strip() {
  return WS2812Strip(GPIO_NUM_8,   // data pin
                     -1,           // channel: ignored, ESP-IDF picks a free one
                     16,           // LEDs
                     LedType::RGB, // 24 bits per LED (LedType::RGBW: 32)
                     4, 8, 9, 5,   // T0H, T1H, T0L, T1L in ticks
                     64);          // brightness 0-255
}
// --8<-- [end:create]

// --8<-- [start:pixels]
esp_err_t show_gradient(WS2812Strip& strip) {
  const uint32_t n = strip.Length();
  for (uint32_t i = 0; i < n; ++i) {
    const auto level = static_cast<uint8_t>(255 * i / (n - 1));
    strip.SetPixel(i, Grb(level, 0, 255 - level)); // blue to red
  }
  return strip.Show(); // nothing reaches the LEDs until Show()
}
// --8<-- [end:pixels]

// --8<-- [start:brightness]
esp_err_t dim(WS2812Strip& strip) {
  strip.SetBrightness(16); // scales every channel of every LED at the next Show()
  return strip.Show();
}
// --8<-- [end:brightness]

// --8<-- [start:resize]
esp_err_t grow(WS2812Strip& strip) {
  strip.SetNumLeds(60); // all pixels are cleared to 0
  strip.SetPixel(59, Grb(0, 0, 255));
  return strip.Show();
}
// --8<-- [end:resize]

// --8<-- [start:timings]
void use_ws2812_original_timing(WS2812Strip& strip) {
  // WS2812 (not B): T0H 350 ns, T1H 700 ns, T0L 800 ns, T1L 600 ns.
  strip.SetTimings(4, 7, 8, 6); // ticks of 100 ns; 350 ns rounds to 400 ns, within +-150 ns
}
// --8<-- [end:timings]

// --8<-- [start:animate]
void led_task(void* arg) {
  auto& strip = *static_cast<WS2812Strip*>(arg);
  WS2812Animator anim(strip);
  anim.SetEffect(WS2812Animator::Effect::Rainbow);
  for (;;) {
    anim.Tick(); // computes one frame and calls strip.Show()
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}
// --8<-- [end:animate]

// --8<-- [start:multi]
void two_strips(WS2812Strip& front, WS2812Strip& rear) {
  // unified = true: both strips run the effect over the longer strip's length.
  // sync = true: one step counter for both.
  WS2812MultiAnimator multi({&front, &rear}, true, true);
  multi.SetEffect(WS2812Animator::Effect::Chase, Grb(255, 80, 0));
  multi.SetEffect(1, WS2812Animator::Effect::Blink, Grb(255, 0, 0)); // rear strip only
  for (int frame = 0; frame < 500; ++frame) {
    multi.Tick();
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}
// --8<-- [end:multi]

// --8<-- [start:errors]
bool show_or_report(WS2812Strip& strip) {
  const esp_err_t err = strip.Show();
  if (err != ESP_OK) {
    // The RMT driver's code, for example ESP_ERR_INVALID_STATE when the channel is not enabled.
    ESP_LOGE("leds", "Show failed: %s", esp_err_to_name(err));
    return false;
  }
  return true;
}
// --8<-- [end:errors]

// --8<-- [start:raw]
esp_err_t send_raw_grb(ws2812::RmtTx& tx) {
  // Bytes go out as they are, MSB first, with the WS2812B timing built into RmtTx (10 MHz
  // resolution).
  const uint8_t frame[] = {0x00, 0xFF, 0x00,  // LED 0: red   (G, R, B)
                           0xFF, 0x00, 0x00}; // LED 1: green
  return tx.TransmitWs2812(frame, sizeof(frame));
}
// --8<-- [end:raw]

// --8<-- [start:version]
void print_version() {
  std::printf("hf-ws2812-rmt-driver %s\n", WS2812Strip::GetDriverVersion());
}
// --8<-- [end:version]
