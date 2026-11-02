# API Reference

Complete reference documentation for all public methods and types in the WS2812 driver.

## Source Code

- **C API Header**: [`inc/ws2812_control.h`](../inc/ws2812_control.h)
- **C API Implementation**: [`src/ws2812_control.c`](../src/ws2812_control.c)
- **C++ API Header**: [`inc/ws2812_cpp.hpp`](../inc/ws2812_cpp.hpp)
- **C++ API Implementation**: [`src/ws2812_cpp.cpp`](../src/ws2812_cpp.cpp)
- **Effects Header**: [`inc/ws2812_effects.hpp`](../inc/ws2812_effects.hpp)

## C API

### Functions

| Function | Signature | Location |
|----------|-----------|----------|
| `ws2812ControlInit()` | `esp_err_t ws2812ControlInit(gpio_num_t gpio_num, int channel)` | [`src/ws2812_control.c#L59`](../src/ws2812_control.c#L59) |
| `ws2812WriteLeds()` | `esp_err_t ws2812WriteLeds(struct led_state new_state)` | [`src/ws2812_control.c#L91`](../src/ws2812_control.c#L91) |
| `ws2812SetBrightness()` | `void ws2812SetBrightness(uint8_t brightness)` | [`src/ws2812_control.c#L104`](../src/ws2812_control.c#L104) |

### Structures

| Type | Description | Location |
|------|-------------|----------|
| `led_state` | LED color state structure | [`inc/ws2812_control.h#L62`](../inc/ws2812_control.h#L62) |

## C++ API

### `WS2812Strip`

Main C++ class for WS2812 LED control.

**Location**: [`inc/ws2812_cpp.hpp#L31`](../inc/ws2812_cpp.hpp#L31)

**Constructor:**
```cpp
WS2812Strip(gpio_num_t gpio, int channel, uint32_t numLeds, LedType type, 
            uint16_t t0h, uint16_t t1h, uint16_t t0l, uint16_t t1l, 
            uint8_t brightness);
```

**Location**: [`inc/ws2812_cpp.hpp#L39`](../inc/ws2812_cpp.hpp#L39)

### Methods

| Method | Signature | Location |
|--------|-----------|----------|
| `begin()` | `esp_err_t begin()` | [`inc/ws2812_cpp.hpp#L55`](../inc/ws2812_cpp.hpp#L55) |
| `setPixel()` | `void setPixel(uint32_t index, uint32_t rgbw)` | [`inc/ws2812_cpp.hpp#L63`](../inc/ws2812_cpp.hpp#L63) |
| `length()` | `uint32_t length() const` | [`inc/ws2812_cpp.hpp#L66`](../inc/ws2812_cpp.hpp#L66) |
| `show()` | `esp_err_t show()` | [`inc/ws2812_cpp.hpp#L71`](../inc/ws2812_cpp.hpp#L71) |
| `setBrightness()` | `void setBrightness(uint8_t value)` | [`inc/ws2812_cpp.hpp#L78`](../inc/ws2812_cpp.hpp#L78) |
| `setTimings()` | `void setTimings(uint16_t t0h, uint16_t t1h, uint16_t t0l, uint16_t t1l)` | [`inc/ws2812_cpp.hpp#L85`](../inc/ws2812_cpp.hpp#L85) |
| `colorWheel()` | `static uint32_t colorWheel(uint8_t pos)` | [`inc/ws2812_cpp.hpp#L93`](../inc/ws2812_cpp.hpp#L93) |

### `WS2812Animator`

Animation helper class for WS2812 strips.

**Location**: [`inc/ws2812_effects.hpp#L14`](../inc/ws2812_effects.hpp#L14)

**Constructor:**
```cpp
WS2812Animator(WS2812Strip& strip, uint32_t virtualLength = 0);
```

**Location**: [`inc/ws2812_effects.hpp#L18`](../inc/ws2812_effects.hpp#L18)

### Methods

| Method | Signature | Location |
|--------|-----------|----------|
| `setEffect()` | `void setEffect(Effect effect, uint32_t color = 0xFFFFFF)` | [`inc/ws2812_effects.hpp#L26`](../inc/ws2812_effects.hpp#L26) |
| `setVirtualLength()` | `void setVirtualLength(uint32_t length)` | [`inc/ws2812_effects.hpp#L29`](../inc/ws2812_effects.hpp#L29) |
| `setStep()` | `void setStep(uint16_t step)` | [`inc/ws2812_effects.hpp#L32`](../inc/ws2812_effects.hpp#L32) |
| `step()` | `uint16_t step() const` | [`inc/ws2812_effects.hpp#L33`](../inc/ws2812_effects.hpp#L33) |
| `tick()` | `void tick()` | [`inc/ws2812_effects.hpp#L38`](../inc/ws2812_effects.hpp#L38) |

## Types

### Enumerations

| Type | Values | Location |
|------|--------|----------|
| `LedType` | `RGB`, `RGBW` | [`inc/ws2812_cpp.hpp#L22`](../inc/ws2812_cpp.hpp#L22) |
| `WS2812Animator::Effect` | `Off`, `SolidColor`, `Rainbow`, `Chase`, `Blink`, `Breath`, `Larson` | [`inc/ws2812_effects.hpp#L16`](../inc/ws2812_effects.hpp#L16) |

### Structures

| Type | Description | Location |
|------|-------------|----------|
| `led_state` | LED color state structure (C API) | [`inc/ws2812_control.h#L62`](../inc/ws2812_control.h#L62) |

---

**Navigation**
⬅️ [Configuration](configuration.md) | [Next: Examples ➡️](examples.md) | [Back to Index](index.md)
