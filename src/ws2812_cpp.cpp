/**
 * @file ws2812_cpp.cpp
 * @brief WS2812Strip C++ wrapper implementation
 * @copyright Copyright (c) 2024-2025 HardFOC. All rights reserved.
 */

#include "ws2812_cpp.hpp"
#include "driver/rmt_types.h"

#ifdef __cplusplus

WS2812Strip::WS2812Strip(gpio_num_t gpio, int channel, uint32_t num_leds,
#if CONFIG_WS2812_LED_TYPE_RGBW
                         LedType type,
#else
                         LedType type,
#endif
                         uint16_t t0h, uint16_t t1h, uint16_t t0l, uint16_t t1l, uint8_t brightness)
    : pixels_(num_leds, 0), buffer_(num_leds * (type == LedType::RGBW ? 32 : 24) + 1),
      rmt_(gpio, 10'000'000, ws2812::RmtTx::kDefaultMemSymbols, false, 4, RMT_CLK_SRC_DEFAULT,
           channel),
      gpio_(gpio), channel_(channel), type_(type), t0h_(t0h), t1h_(t1h), t0l_(t0l), t1l_(t1l),
      brightness_(brightness), num_leds_(num_leds) {
}

esp_err_t WS2812Strip::Begin() {
  return rmt_.Init(); // ESP_OK if the constructor's set-up succeeded, otherwise retried here
}

void WS2812Strip::SetPixel(uint32_t index, uint32_t rgbw) {
  if (index < num_leds_) {
    pixels_[index] = rgbw;
  }
}

uint32_t WS2812Strip::Length() const {
  return num_leds_;
}

esp_err_t WS2812Strip::Show() {
  const uint32_t bits_per_led = (type_ == LedType::RGBW) ? 32 : 24;
  const rmt_symbol_word_t one = {.duration0 = static_cast<uint16_t>(t1h_ & 0x7FFF),
                                 .level0 = 1,
                                 .duration1 = static_cast<uint16_t>(t1l_ & 0x7FFF),
                                 .level1 = 0};
  const rmt_symbol_word_t zero = {.duration0 = static_cast<uint16_t>(t0h_ & 0x7FFF),
                                  .level0 = 1,
                                  .duration1 = static_cast<uint16_t>(t0l_ & 0x7FFF),
                                  .level1 = 0};
  for (uint32_t led = 0; led < num_leds_; ++led) {
    const uint32_t color = pixels_[led];
    const auto scale = [this](uint32_t c) {
      return static_cast<uint32_t>((c & 0xFF) * brightness_ / 255);
    };
    const uint32_t r = scale(color >> 16);
    const uint32_t g = scale(color >> 8);
    const uint32_t b = scale(color);

    // WS2812B datasheet, "Composition of 24bit data": G7..G0 R7..R0 B7..B0, high bit first.
    // SK6812RGBW, "Data structure": the white byte follows blue.
    uint32_t bits = (g << 16) | (r << 8) | b;
    if (type_ == LedType::RGBW) {
      bits = (bits << 8) | scale(color >> 24);
    }

    uint32_t mask = 1U << (bits_per_led - 1);
    for (uint32_t bit = 0; bit < bits_per_led; ++bit) {
      buffer_[led * bits_per_led + bit] = (bits & mask) ? one : zero;
      mask >>= 1;
    }
  }

  // Reset code (datasheet "RES"): the line held low so the LEDs latch the frame. WS2812B asks for
  // more than 50 us, WS2812B-V5 for at least 280 us; kResetTicks covers both.
  const uint32_t frame_symbols = num_leds_ * bits_per_led;
  buffer_[frame_symbols] = {
      .duration0 = kResetTicks / 2, .level0 = 0, .duration1 = kResetTicks / 2, .level1 = 0};

  return rmt_.Transmit(buffer_.data(), frame_symbols + 1);
}

void WS2812Strip::SetBrightness(uint8_t value) {
  brightness_ = value;
}

void WS2812Strip::SetNumLeds(uint32_t num_leds) {
  num_leds_ = num_leds;
  pixels_.assign(num_leds, 0);
  buffer_.resize(num_leds * (type_ == LedType::RGBW ? 32 : 24) + 1);
}

void WS2812Strip::SetTimings(uint16_t t0h, uint16_t t1h, uint16_t t0l, uint16_t t1l) {
  t0h_ = t0h;
  t1h_ = t1h;
  t0l_ = t0l;
  t1l_ = t1l;
}

uint32_t WS2812Strip::ColorWheel(uint8_t pos) {
  pos = 255 - pos;
  if (pos < 85) {
    return ((255 - pos * 3) << 16) | (0 << 8) | (pos * 3);
  } else if (pos < 170) {
    pos -= 85;
    return (0 << 16) | ((pos * 3) << 8) | (255 - pos * 3);
  } else {
    pos -= 170;
    return ((pos * 3) << 16) | ((255 - pos * 3) << 8) | 0;
  }
}

#endif // __cplusplus
