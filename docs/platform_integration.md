# Platform Integration Guide

This guide explains the platform-specific nature of the WS2812 driver and how it integrates with ESP32 hardware.

## ESP32-Specific Design

The WS2812 driver is **ESP32-specific** and uses the ESP-IDF RMT (Remote Control) peripheral directly. Unlike other HardFOC drivers that use CRTP-based hardware abstraction, this driver is tightly coupled to ESP32 hardware for optimal performance.

### Why ESP32-Specific?

The WS2812 protocol requires precise timing (800kHz data rate with sub-microsecond pulse widths). The ESP32's RMT peripheral is specifically designed for this type of protocol:

- **Hardware Timing**: RMT generates precise waveforms without CPU intervention
- **DMA Support**: Automatic data transfer reduces CPU load
- **Multiple Channels**: Supports multiple LED strips simultaneously
- **Low Latency**: Hardware-based timing ensures reliable communication

### Architecture Decision

Unlike other HardFOC drivers that use CRTP (Curiously Recurring Template Pattern) for platform abstraction, the WS2812 driver:

- ✅ **Uses ESP-IDF RMT API directly** for maximum performance
- ✅ **Configured via Kconfig** for compile-time optimization
- ✅ **Provides C++ wrapper** (`WS2812Strip`) for convenience
- ❌ **No hardware abstraction layer** (intentional design choice)

This design prioritizes:
1. **Performance**: Zero abstraction overhead
2. **Simplicity**: Direct hardware access
3. **ESP32 Optimization**: Leverages ESP32-specific features

## RMT Peripheral Configuration

The driver uses ESP-IDF's RMT TX (transmit) channel to generate WS2812 signals.

### RMT Channel Setup

```cpp
#include "ws2812_control.h"

// Initialize RMT for WS2812 output
esp_err_t ws2812ControlInit(gpio_num_t gpio_num, int channel);
```

**Parameters:**
- `gpio_num`: GPIO pin connected to LED data line
- `channel`: RMT channel (0-7, or -1 for auto-select)

**Example:**
```cpp
// Initialize on GPIO 18, auto-select channel
esp_err_t ret = ws2812ControlInit(GPIO_NUM_18, -1);
if (ret != ESP_OK) {
    ESP_LOGE("WS2812", "Initialization failed");
}
```

### RMT Timing Configuration

Timing is configured via Kconfig (see [Configuration](configuration.md)):

- **T0H**: 0-bit high time (default: 14 RMT ticks ≈ 700ns)
- **T1H**: 1-bit high time (default: 52 RMT ticks ≈ 2.6μs)
- **T0L**: 0-bit low time (default: 52 RMT ticks ≈ 2.6μs)
- **T1L**: 1-bit low time (default: 52 RMT ticks ≈ 2.6μs)

**RMT Resolution**: 10 MHz (100ns per tick)

## C++ API Usage

The `WS2812Strip` class provides a convenient C++ wrapper:

```cpp
#include "ws2812_cpp.hpp"

// Create strip instance
WS2812Strip strip(
    GPIO_NUM_18,    // GPIO pin
    0,              // RMT channel
    30,             // Number of LEDs
    LedType::RGB,   // LED type (RGB or RGBW)
    14, 52, 52, 52, // Timing: t0h, t1h, t0l, t1l
    255             // Brightness (0-255)
);

// Initialize
strip.begin();

// Set LED colors
strip.setPixel(0, 0xFF0000); // Red
strip.setPixel(1, 0x00FF00); // Green
strip.setPixel(2, 0x0000FF); // Blue

// Send to LEDs
strip.show();
```

## Multiple Strips

You can drive multiple LED strips using different RMT channels:

```cpp
// Strip 1 on GPIO 18, RMT channel 0
WS2812Strip strip1(GPIO_NUM_18, 0, 30, LedType::RGB);

// Strip 2 on GPIO 19, RMT channel 1
WS2812Strip strip2(GPIO_NUM_19, 1, 30, LedType::RGB);

strip1.begin();
strip2.begin();

// Control independently
strip1.setPixel(0, 0xFF0000);
strip2.setPixel(0, 0x0000FF);
strip1.show();
strip2.show();
```

## Porting to Other Platforms

If you need to port this driver to another platform, you would need to:

1. **Implement RMT-equivalent functionality**:
   - Hardware timer for precise timing
   - DMA or interrupt-based data transfer
   - GPIO control for data output

2. **Adapt the timing code**:
   - Convert RMT ticks to your platform's timing units
   - Implement bit encoding (24/32 bits per LED)

3. **Consider abstraction**:
   - Create a CRTP interface similar to other HardFOC drivers
   - Abstract the timing peripheral (RMT, PWM, SPI, etc.)

**Note**: The current design prioritizes ESP32 performance over portability. For other platforms, consider creating a new driver with hardware abstraction.

## Performance Characteristics

- **Update Rate**: ~30 FPS for 30 LEDs (depends on LED count)
- **CPU Usage**: Minimal (RMT handles timing, DMA handles transfer)
- **Memory**: ~1KB per LED (RMT buffer)
- **Latency**: <1ms for typical strips

## Troubleshooting

### RMT Channel Conflicts

If you're using other RMT channels in your project:

```cpp
// Manually specify RMT channel to avoid conflicts
WS2812Strip strip(GPIO_NUM_18, 2, 30); // Use channel 2
```

### Timing Issues

If LEDs show incorrect colors:

1. Check timing values in Kconfig
2. Verify GPIO pin configuration
3. Check power supply (WS2812 requires stable 5V)
4. Verify data line connections

See [Troubleshooting](troubleshooting.md) for more details.

## Comparison with Other HardFOC Drivers

| Aspect | WS2812 Driver | Other Drivers |
|--------|---------------|---------------|
| **Abstraction** | ESP32-specific | CRTP-based |
| **Portability** | ESP32 only | Platform-agnostic |
| **Performance** | Maximum (direct RMT) | Optimized (CRTP) |
| **Configuration** | Kconfig + Runtime | Runtime only |
| **Use Case** | ESP32 projects | Multi-platform |

## Summary

The WS2812 driver is intentionally ESP32-specific to leverage the RMT peripheral's capabilities. While this limits portability, it provides:

- ✅ Maximum performance
- ✅ Simple integration
- ✅ Hardware-optimized timing
- ✅ Low CPU overhead

For ESP32 projects, this design is optimal. For other platforms, consider implementing a hardware abstraction layer or using platform-specific alternatives.

---

**Navigation**
⬅️ [Hardware Setup](hardware_setup.md) | [Next: Configuration ➡️](configuration.md) | [Back to Index](index.md)

