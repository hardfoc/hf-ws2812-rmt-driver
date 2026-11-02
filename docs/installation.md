# Installation

Add the driver to an ESP-IDF project. It is an ESP-IDF component; it does not build without ESP-IDF.

## Requirements

| Requirement | Value | Why |
| --- | --- | --- |
| Chip | An ESP32-family chip with an RMT peripheral | The driver transmits through the RMT |
| ESP-IDF | v5.3 or later | The component requires `esp_driver_rmt`, which ESP-IDF split out in v5.3 |
| C++ | C++20 (`-std=gnu++20`, set by the component) | |
| Tested | ESP32-C6, ESP-IDF v5.5 | What the ESP32 examples CI builds |

The manifest [`idf_component.yml`](../idf_component.yml) lists the ESP32, ESP32-S2, ESP32-S3, ESP32-C3, ESP32-C6
and ESP32-H2. All of them have an RMT peripheral, but only the ESP32-C6 is built in CI.

## Get the source

A git submodule keeps the driver pinned to a version you chose:

```bash
git submodule add https://github.com/hardfoc/hf-ws2812-rmt-driver.git external/hf-ws2812-rmt-driver
git submodule update --init external/hf-ws2812-rmt-driver
```

Only `inc/`, `src/`, `cmake/`, `Kconfig` and `CMakeLists.txt` are needed to build. The examples have their own
submodule (`examples/esp32/scripts`); you do not need it to use the driver.

## ESP-IDF component

=== "Component wrapper (recommended)"

    The wrapper in [`examples/esp32/components/hf_ws2812_rmt/`](../examples/esp32/components/hf_ws2812_rmt/CMakeLists.txt)
    is what the ESP32 examples and CI use. Copy the folder into your project's `components/` and tell it where the
    driver is, in your project's top-level `CMakeLists.txt`, before `include($ENV{IDF_PATH}/...)`:

    ```cmake
    set(HF_WS2812_RMT_ROOT ${CMAKE_SOURCE_DIR}/external/hf-ws2812-rmt-driver CACHE PATH "WS2812 driver checkout")
    ```

    The wrapper stops with a clear message when the path is wrong. Your component then declares
    `REQUIRES hf_ws2812_rmt`.

=== "Repository as a component"

    The repository's root [`CMakeLists.txt`](../CMakeLists.txt) registers an ESP-IDF component when
    `ESP_PLATFORM` is set, with the root [`Kconfig`](../Kconfig). Clone or link the repository into
    `components/` (or add its parent folder to `EXTRA_COMPONENT_DIRS`) and require it by its folder name. CI
    does not build this path.

Both register the same sources and the same Kconfig menu, **HF-ESP32-WS2812-RMT Configuration**
([Configuration](configuration.md)), and require the ESP-IDF components `driver`, `freertos` and
`esp_driver_rmt`.

## How the build is organised

| Layer | File | Role |
| --- | --- | --- |
| Settings | [`cmake/hf_ws2812_rmt_build_settings.cmake`](../cmake/hf_ws2812_rmt_build_settings.cmake) | Sources, include directories, ESP-IDF requirements, version: the single source of truth |
| Root | [`CMakeLists.txt`](../CMakeLists.txt) | ESP-IDF component when `ESP_PLATFORM` is set; otherwise a static library for the host tests |
| ESP-IDF | [`examples/esp32/components/hf_ws2812_rmt/CMakeLists.txt`](../examples/esp32/components/hf_ws2812_rmt/CMakeLists.txt) | Component wrapper over the same settings |

Outside ESP-IDF the root `CMakeLists.txt` only builds with `HF_WS2812_RMT_BUILD_TESTS=ON`, which supplies stubs of
the ESP-IDF headers ([Testing and CI](testing.md)). It is not a way to run the driver on a PC.

The version is set once in the settings file and written into `ws2812_version.h` at configure time. Read it at
run time:

```cpp
--8<-- "tests/docs/api_tasks.cpp:version"
```

## Next

[Quickstart](quickstart.md) wires a strip and lights it; [Configuration](configuration.md) lists the Kconfig
options and constructor arguments.
