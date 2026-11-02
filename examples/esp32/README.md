# ESP32 examples

The ESP-IDF test application for the WS2812 RMT driver, built for the ESP32-C6.

## The application

| App | Source | State |
| --- | --- | --- |
| `ws2812_comprehensive_test` | [`main/WS2812ComprehensiveTest.cpp`](main/WS2812ComprehensiveTest.cpp) | Hardware test on GPIO8: `Begin()`, red, green, blue, white, a walking pixel, a brightness ramp, the rainbow animator and a second strip on GPIO4; GPIO14 toggles as a progress indicator |

Connect a WS2812B strip (16 LEDs by default) to GPIO8 through a level shifter and watch the colours the log
names. The compiled snippets in [`tests/docs/`](../../tests/docs/api_tasks.cpp) show the rest of the API, and the
host tests in [`tests/`](../../tests/test_strip.cpp) check what the driver transmits.

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
| `main/esp32_ws2812_test_config.hpp` | Pin, strip and timing settings for the test board (GPIO8 and GPIO4, 16 LEDs, WS2812B timing) |
| `main/TestFramework.h` | Test runner, GPIO14 progress indicator and result summary |
| `sdkconfig` | ESP32-C6 configuration, Kconfig defaults of the driver |

The driver documentation: <https://hardfoc.github.io/hf-ws2812-rmt-driver/latest/>.
