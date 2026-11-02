# Examples

This guide provides complete, working examples demonstrating various use cases for the WS2812 driver.

## Example 1: Basic LED Control

This example shows how to set individual LED colors.

```cpp
#include "ws2812_cpp.hpp"

void app_main(void) {
    WS2812Strip strip(GPIO_NUM_18, 0, 30, LedType::RGB);
    strip.begin();
    
    // Set LEDs to different colors
    strip.setPixel(0, 0xFF0000);  // Red
    strip.setPixel(1, 0x00FF00);  // Green
    strip.setPixel(2, 0x0000FF);  // Blue
    
    // Send to strip
    strip.show();
}
```

### Explanation

1. **Create strip**: Configure GPIO, channel, LED count, and type
2. **Initialize**: Call `begin()` to start RMT peripheral
3. **Set colors**: Use `setPixel()` to set individual LED colors
4. **Update**: Call `show()` to transmit colors to the strip

---

## Example 2: Rainbow Animation

This example demonstrates the built-in rainbow effect.

```cpp
#include "ws2812_cpp.hpp"
#include "ws2812_effects.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main(void) {
    WS2812Strip strip(GPIO_NUM_18, 0, 30, LedType::RGB);
    WS2812Animator anim(strip);
    
    strip.begin();
    anim.setEffect(WS2812Animator::Effect::Rainbow);
    
    while (true) {
        anim.tick();
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
```

### Explanation

The `WS2812Animator` class provides built-in effects. Call `tick()` periodically to update the animation.

---

## Example 3: FreeRTOS Task

This example shows how to run animations in a FreeRTOS task.

```cpp
#include "ws2812_cpp.hpp"
#include "ws2812_effects.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static void ledTask(void* param) {
    WS2812Strip strip(GPIO_NUM_18, 0, 30, LedType::RGB);
    WS2812Animator anim(strip);
    
    strip.begin();
    anim.setEffect(WS2812Animator::Effect::Breath, 0x00FF00);
    
    for (;;) {
        anim.tick();
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void app_main(void) {
    xTaskCreate(ledTask, "led", 4096, nullptr, 5, nullptr);
}
```

### Explanation

Create a dedicated FreeRTOS task for LED control. This allows the animation to run independently of other tasks.

---

## Example 4: Custom Colors

This example shows how to create custom color patterns.

```cpp
#include "ws2812_cpp.hpp"

void app_main(void) {
    WS2812Strip strip(GPIO_NUM_18, 0, 30, LedType::RGB);
    strip.begin();
    
    // Create gradient from red to blue
    for (int i = 0; i < 30; i++) {
        uint8_t r = 255 - (i * 255 / 30);
        uint8_t b = i * 255 / 30;
        uint32_t color = (r << 16) | (b << 0);
        strip.setPixel(i, color);
    }
    
    strip.show();
}
```

---

## Example 5: Brightness Control

This example demonstrates brightness adjustment.

```cpp
#include "ws2812_cpp.hpp"

void app_main(void) {
    WS2812Strip strip(GPIO_NUM_18, 0, 30, LedType::RGB);
    strip.begin();
    
    // Set all LEDs to white
    for (int i = 0; i < 30; i++) {
        strip.setPixel(i, 0xFFFFFF);
    }
    
    // Fade brightness
    for (int brightness = 255; brightness >= 0; brightness--) {
        strip.setBrightness(brightness);
        strip.show();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
```

---

## Example 6: C API Usage

This example shows how to use the C API directly.

```c
#include "ws2812_control.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main(void) {
    // Initialize
    ws2812ControlInit(GPIO_NUM_18, 0);
    
    // Set brightness
    ws2812SetBrightness(255);
    
    // Create LED state
    struct led_state state = {0};
    
    // Set colors
    state.leds[0] = 0xFF0000; // Red
    state.leds[1] = 0x00FF00; // Green
    state.leds[2] = 0x0000FF; // Blue
    
    // Send to strip
    ws2812WriteLeds(state);
}
```

---

## Running the Examples

### ESP32

The examples are available in the [`examples/esp32`](../examples/esp32/) directory.

```bash
cd examples/esp32
idf.py build flash monitor
```

## Next Steps

- Review the [API Reference](api_reference.md) for method details
- Check [Troubleshooting](troubleshooting.md) if you encounter issues
- Explore the [examples directory](../examples/) for more examples

---

**Navigation**
⬅️ [API Reference](api_reference.md) | [Next: Troubleshooting ➡️](troubleshooting.md) | [Back to Index](index.md)

