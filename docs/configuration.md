# Configuration

The Kconfig options and constructor arguments that set a strip's pin, length, LED type, timing and brightness.

## Kconfig

The component adds the menu **HF-ESP32-WS2812-RMT Configuration** to `idf.py menuconfig`. Its values are the
defaults of the `WS2812Strip` constructor and the fixed settings of the C interface.

| Option | Default | Meaning |
| --- | --- | --- |
| `CONFIG_WS2812_LED_TYPE_RGB` / `_RGBW` | RGB | 24 or 32 bits per LED |
| `CONFIG_WS2812_NUM_LEDS` | 8 | Number of LEDs |
| `CONFIG_WS2812_LED_RMT_TX_GPIO` | 18 | Data pin |
| `CONFIG_WS2812_LED_RMT_TX_CHANNEL` | -1 | Ignored: ESP-IDF chooses the channel |
| `CONFIG_WS2812_T0H` | 14 | 0 bit high time, in ticks |
| `CONFIG_WS2812_T1H` | 52 | 1 bit high time, in ticks |
| `CONFIG_WS2812_T0L` | 52 | 0 bit low time, in ticks |
| `CONFIG_WS2812_T1L` | 52 | 1 bit low time, in ticks |
| `CONFIG_WS2812_DEFAULT_BRIGHTNESS` | 255 | 0 to 255 |

Every option is the default of the matching `WS2812Strip` constructor argument. The LED type, count, timing and
brightness are also the fixed settings of the C interface.

!!! warning "The default timing is not WS2812B timing"
    `WS2812Strip` counts in ticks of 100 ns. The defaults 14, 52, 52 and 52 mean 1.4 µs and 5.2 µs, which a
    WS2812B reads as all ones. Pass the timing to the constructor, as every example on this site does, or set the
    options to the values below.

To make the defaults right for a WS2812B, put them in your project's `sdkconfig.defaults`:

```ini title="sdkconfig.defaults"
CONFIG_WS2812_T0H=4
CONFIG_WS2812_T1H=8
CONFIG_WS2812_T0L=9
CONFIG_WS2812_T1L=5
CONFIG_WS2812_LED_RMT_TX_GPIO=8
CONFIG_WS2812_NUM_LEDS=16
```

## Constructor arguments

```cpp
--8<-- "tests/docs/api_tasks.cpp:create"
```

| Argument | Default | Notes |
| --- | --- | --- |
| `gpio` (`gpio_num_t`) | `CONFIG_WS2812_LED_RMT_TX_GPIO` | Any output-capable GPIO |
| `channel` (`int`) | `CONFIG_WS2812_LED_RMT_TX_CHANNEL` | Kept for compatibility; ignored |
| `num_leds` (`uint32_t`) | `CONFIG_WS2812_NUM_LEDS` | Change later with `SetNumLeds()` |
| `type` (`LedType`) | `RGB`, or `RGBW` when Kconfig selects it | `RGBW` sends 32 bits per LED, white first |
| `t0h`, `t1h`, `t0l`, `t1l` (`uint16_t`) | `CONFIG_WS2812_T0H` ... | Ticks of 100 ns; change later with `SetTimings()` |
| `brightness` (`uint8_t`) | `CONFIG_WS2812_DEFAULT_BRIGHTNESS` | Change later with `SetBrightness()` |

## Timing values by LED

| LED | `t0h, t1h, t0l, t1l` | Datasheet values (each ±150 ns) |
| --- | --- | --- |
| WS2812B | `4, 8, 9, 5` | [0.4, 0.8, 0.85, 0.45 µs](https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf) |
| WS2812 | `4, 7, 8, 6` | [0.35, 0.7, 0.8, 0.6 µs](https://cdn.sparkfun.com/datasheets/Components/LED/WS2812.pdf) |
| Others (SK6812, WS2813, clones) | Round each time to 100 ns | Their own datasheet |

The WS2812B row also fits the WS2812's windows, so one setting covers both.

## Next

[API guide](api-guide.md) uses these settings task by task; [Signal timing](timing.md) explains the ticks.
