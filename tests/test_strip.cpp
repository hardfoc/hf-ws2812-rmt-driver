/**
 * @file test_strip.cpp
 * @brief WS2812Strip and ws2812::RmtTx against the fake RMT driver: channel set-up, bit encoding,
 * timing, brightness, RGBW and errors.
 */
#include "fake_rmt.hpp"
#include "test_harness.hpp"
#include "ws2812_cpp.hpp"

#include <cstdint>
#include <vector>

namespace {

constexpr uint16_t kT0H = 4, kT1H = 8, kT0L = 9, kT1L = 5; // 100 ns ticks at 10 MHz

WS2812Strip makeStrip(uint32_t leds, LedType type = LedType::RGB, uint8_t brightness = 255) {
  return WS2812Strip(GPIO_NUM_8, -1, leds, type, kT0H, kT1H, kT0L, kT1L, brightness);
}

/// Decode the transmitted symbols back to bits, `bits` per LED, first-sent bit as the MSB: the
/// wire word, G7 first (0xGGRRBB, or 0xGGRRBBWW for RGBW). The trailing reset symbol is skipped.
std::vector<uint32_t> decode(const std::vector<rmt_symbol_word_t>& symbols, uint32_t bits) {
  std::vector<uint32_t> words;
  for (size_t i = 0; i + bits <= symbols.size(); i += bits) {
    uint32_t w = 0;
    for (uint32_t b = 0; b < bits; ++b) {
      w = (w << 1) | (symbols[i + b].duration0 == kT1H ? 1U : 0U);
    }
    words.push_back(w);
  }
  return words;
}

} // namespace

TEST_CASE("constructor creates a 10 MHz TX channel on the given GPIO") {
  fake_rmt::reset();
  {
    auto strip = makeStrip(4);
    const auto& cfg = fake_rmt::state().last_tx_config;
    CHECK_EQ(fake_rmt::state().channels_created, 1);
    CHECK_EQ(cfg.gpio_num, GPIO_NUM_8);
    CHECK_EQ(cfg.resolution_hz, 10'000'000U);
    CHECK_EQ(cfg.mem_block_symbols, 64U);
    CHECK_EQ(cfg.flags.with_dma, 0U);
    CHECK_EQ(strip.Begin(), ESP_OK);
    CHECK_EQ(strip.Length(), 4U);
  }
  CHECK_EQ(fake_rmt::state().channels_deleted, 1);
}

TEST_CASE("Show sends 24 symbols per RGB LED with the configured timing") {
  fake_rmt::reset();
  auto strip = makeStrip(3);
  strip.SetPixel(0, 0x008001); // R 0x00, G 0x80, B 0x01
  CHECK_EQ(strip.Show(), ESP_OK);
  auto sym = fake_rmt::state().last_symbols;
  REQUIRE(sym.size() == 3 * 24 + 1); // + the reset symbol
  sym.pop_back();
  for (const auto& s : sym) {
    CHECK_EQ(s.level0, 1U);
    CHECK_EQ(s.level1, 0U);
    CHECK(s.duration0 == kT0H || s.duration0 == kT1H);
    CHECK_EQ(s.duration1, s.duration0 == kT1H ? kT1L : kT0L);
  }
  // G7 is the first symbol, B0 the last of the LED.
  CHECK_EQ(sym[0].duration0, kT1H);
  CHECK_EQ(sym[23].duration0, kT1H);
  CHECK_EQ(decode(sym, 24)[0], 0x800001U);
  CHECK_EQ(decode(sym, 24)[1], 0U);
}

// WS2812B datasheet, "Composition of 24bit data": G7..G0, R7..R0, B7..B0, high bit first.
TEST_CASE("Show sends a 0xRRGGBB pixel as green, red, blue") {
  fake_rmt::reset();
  auto strip = makeStrip(2);
  strip.SetPixel(0, 0x112233);
  strip.SetPixel(1, 0xFF0000); // pure red
  REQUIRE(strip.Show() == ESP_OK);
  const auto words = decode(fake_rmt::state().last_symbols, 24);
  REQUIRE(words.size() == 2);
  CHECK_EQ(words[0], 0x221133U);
  CHECK_EQ(words[1], 0x00FF00U);
}

