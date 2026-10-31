---
layout: default
title: "HardFOC WS2812 Driver"
description: "ESP32 RMT-based driver for WS2812 / NeoPixel addressable RGB(W) LED strips"
nav_order: 1
permalink: /
---

# HF-WS2812 Driver
**ESP32 RMT-based driver for WS2812 / NeoPixel addressable RGB(W) LED strips**

This repository is an **ESP-IDF RMT backend**, not a portable CRTP chip
driver. `inc/` includes ESP-IDF headers on purpose. Other `hf-*-driver`
trees must not copy that pattern.

[![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![License](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![CI](https://github.com/hardfoc/hf-ws2812-rmt-driver/actions/workflows/esp32-examples-build-ci.yml/badge.svg?branch=main)](https://github.com/hardfoc/hf-ws2812-rmt-driver/actions/workflows/esp32-examples-build-ci.yml)
[![Docs](https://img.shields.io/badge/docs-GitHub%20Pages-blue)](https://hardfoc.github.io/hf-ws2812-rmt-driver/)

## 📚 Table of Contents
1. [Overview](#-overview)
2. [Features](#-features)
3. [Quick Start](#-quick-start)
4. [Installation](#-installation)
5. [API Reference](#-api-reference)
6. [Examples](#-examples)
7. [Documentation](#-documentation)
8. [References](#-references)
9. [Contributing](#-contributing)
10. [License](#-license)

## 📦 Overview

> **📖 [📚🌐 Live Complete Documentation](https://hardfoc.github.io/hf-ws2812-rmt-driver/)** - 
> Interactive guides, examples, and step-by-step tutorials

**HF-WS2812** is an ESP32-specific driver for **WS2812** and compatible addressable RGB(W) LED strips (commonly known as NeoPixels). The driver uses the ESP32's RMT (Remote Control) peripheral to generate the precise timing signals required by WS2812 LEDs, providing hardware-accelerated LED control with minimal CPU overhead.

The driver supports both C and C++ APIs, includes built-in animation effects, and is fully configurable via ESP-IDF's Kconfig system. It's compatible with WS2812, WS2812B, WS2813, SK6812, and other WS2812-compatible LED variants.

![WS2812 NRZ bit-encoding and 24-bit pixel cascade](docs/assets/ws2812-nrz.svg)

### 🔀 Chip Compatibility

All variants share the same single-wire NRZ protocol with chip-specific bit timings.
The driver exposes per-LED-type timing presets via the `LedType` parameter and lets
you override each pulse width for non-standard parts.

| Chip family | Channels | Bits / LED | T0H / T0L (ns, typ.) | T1H / T1L (ns, typ.) | Reset | Notes |
|-------------|----------|------------|----------------------|----------------------|-------|-------|
| WS2812      | RGB      | 24         | 350 / 800            | 700 / 600            | ≥ 50 µs | Original NeoPixel |
| WS2812B     | RGB      | 24         | 400 / 850            | 800 / 450            | ≥ 50 µs | Most common variant |
| WS2813      | RGB      | 24         | 300 / 300            | 750 / 300            | ≥ 250 µs | Backup data line for fault tolerance |
| SK6812 RGBW | RGBW     | 32         | 300 / 900            | 600 / 600            | ≥ 80 µs | Adds dedicated white LED |
| SK6812 RGB  | RGB      | 24         | 300 / 900            | 600 / 600            | ≥ 80 µs | Drop-in WS2812B alternative |

For non-standard parts, override timings via Kconfig (`Component Config → HF-ESP32-WS2812-RMT`).

## ✨ Features

- ✅ **Hardware-Accelerated**: Uses ESP32 RMT peripheral for precise timing
- ✅ **Dual API**: Both C and C++ interfaces available
- ✅ **Animation Effects**: Built-in effects library (rainbow, chase, fade, etc.)
- ✅ **RGB and RGBW Support**: 24-bit RGB and 32-bit RGBW LED types
- ✅ **Configurable Timing**: Adjustable timing parameters for different LED variants
- ✅ **Brightness Control**: Global brightness scaling (0-255)
- ✅ **Kconfig Integration**: Full ESP-IDF Kconfig configuration support
- ✅ **Zero CPU Overhead**: RMT handles signal generation automatically

## 🚀 Quick Start

```cpp
#include "ws2812_cpp.hpp"
#include "ws2812_effects.hpp"

// Create strip: GPIO, RMT channel, LED count, LED type
WS2812Strip strip(GPIO_NUM_18, 0, 30, LedType::RGB);
WS2812Animator anim(strip);

void app_main(void) {
    strip.begin();
    anim.setEffect(WS2812Animator::Effect::Rainbow);
    
    while (true) {
        anim.tick();
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
```

For detailed setup, see [Installation](docs/installation.md) and [Quick Start Guide](docs/quickstart.md).

## 🔧 Installation

1. **Add as ESP-IDF component** to your project
2. **Configure via Kconfig**: Run `idf.py menuconfig` → Component Config → HF-ESP32-WS2812-RMT
3. **Include the header** in your code:
   ```cpp
   #include "ws2812_cpp.hpp"  // C++ API
   // or
   #include "ws2812_control.h"  // C API
   ```

For detailed installation instructions, see [docs/installation.md](docs/installation.md).

## 📖 API Reference

| Method | Description |
|--------|-------------|
| `begin()` | Initialize the LED strip |
| `setPixel()` | Set color of a single LED |
| `show()` | Send colors to LED strip |
| `setBrightness()` | Set global brightness |
| `setEffect()` | Set animation effect |

For complete API documentation, see [docs/api_reference.md](docs/api_reference.md).

## 📊 Examples

For ESP32 examples, see the [examples/esp32](examples/esp32/) directory.

Detailed example walkthroughs are available in [docs/examples.md](docs/examples.md).

## 📚 Documentation

For complete documentation, see the [docs directory](docs/index.md).

## 🔗 References

| Resource | Link |
|----------|------|
| WS2812B datasheet (Worldsemi) | <https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf> |
| WS2813 datasheet (Worldsemi) | <https://cdn-shop.adafruit.com/product-files/3777/3777_datasheet.pdf> |
| SK6812 (RGBW) datasheet | <https://cdn-shop.adafruit.com/product-files/2757/p2757_SK6812RGBW_REV01.pdf> |
| ESP-IDF RMT peripheral | <https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/rmt.html> |
| Adafruit NeoPixel Überguide | <https://learn.adafruit.com/adafruit-neopixel-uberguide> |
| C++17 language reference | <https://en.cppreference.com/w/cpp/17> |

## 🤝 Contributing

Pull requests and suggestions are welcome! Please follow the existing code style and include tests for new features.

## 📄 License

This project is licensed under the **GNU General Public License v3.0**.
See the [LICENSE](LICENSE) file for details.

