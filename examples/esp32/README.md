# ESP32 examples

The ESP-IDF test application for the WS2812 RMT driver, built for the ESP32-C6.

## The application

| App | Source | State |
| --- | --- | --- |
| `ws2812_comprehensive_test` | [`main/WS2812ComprehensiveTest.cpp`](main/WS2812ComprehensiveTest.cpp) | Scaffold: prints the driver version, runs one placeholder test, toggles GPIO14 as a progress indicator |

The app does not drive LEDs yet. Until it does, the compiled snippets in [`tests/docs/`](../../tests/docs/api_tasks.cpp)
are the reference for using the driver, and the host tests in [`tests/`](../../tests/test_strip.cpp) check what it
transmits.

## Build and flash

```bash
git submodule update --init scripts          # the HardFOC ESP-IDF build scripts
. $IDF_PATH/export.sh                         # ESP-IDF v5.5
./scripts/build_app.sh list
./scripts/build_app.sh ws2812_comprehensive_test Release
./scripts/flash_app.sh flash_monitor ws2812_comprehensive_test Release
```

The apps, ESP-IDF versions and build types are listed in [`app_config.yml`](app_config.yml).

## Files

| File | What it is |
| --- | --- |
| `components/hf_ws2812_rmt/` | ESP-IDF component wrapper over the driver (the one to copy into a project) |
| `main/esp32_ws2812_test_config.hpp` | Pin and strip settings for the test board (GPIO8, 16 LEDs); not included by the app yet |
| `main/TestFramework.h` | Test runner, GPIO14 progress indicator and result summary |
| `sdkconfig` | ESP32-C6 configuration, Kconfig defaults of the driver |

The driver documentation: <https://hardfoc.github.io/hf-ws2812-rmt-driver/latest/>.
