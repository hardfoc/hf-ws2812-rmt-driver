/**
 * @file test_c_api.cpp
 * @brief The C API (ws2812_control.h) against the fake RMT driver: the waveform of
 * ws2812_write_leds() decoded back to colours.
 */
#include "fake_rmt.hpp"
#include "test_harness.hpp"
#include "ws2812_control.h"

#include <cstdint>
#include <vector>

namespace {

/// The 24-bit wire words of the last transmit (first-sent bit as the MSB), `count` LEDs.
std::vector<uint32_t> wireWords(size_t count) {
  const auto& sym = fake_rmt::state().last_symbols;
  std::vector<uint32_t> words;
  for (size_t led = 0; led < count && (led + 1) * 24 <= sym.size(); ++led) {
    uint32_t w = 0;
    for (size_t b = 0; b < 24; ++b) {
      w = (w << 1) | (sym[led * 24 + b].duration0 == WS2812_T1H ? 1U : 0U);
    }
    words.push_back(w);
  }
  return words;
}

} // namespace

// The C API keeps one strip in static storage, so these checks run in order in one test case.
TEST_CASE("C API: ws2812_write_leds sends the colours, green first, then a reset") {
  fake_rmt::reset();
  led_state state{};
  CHECK_EQ(ws2812_write_leds(state), ESP_ERR_INVALID_STATE); // before init

  REQUIRE(ws2812_control_init(GPIO_NUM_8, -1) == ESP_OK);
  CHECK_EQ(fake_rmt::state().last_tx_config.gpio_num, GPIO_NUM_8);
  CHECK_EQ(fake_rmt::state().last_tx_config.resolution_hz, 10'000'000U);
  CHECK_EQ(fake_rmt::state().last_tx_config.mem_block_symbols, 48U);    // one C6 memory block
  CHECK_EQ(ws2812_control_init(GPIO_NUM_8, -1), ESP_ERR_INVALID_STATE); // no second channel
  CHECK_EQ(fake_rmt::state().channels_created, 1);

  state.leds[0] = 0x112233; // R 0x11, G 0x22, B 0x33
  state.leds[1] = 0xFF0000; // red
  REQUIRE(ws2812_write_leds(state) == ESP_OK);

  // WS2812B datasheet, "Composition of 24bit data": G7..G0 R7..R0 B7..B0.
  const auto& s = fake_rmt::state();
  CHECK_EQ(s.last_bytes.size(), static_cast<size_t>(NUM_LEDS) * 3U);
  REQUIRE(s.last_symbols.size() == static_cast<size_t>(NUM_LEDS) * 24U + 1U);
  const auto words = wireWords(2);
  REQUIRE(words.size() == 2);
  CHECK_EQ(words[0], 0x221133U);
  CHECK_EQ(words[1], 0x00FF00U);

  // Bit timing from Kconfig (100 ns ticks).
  CHECK_EQ(s.last_symbols[0].duration0, static_cast<unsigned>(WS2812_T0H));
  CHECK_EQ(s.last_symbols[0].duration1, static_cast<unsigned>(WS2812_T0L));
  CHECK_EQ(s.last_symbols[24 + 8].duration0, static_cast<unsigned>(WS2812_T1H));
  CHECK_EQ(s.last_symbols[24 + 8].duration1, static_cast<unsigned>(WS2812_T1L));

  // RES: line low for at least 280 us (WS2812B-V5).
  const auto& reset = s.last_symbols.back();
  CHECK_EQ(reset.level0, 0U);
  CHECK_EQ(reset.level1, 0U);
  CHECK(reset.duration0 + reset.duration1 >= 2800);

  ws2812_set_brightness(128);
  state.leds[0] = 0x00FF00;
  REQUIRE(ws2812_write_leds(state) == ESP_OK);
  CHECK_EQ(wireWords(1)[0], 0x800000U); // 0xFF * 128 / 255
  ws2812_set_brightness(255);
}
