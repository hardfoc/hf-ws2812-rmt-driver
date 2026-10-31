---
layout: default
title: "🛠️ Installation"
description: "Installation and integration instructions for the WS2812 driver"
nav_order: 1
parent: "📚 Documentation"
permalink: /docs/installation/
---

# Installation

This guide covers how to integrate the WS2812 driver into your ESP-IDF project.

## Prerequisites

Before installing the driver, ensure you have:

- **ESP-IDF**: Version 5.0 or later installed and configured
- **ESP32 Target**: ESP32, ESP32-S2, ESP32-S3, ESP32-C3, or ESP32-C6
- **Project Setup**: An existing ESP-IDF project

## Integration Methods

### Option 1: ESP-IDF Component (Recommended)

1. **Add the component** to your project's `components/` directory:

   ```bash
   cd your-project
   git clone https://github.com/hardfoc/hf-ws2812-rmt-driver.git components/hf-ws2812-rmt
   ```

   Or copy the entire driver directory into `components/hf-ws2812-rmt/`.

2. **Configure via menuconfig**:

   ```bash
   idf.py menuconfig
   ```

   Navigate to: **Component Config** → **HF-ESP32-WS2812-RMT**

   Configure:
   - LED Type (RGB or RGBW)
   - Number of LEDs
   - GPIO pin
   - RMT channel
   - Timing parameters
   - Default brightness

3. **Include in your component's CMakeLists.txt**:

   ```cmake
   idf_component_register(
       REQUIRES hf-ws2812-rmt
       # ... other settings
   )
   ```

### Option 2: Manual Integration

1. **Copy source files** into your project:

   ```
   your-project/
   ├── components/
   │   └── hf-ws2812-rmt/
   │       ├── inc/
   │       │   ├── ws2812_control.h
   │       │   ├── ws2812_cpp.hpp
   │       │   ├── ws2812_effects.hpp
   │       │   └── ...
   │       ├── src/
   │       │   ├── ws2812_control.c
   │       │   ├── ws2812_cpp.cpp
   │       │   └── ...
   │       └── CMakeLists.txt
   ```

2. **Create CMakeLists.txt**:

   ```cmake
   idf_component_register(
       SRCS "src/ws2812_control.c"
            "src/ws2812_cpp.cpp"
            "src/ws2812_effects.cpp"
            "src/ws2812_multi_animator.cpp"
            "src/led_strip_encoder.c"
       INCLUDE_DIRS "inc"
       REQUIRES driver
   )
   ```

## Including Headers

### C API

```c
#include "ws2812_control.h"
```

### C++ API

```cpp
#include "ws2812_cpp.hpp"        // Main C++ wrapper
#include "ws2812_effects.hpp"    // Animation effects
#include "ws2812_multi_animator.hpp"  // Multi-strip animations
```

## Verification

To verify the installation:

1. **Check compilation**:
   ```bash
   idf.py build
   ```

2. **Test include**:
   ```c
   #include "ws2812_control.h"
   
   void app_main(void) {
       // If this compiles, installation is successful
   }
   ```

## Next Steps

- Follow the [Quick Start](quickstart.md) guide to create your first application
- Review [Hardware Setup](hardware_setup.md) for wiring instructions
- Check [Configuration](configuration.md) for customization options

---

**Navigation**
⬅️ [Back to Index](index.md) | [Next: Quick Start ➡️](quickstart.md)

