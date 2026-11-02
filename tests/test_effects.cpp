/**
 * @file test_effects.cpp
 * @brief WS2812Animator and WS2812MultiAnimator over the fake RMT driver: what each Tick()
 * transmits.
 */
#include "fake_rmt.hpp"
#include "test_harness.hpp"
#include "ws2812_multi_animator.hpp"

#include <vector>

namespace {

constexpr uint16_t kT1H = 8;

std::vector<uint32_t> lastFrame() {
  const auto& sym = fake_rmt::state().last_symbols;
  std::vector<uint32_t> words;
  for (size_t i = 0; i + 24 <= sym.size(); i += 24) {
    uint32_t w = 0;
    for (size_t b = 0; b < 24; ++b) {
      w = (w << 1) | (sym[i + b].duration0 == kT1H ? 1U : 0U);
    }
    words.push_back(w);
  }
  return words;
}

WS2812Strip makeStrip(uint32_t leds) {
  return WS2812Strip(GPIO_NUM_8, -1, leds, LedType::RGB, 4, kT1H, 9, 5, 255);
}

} // namespace

TEST_CASE("SolidColor fills the strip and transmits once per Tick") {
  fake_rmt::reset();
  auto strip = makeStrip(4);
  WS2812Animator anim(strip);
  anim.SetEffect(WS2812Animator::Effect::SolidColor, 0x00FF00);
  anim.Tick();
  CHECK_EQ(fake_rmt::state().transmits, 1);
  for (uint32_t w : lastFrame()) {
    CHECK_EQ(w, 0x00FF00U);
  }
}

TEST_CASE("Chase lights every third LED and moves by one per Tick") {
  fake_rmt::reset();
  auto strip = makeStrip(6);
  WS2812Animator anim(strip);
  anim.SetEffect(WS2812Animator::Effect::Chase, 0x0000FF);
  anim.Tick();
  auto f = lastFrame();
  CHECK_EQ(f[0], 0x0000FFU);
  CHECK_EQ(f[1], 0U);
  CHECK_EQ(f[3], 0x0000FFU);
  anim.Tick();
  f = lastFrame();
  CHECK_EQ(f[0], 0U);
  CHECK_EQ(f[2], 0x0000FFU);
}

TEST_CASE("Larson moves one lit LED and bounces at the ends") {
  fake_rmt::reset();
  auto strip = makeStrip(3);
  WS2812Animator anim(strip);
  anim.SetEffect(WS2812Animator::Effect::Larson, 0xFF0000);
  std::vector<uint16_t> positions;
  for (int i = 0; i < 5; ++i) {
    positions.push_back(anim.Step());
    anim.Tick();
  }
  CHECK(positions == (std::vector<uint16_t>{0, 1, 2, 1, 0}));
}

TEST_CASE("Off clears the strip") {
  fake_rmt::reset();
  auto strip = makeStrip(2);
  strip.SetPixel(0, 0xFFFFFF);
  WS2812Animator anim(strip);
  anim.SetEffect(WS2812Animator::Effect::Off);
  anim.Tick();
  for (uint32_t w : lastFrame()) {
    CHECK_EQ(w, 0U);
  }
}

TEST_CASE("MultiAnimator ticks every strip with one shared step") {
  fake_rmt::reset();
  auto a = makeStrip(2);
  auto b = makeStrip(4);
  WS2812MultiAnimator multi({&a, &b});
  multi.SetEffect(WS2812Animator::Effect::Blink, 0x010203);
  multi.Tick();
  CHECK_EQ(fake_rmt::state().transmits, 2);
  multi.Tick();
  CHECK_EQ(fake_rmt::state().transmits, 4);
  // Blink lights on odd steps: the second Tick ran step 1 on both strips, so the last frame (strip
  // b) is lit.
  for (uint32_t w : lastFrame()) {
    CHECK_EQ(w, 0x010203U);
  }
}
