# Testing and CI

How the driver is checked: host unit tests over a fake of ESP-IDF's RMT driver, compiled documentation, the
ESP32 build and static analysis, on every pull request.

## Host unit tests

The driver is ESP-IDF code, so a PC cannot compile it as it is. The host build adds
[`tests/stubs/`](../tests/stubs/README.md) to the include path: minimal copies of the ESP-IDF headers the driver
includes (`driver/rmt_*.h`, `driver/gpio.h`, `esp_err.h`, `esp_check.h`, `esp_log.h`, FreeRTOS, `sdkconfig.h`),
with ESP-IDF's names and its RMT symbol layout. [`fake_rmt.cpp`](../tests/stubs/fake_rmt.cpp) implements the RMT
calls by recording the channel configuration and every symbol or byte the driver transmits. The real driver
sources (`src/*.cpp` and `src/*.c`) build against them unchanged.

```bash
cmake -S . -B build-host -DHF_WS2812_RMT_BUILD_TESTS=ON
cmake --build build-host
ctest --test-dir build-host --output-on-failure
```

| File | Covers |
| --- | --- |
| [`tests/test_strip.cpp`](../tests/test_strip.cpp) | Channel set-up (GPIO, 10 MHz, memory), 24 and 32 symbols per LED, the timing of each symbol, MSB-first order, brightness scaling, resizing, errors from the RMT driver, `ColorWheel()`, `RmtTx::TransmitWs2812()` timing |
| [`tests/test_effects.cpp`](../tests/test_effects.cpp) | What `SolidColor`, `Chase`, `Larson` and `Off` transmit per `Tick()`, and the shared step of `WS2812MultiAnimator` |
| [`tests/stubs/`](../tests/stubs/README.md) | The ESP-IDF stubs and the recording RMT fake |

The stubs check that the driver uses ESP-IDF's API correctly as far as names and types go; they cannot check
the RMT hardware. That is what the ESP32 build and a strip on a bench are for.

## Documentation snippets

Every C++ block on this site is included from [`tests/docs/`](../tests/docs/api_tasks.cpp), which the host build
compiles against the same stubs (target `hf_ws2812_rmt_doc_snippets`, `-Wall -Wextra`). A change to the API that
breaks a page breaks the build first. The README's example is a copy of the Quickstart snippet that
`hardfoc-docs check` keeps in step.

## CI workflows

| Workflow | What it checks |
| --- | --- |
| Host unit tests | GCC and Clang build and run the tests and the doc snippets |
| ESP32 examples build | Every app in `examples/esp32/app_config.yml` with ESP-IDF, for each listed version and build type |
| C++ lint, C++ analysis | clang-format, clang-tidy and cppcheck |
| Docs | Strict site build, figure and generated-table drift, Doxygen, link check, Lighthouse budget |
| Markdown lint, YAML lint, link check | Repository text files |

The workflows call the shared, versioned workflows of
[hf-general-ci-tools](https://github.com/hardfoc/hf-general-ci-tools) and
[hf-espidf-ci-tools](https://github.com/hardfoc/hf-espidf-ci-tools).

## On hardware

The [ESP32 test application](examples.md) builds in CI but does not drive LEDs yet. Until it does, check a
change on a strip with the [Quickstart](quickstart.md) example, and look at the data line with a logic analyser:
the high times should be 400 ns and 800 ns with the WS2812B timing.
