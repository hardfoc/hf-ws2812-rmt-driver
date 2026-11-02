# Examples

The ESP-IDF test application in `examples/esp32`, how to build it, and where to look for working code.

The application targets an ESP32-C6. CI builds every app below on each pull request.

## The apps

<!-- generated: examples -->
| App | What it shows | Source | CI |
| --- | --- | --- | --- |
| `ws2812_comprehensive_test` :material-star:{ title="featured" } | Test application scaffold: prints the driver version and runs a placeholder test; no LED output yet | [`WS2812ComprehensiveTest.cpp`](../examples/esp32/main/WS2812ComprehensiveTest.cpp) | yes |

Build and flash one (from `examples/esp32`, ESP-IDF environment loaded):

```bash
./scripts/build_app.sh ws2812_comprehensive_test Release
./scripts/flash_app.sh flash_monitor ws2812_comprehensive_test Release
```

Source of this table: [`examples/esp32/app_config.yml`](../examples/esp32/app_config.yml).
<!-- /generated -->

## Set up once

```bash
git submodule update --init examples/esp32/scripts   # the HardFOC ESP-IDF build scripts
cd examples/esp32
. $IDF_PATH/export.sh                                 # ESP-IDF v5.5
./scripts/build_app.sh list                           # every app and build type
```

The scripts read [`app_config.yml`](../examples/esp32/app_config.yml) (the same file this page's table comes
from) and build each app as its own firmware image.

## What to read

The test application is a scaffold: it starts, prints the driver version and runs a placeholder test, but it does
not drive LEDs yet. Until it does, these are the working references:

| You want to | Read |
| --- | --- |
| Light a strip with the fewest lines | [`tests/docs/quickstart.cpp`](../tests/docs/quickstart.cpp) ([Quickstart](quickstart.md)) |
| Set pixels, dim, resize, animate, run two strips | [`tests/docs/api_tasks.cpp`](../tests/docs/api_tasks.cpp) ([API guide](api-guide.md)) |
| See exactly which pulses `Show()` sends | [`tests/test_strip.cpp`](../tests/test_strip.cpp) ([Testing and CI](testing.md)) |
| See what each effect draws per `Tick()` | [`tests/test_effects.cpp`](../tests/test_effects.cpp) |
| Copy the ESP-IDF component wrapper | [`examples/esp32/components/hf_ws2812_rmt/`](../examples/esp32/components/hf_ws2812_rmt/CMakeLists.txt) |
