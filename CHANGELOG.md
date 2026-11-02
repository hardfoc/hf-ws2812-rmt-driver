# Changelog

All notable changes to this driver are listed here. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/) and the project uses
[Semantic Versioning](https://semver.org/).

## [Unreleased]

### Added

- Host unit tests (`-DHF_WS2812_RMT_BUILD_TESTS=ON`) that build the driver sources against stubs of the ESP-IDF
  headers and a recording fake of the RMT driver, and a CI workflow that runs them with GCC and Clang.

### Changed

- Documentation moved to the HardFOC MkDocs platform: new page set (quickstart, hardware, architecture, signal
  timing, API guide, configuration, porting, testing), code snippets compiled by the host build from
  `tests/docs/`, Doxygen under `/api/`, versioned with mike. The Jekyll site files and the doxygen-awesome
  submodule are gone.
- The ESP32 example's description and README describe the test application as it is (a scaffold).

## 1.0.0 (version in the build settings; not tagged)

- `WS2812Strip` over the ESP-IDF v5 RMT driver, RGB and RGBW, brightness, run-time timing and length.
- `WS2812Animator` effects and `WS2812MultiAnimator`; the C interface in `ws2812_control.h`.
