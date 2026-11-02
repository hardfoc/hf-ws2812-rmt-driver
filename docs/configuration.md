# Configuration

This guide covers all configuration options available for the WS2812 driver.

## Kconfig Configuration

The driver is primarily configured via ESP-IDF's Kconfig system. Run `idf.py menuconfig` and navigate to **Component Config** → **HF-ESP32-WS2812-RMT**.

### Configuration Options

| Option | Default | Description |
|--------|---------|-------------|
| `WS2812_LED_TYPE` | RGB | LED type: RGB (24-bit) or RGBW (32-bit) |
| `WS2812_NUM_LEDS` | 8 | Number of LEDs in the strip |
| `WS2812_LED_RMT_TX_CHANNEL` | -1 | RMT channel (-1 for auto) |
| `WS2812_LED_RMT_TX_GPIO` | 18 | GPIO pin for data output |
| `WS2812_T0H` | 14 | 0-bit high time (RMT ticks) |
| `WS2812_T1H` | 52 | 1-bit high time (RMT ticks) |
| `WS2812_T0L` | 52 | 0-bit low time (RMT ticks) |
| `WS2812_T1L` | 52 | 1-bit low time (RMT ticks) |
| `WS2812_DEFAULT_BRIGHTNESS` | 255 | Default brightness (0-255) |

**Location**: [`Kconfig`](../Kconfig)

## Runtime Configuration (C++ API)

The C++ `WS2812Strip` class allows runtime configuration, overriding Kconfig defaults:

### Constructor Parameters

```cpp
WS2812Strip strip(
    GPIO_NUM_18,      // GPIO pin
    0,                // RMT channel
    30,               // Number of LEDs
    LedType::RGB,     // LED type
    14, 52, 52, 52,   // t0h, t1h, t0l, t1l (timings)
    255               // Brightness
);
```

### Setting Timings

Adjust protocol timings for different LED variants:

```cpp
strip.setTimings(14, 52, 52, 52); // t0h, t1h, t0l, t1l
```

**Timing Values** (in RMT ticks, 1 tick ≈ 50ns):
- **WS2812**: T0H=14, T1H=52, T0L=52, T1L=52
- **WS2812B**: May need slight adjustments
- **SK6812**: Different timing requirements

### Setting Brightness

```cpp
strip.setBrightness(128); // 50% brightness (0-255)
```

## LED Types

### RGB (24-bit)

Each LED uses 3 bytes: Red, Green, Blue

```cpp
WS2812Strip strip(GPIO_NUM_18, 0, 30, LedType::RGB);
strip.setPixel(0, 0xFF0000); // Red
strip.setPixel(1, 0x00FF00); // Green
strip.setPixel(2, 0x0000FF); // Blue
```

### RGBW (32-bit)

Each LED uses 4 bytes: Red, Green, Blue, White

```cpp
WS2812Strip strip(GPIO_NUM_18, 0, 30, LedType::RGBW);
strip.setPixel(0, 0xFF000000); // Red + White
```

## RMT Channel Selection

- **Auto (-1)**: Driver selects first available channel
- **Fixed (0-7)**: Use specific channel (ESP32-C6: 0-3 only)

```cpp
// Auto-select channel
WS2812Strip strip(GPIO_NUM_18, -1, 30, LedType::RGB);

// Use specific channel
WS2812Strip strip(GPIO_NUM_18, 0, 30, LedType::RGB);
```

## Timing Configuration

### Standard Timings

Based on WS2812 datasheet (800 Kbps, 1.25μs bit time):

| Parameter | Time | RMT Ticks (approx) |
|-----------|------|-------------------|
| T0H (0-bit high) | 0.35μs | 7 |
| T0L (0-bit low) | 0.8μs | 16 |
| T1H (1-bit high) | 0.7μs | 14 |
| T1L (1-bit low) | 0.6μs | 12 |
| RES (reset) | 50μs | 1000 |

**Note**: Default values in Kconfig (14, 52, 52, 52) are tuned for reliability and may differ slightly from datasheet.

### Custom Timings

For different LED variants or fine-tuning:

```cpp
// Custom timings for specific LED variant
strip.setTimings(7, 14, 16, 12); // WS2812 datasheet values
```

## Recommended Settings

### For Small Strips (< 20 LEDs)

```cpp
WS2812Strip strip(GPIO_NUM_18, 0, 20, LedType::RGB);
strip.setBrightness(255); // Full brightness OK
```

### For Large Strips (> 50 LEDs)

```cpp
WS2812Strip strip(GPIO_NUM_18, 0, 100, LedType::RGB);
strip.setBrightness(128); // Reduce brightness to limit current
```

### For RGBW Strips

```cpp
WS2812Strip strip(GPIO_NUM_18, 0, 30, LedType::RGBW);
// Use white channel for better color rendering
```

## Next Steps

- See [Examples](examples.md) for configuration examples
- Review [API Reference](api_reference.md) for all configuration methods

---

**Navigation**
⬅️ [Platform Integration](platform_integration.md) | [Next: API Reference ➡️](api_reference.md) | [Back to Index](index.md)

