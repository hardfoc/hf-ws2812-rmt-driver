# Quick Start

This guide will get you up and running with the WS2812 driver in just a few steps.

## Prerequisites

- [Driver installed](installation.md)
- [Hardware wired](hardware_setup.md)
- ESP-IDF project configured

## Minimal Example (C++)

Here's a complete working example using the C++ API:

```cpp
#include "ws2812_cpp.hpp"
#include "ws2812_effects.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Create strip: GPIO, RMT channel, LED count, LED type
WS2812Strip strip(GPIO_NUM_18, 0, 30, LedType::RGB);
WS2812Animator anim(strip);

void app_main(void) {
    // Initialize the strip
    strip.begin();
    
    // Set rainbow effect
    anim.setEffect(WS2812Animator::Effect::Rainbow);
    
    // Update animation in loop
    while (true) {
        anim.tick();
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
```

## Minimal Example (C API)

Here's the same example using the C API:

```c
#include "ws2812_control.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main(void) {
    // Initialize RMT hardware
    ws2812ControlInit(GPIO_NUM_18, 0);
    
    // Create LED state
    struct led_state state = {0};
    
    // Set all LEDs to red
    for (int i = 0; i < NUM_LEDS; i++) {
        state.leds[i] = 0xFF0000; // RGB: Red
    }
    
    // Send to LEDs
    ws2812WriteLeds(state);
}
```

## Step-by-Step Explanation

### Step 1: Include Headers

```cpp
#include "ws2812_cpp.hpp"      // C++ API
#include "ws2812_effects.hpp"  // Animation effects
```

### Step 2: Create Strip Instance

```cpp
WS2812Strip strip(GPIO_NUM_18, 0, 30, LedType::RGB);
```

Parameters:
- `GPIO_NUM_18`: GPIO pin connected to LED strip data line
- `0`: RMT channel (0-7, or -1 for auto)
- `30`: Number of LEDs in the strip
- `LedType::RGB`: LED type (RGB or RGBW)

### Step 3: Initialize

```cpp
strip.begin();
```

This initializes the RMT peripheral and prepares it for transmission.

### Step 4: Control LEDs

**Set individual LED colors:**
```cpp
strip.setPixel(0, 0xFF0000);  // LED 0: Red
strip.setPixel(1, 0x00FF00);  // LED 1: Green
strip.setPixel(2, 0x0000FF);  // LED 2: Blue
strip.show();                 // Send to strip
```

**Use animations:**
```cpp
WS2812Animator anim(strip);
anim.setEffect(WS2812Animator::Effect::Rainbow);
anim.tick();  // Call periodically to update
```

## Expected Output

When running the rainbow example:
- LEDs should display a moving rainbow pattern
- Colors should smoothly transition
- No error messages should appear

## Troubleshooting

If you encounter issues:

- **No LEDs light up**: Check GPIO pin and wiring
- **Wrong colors**: Verify LED type (RGB vs RGBW) matches configuration
- **Timing issues**: Try adjusting timing parameters or pin task to Core 1
- **See**: [Troubleshooting](troubleshooting.md) for common issues

## Next Steps

- Explore [Examples](examples.md) for more advanced usage
- Review the [API Reference](api_reference.md) for all available methods
- Check [Configuration](configuration.md) for customization options

---

**Navigation**
⬅️ [Installation](installation.md) | [Next: Hardware Setup ➡️](hardware_setup.md) | [Back to Index](index.md)