// WS2812B datasheet, "Data transmission time": RES, line low above 50 us (WS2812B-V5 and SK6812:
// 280 us / 80 us). Without it two frames in a row run together and the second one never latches.
TEST_CASE("Show ends every frame with a reset: the line low for at least 280 us") {
  fake_rmt::reset();
  auto strip = makeStrip(1);
  REQUIRE(strip.Show() == ESP_OK);
  const auto& sym = fake_rmt::state().last_symbols;
  REQUIRE(sym.size() == 25U);
  const auto& reset = sym.back();
  CHECK_EQ(reset.level0, 0U);
  CHECK_EQ(reset.level1, 0U);
  CHECK(reset.duration0 + reset.duration1 >= 2800U); // ticks of 100 ns
}

TEST_CASE("brightness scales every channel before encoding") {
  fake_rmt::reset();
  auto strip = makeStrip(1, LedType::RGB, 128);
  strip.SetPixel(0, 0xFF4002);
  REQUIRE(strip.Show() == ESP_OK);
  // 0xFF * 128 / 255 = 0x80, 0x40 * 128 / 255 = 0x20, 2 * 128 / 255 = 1; sent G, R, B
  CHECK_EQ(decode(fake_rmt::state().last_symbols, 24)[0], 0x208001U);
  strip.SetBrightness(0);
  REQUIRE(strip.Show() == ESP_OK);
  CHECK_EQ(decode(fake_rmt::state().last_symbols, 24)[0], 0U);
}

// SK6812RGBW datasheet, "Data structure": G7..G0, R7..R0, B7..B0, W7..W0.
TEST_CASE("RGBW strips send 32 bits per LED: green, red, blue, then white") {
  fake_rmt::reset();
  auto strip = makeStrip(2, LedType::RGBW);
  strip.SetPixel(1, 0x11223344); // W 0x11, R 0x22, G 0x33, B 0x44
  REQUIRE(strip.Show() == ESP_OK);
  const auto words = decode(fake_rmt::state().last_symbols, 32);
  REQUIRE(words.size() == 2);
  CHECK_EQ(words[0], 0U);
  CHECK_EQ(words[1], 0x33224411U);
}

TEST_CASE("SetPixel ignores out-of-range indices; SetNumLeds clears and resizes") {
  fake_rmt::reset();
  auto strip = makeStrip(2);
  strip.SetPixel(2, 0xFFFFFF);
  strip.SetPixel(1, 0x0000FF);
  REQUIRE(strip.Show() == ESP_OK);
  CHECK_EQ(fake_rmt::state().last_symbols.size(), 48U + 1U);
  CHECK_EQ(decode(fake_rmt::state().last_symbols, 24)[1], 0x0000FFU);
  strip.SetNumLeds(5);
  CHECK_EQ(strip.Length(), 5U);
  REQUIRE(strip.Show() == ESP_OK);
  CHECK_EQ(fake_rmt::state().last_symbols.size(), 5U * 24U + 1U);
  for (uint32_t w : decode(fake_rmt::state().last_symbols, 24)) {
    CHECK_EQ(w, 0U);
  }
}

TEST_CASE("SetTimings changes the symbols of the next Show") {
  fake_rmt::reset();
  auto strip = makeStrip(1);
  strip.SetTimings(3, 7, 9, 6);
  strip.SetPixel(0, 0x008000); // green: the first bit on the wire
  REQUIRE(strip.Show() == ESP_OK);
  const auto& sym = fake_rmt::state().last_symbols;
  CHECK_EQ(sym[0].duration0, 7U);
  CHECK_EQ(sym[0].duration1, 6U);
  CHECK_EQ(sym[1].duration0, 3U);
  CHECK_EQ(sym[1].duration1, 9U);
}

TEST_CASE("Show returns the RMT driver's error") {
  fake_rmt::reset();
  auto strip = makeStrip(1);
  fake_rmt::state().transmit_result = ESP_ERR_INVALID_STATE;
  CHECK_EQ(strip.Show(), ESP_ERR_INVALID_STATE);
}

TEST_CASE("ColorWheel walks red, green, blue") {
  CHECK_EQ(WS2812Strip::ColorWheel(0), 0xFF0000U);
  CHECK_EQ(WS2812Strip::ColorWheel(85), 0x00FF00U);
  CHECK_EQ(WS2812Strip::ColorWheel(170), 0x0000FFU);
}

