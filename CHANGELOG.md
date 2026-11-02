# Changelog

All notable changes to this driver are listed here. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/) and the project uses
[Semantic Versioning](https://semver.org/).

## [Unreleased]

### Breaking

- **`WS2812Strip::Show()` sends green first.** Pixels are `0xRRGGBB` (`0xWWRRGGBB` for RGBW) and go out in the
  WS2812B order G, R, B (then W, the SK6812RGBW order). Before, the packed value went out as it was, so red came
  first and RGBW sent white first. Code that packed `0xGGRRBB` to work around it now swaps red and green: pack
  `0xRRGGBB` and drop the workaround. `ColorWheel()` and the effects now show their intended colours.
- **Kconfig timing defaults are WS2812B values in 100 ns ticks** (`CONFIG_WS2812_T0H/T1H/T0L/T1L` = 4, 8, 9, 5,
  was 14, 52, 52, 52, which a WS2812B read as all ones). Constructor calls without timing arguments and the C
  interface change output. A project `sdkconfig` keeps old values until you edit it.
- **`ws2812::RmtTx` and `WS2812Strip` no longer abort when the RMT channel cannot be created.**
  `WS2812Strip::Begin()` returns the error (and retries) instead of always `ESP_OK`; until it succeeds `Show()`
  returns `ESP_ERR_INVALID_STATE`. Check `Begin()` where you relied on the abort.
- **`ws2812_control_init()`** returns `ESP_ERR_INVALID_STATE` when called a second time (it used to leak a
  channel), and `ws2812_write_leds()` before init returns `ESP_ERR_INVALID_STATE`.

### Fixed

- `ws2812_write_leds()` sent the bytes of its RMT symbol buffer through the byte encoder instead of the colours;
  it now sends the pixel bytes in wire order with the Kconfig timing and a reset.
- `Show()` sends a reset (line low for `WS2812Strip::kResetTimeUs` = 300 µs, WS2812B-V5 needs at least 280 µs)
  after every frame, so back-to-back frames latch.
- Each strip requests one RMT memory block of the target (`SOC_RMT_MEM_WORDS_PER_CHANNEL`: 48 symbols on S3, C3,
  C6, H2) instead of 64 symbols, which took two blocks there and left no memory for a second strip on the
  two-channel chips. Override with `WS2812_RMT_MEM_BLOCK_SYMBOLS`.
- `ws2812_control_init()` releases the channel and encoder when a later step fails.
- CMake: install rules for the library, headers and generated version header, an exported `hf::hf_ws2812_rmt`
  target and a package config that works after install (it included an uninstalled file); the project version
  comes from the build settings.
- The ESP32 example is a real hardware test (colours, brightness, animation, two strips) instead of a
  placeholder; its config header compiles.

### Added

- `ws2812::RmtTx::Init()`, `InitStatus()`, `IsReady()`; `WS2812Strip::kResetTimeUs`; `ws2812_rmt_caps.h`.
- `led_strip_encoder_config_t` takes optional bit times and reset length (0 keeps the WS2812B defaults, now
  400/850/800/450 ns and 300 µs instead of 300/900/900/300 ns and 50 µs).
- Host tests for the colour order, reset, Kconfig timing, channel memory, set-up failures and the C interface; CI
  installs the package and runs a `find_package` consumer.

- Host unit tests (`-DHF_WS2812_RMT_BUILD_TESTS=ON`) that build the driver sources against stubs of the ESP-IDF
  headers and a recording fake of the RMT driver, and a CI workflow that runs them with GCC and Clang.

### Changed

- Documentation moved to the HardFOC MkDocs platform: new page set (quickstart, hardware, architecture, signal
  timing, API guide, configuration, porting, testing), code snippets compiled by the host build from
  `tests/docs/`, Doxygen under `/api/`, versioned with mike. The Jekyll site files and the doxygen-awesome
  submodule are gone.
- The ESP32 example's description and README describe the test application as it is.
- The Kconfig help says `CONFIG_WS2812_LED_RMT_TX_CHANNEL` is unused: ESP-IDF v5 allocates RMT channels itself.

## 1.0.0 (version in the build settings; not tagged)

- `WS2812Strip` over the ESP-IDF v5 RMT driver, RGB and RGBW, brightness, run-time timing and length.
- `WS2812Animator` effects and `WS2812MultiAnimator`; the C interface in `ws2812_control.h`.