TEST_CASE("RmtTx::TransmitWs2812 uses WS2812B timing at 10 MHz, MSB first") {
  fake_rmt::reset();
  ws2812::RmtTx tx(GPIO_NUM_2);
  const uint8_t grb[3] = {0x10, 0x20, 0x30};
  REQUIRE(tx.TransmitWs2812(grb, sizeof(grb)) == ESP_OK);
  const auto& s = fake_rmt::state();
  CHECK_EQ(s.last_bytes.size(), 3U);
  CHECK_EQ(s.last_bytes_config.flags.msb_first, 1U);
  CHECK_EQ(s.last_bytes_config.bit0.duration0, 4U); // 400 ns
  CHECK_EQ(s.last_bytes_config.bit0.duration1, 9U); // 900 ns
  CHECK_EQ(s.last_bytes_config.bit1.duration0, 8U); // 800 ns
  CHECK_EQ(s.last_bytes_config.bit1.duration1, 5U); // 500 ns
}

// WS2812B datasheet, "Data transfer time": T0H 0.4 us, T1H 0.8 us, T0L 0.85 us, T1L 0.45 us, each
// +-150 ns. The Kconfig defaults are ticks of 100 ns and must land inside those windows.
TEST_CASE("Kconfig default timing is inside the WS2812B windows at 100 ns per tick") {
  auto in = [](unsigned ticks, unsigned nominal_ns) {
    const unsigned ns = ticks * 100;
    return ns + 150 >= nominal_ns && ns <= nominal_ns + 150;
  };
  CHECK(in(WS2812_T0H, 400));
  CHECK(in(WS2812_T1H, 800));
  CHECK(in(WS2812_T0L, 850));
  CHECK(in(WS2812_T1L, 450));

  fake_rmt::reset();
  WS2812Strip strip(GPIO_NUM_8); // every other argument from Kconfig
  strip.SetPixel(0, 0x00FF00);   // green: the first 8 bits on the wire are ones
  REQUIRE(strip.Show() == ESP_OK);
  const auto& sym = fake_rmt::state().last_symbols;
  REQUIRE(sym.size() > 8U);
  CHECK(in(sym[0].duration0, 800));
  CHECK(in(sym[0].duration1, 450));
  CHECK(in(sym[8].duration0, 400));
  CHECK(in(sym[8].duration1, 850));
}

// A strip whose channel cannot be created (no free RMT TX channel: ESP_ERR_NOT_FOUND) used to abort
// in the constructor, and Begin() always returned ESP_OK.
TEST_CASE("a failed channel does not abort: Begin() reports it and retries") {
  fake_rmt::reset();
  fake_rmt::state().new_tx_channel_result = ESP_ERR_NOT_FOUND;
  auto strip = makeStrip(2);
  CHECK_EQ(strip.Begin(), ESP_ERR_NOT_FOUND);
  CHECK_EQ(strip.Show(), ESP_ERR_INVALID_STATE);
  CHECK_EQ(fake_rmt::state().transmits, 0);

  fake_rmt::state().new_tx_channel_result = ESP_OK; // a channel became free
  CHECK_EQ(strip.Begin(), ESP_OK);
  CHECK_EQ(strip.Show(), ESP_OK);
  CHECK_EQ(fake_rmt::state().channels_created, 1);
  CHECK_EQ(strip.Begin(), ESP_OK); // already set up: no second channel
  CHECK_EQ(fake_rmt::state().channels_created, 1);
}

TEST_CASE("RmtTx reports its init status") {
  fake_rmt::reset();
  fake_rmt::state().new_tx_channel_result = ESP_ERR_NO_MEM;
  ws2812::RmtTx tx(GPIO_NUM_2);
  CHECK(!tx.IsReady());
  CHECK_EQ(tx.InitStatus(), ESP_ERR_NO_MEM);
  const uint8_t grb[3] = {1, 2, 3};
  CHECK_EQ(tx.TransmitWs2812(grb, sizeof(grb)), ESP_ERR_INVALID_STATE);
  fake_rmt::state().new_tx_channel_result = ESP_OK;
  CHECK_EQ(tx.Init(), ESP_OK);
  CHECK(tx.IsReady());
  CHECK_EQ(tx.InitStatus(), ESP_OK);
}
