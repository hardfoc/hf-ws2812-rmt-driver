# API guide

How to do the common jobs with the driver: create a strip, set pixels, dim, resize, animate, drive several
strips and handle errors.

Every snippet on this page is compiled by the host CI from [`tests/docs/api_tasks.cpp`](../tests/docs/api_tasks.cpp),
where `Rgb(r, g, b)` packs a colour as `0xRRGGBB`, the format `SetPixel()` takes (see [Colour
order](timing.md#colour-order)). The [API reference](api-reference.md) lists every member.

## The shape of the API

| Class | Header | Owns |
| --- | --- | --- |
| `WS2812Strip` | `ws2812_cpp.hpp` | One RMT channel, the pixel buffer, brightness, timing, LED type |
| `WS2812Animator` | `ws2812_effects.hpp` | One effect on one strip: a step counter and a colour |
| `WS2812MultiAnimator` | `ws2812_multi_animator.hpp` | One animator per strip, with a shared step |
| `ws2812::RmtTx` | `rmt_wrapper.hpp` | An RMT transmit channel; raw symbols or WS2812 bytes |

| `WS2812Strip` member | Does |
| --- | --- |
| `WS2812Strip(gpio, channel, num_leds, type, t0h, t1h, t0l, t1l, brightness)` | Creates the RMT channel; every argument has a Kconfig default |
| `SetPixel(index, colour)` | Writes one pixel (`0xRRGGBB`, or `0xWWRRGGBB` for RGBW); out-of-range indices are ignored |
| `Show()` | Scales, encodes and sends the buffer green first, then the reset; blocks until done; returns `esp_err_t` |
| `SetBrightness(0-255)` | Global scale applied at the next `Show()` |
| `SetNumLeds(n)` | Resizes and clears the buffer |
| `SetTimings(t0h, t1h, t0l, t1l)` | New bit timing in 100 ns ticks |
| `Length()` | The number of LEDs |
| `ColorWheel(0-255)` | A colour on a red-green-blue wheel, packed `0xRRGGBB` |
| `Begin()` | `ESP_OK` if the constructor set the channel up, otherwise retries and returns the RMT error |
| `GetDriverVersion()` | The version string, for example `"1.0.0"` |

## Create a strip

```cpp
--8<-- "tests/docs/api_tasks.cpp:create"
```

The constructor creates and enables the RMT channel at once. It does not abort when that fails (no free channel,
an invalid GPIO, no memory); check `Begin()` before the first frame:

```cpp
--8<-- "tests/docs/api_tasks.cpp:begin"
```

Until `Begin()` succeeds, `Show()` returns `ESP_ERR_INVALID_STATE`. Keep the object alive as long as you use the
LEDs: the destructor releases the channel. The `channel` argument is accepted and unused: ESP-IDF v5's RMT driver
picks a free transmit channel and has no way to request a number.

## Set pixels and show them

```cpp
--8<-- "tests/docs/api_tasks.cpp:pixels"
```

`SetPixel()` only writes the buffer. `Show()` sends the whole strip every time; there is no partial update, and
the LEDs keep their colours until the next frame.

## Dim the strip

```cpp
--8<-- "tests/docs/api_tasks.cpp:brightness"
```

Each channel is scaled as `value × brightness / 255` while encoding. The buffer keeps the full values, so raising
the brightness again restores the colours exactly. At low brightness, small values round down to 0.

## Change the length

```cpp
--8<-- "tests/docs/api_tasks.cpp:resize"
```

`SetNumLeds()` reallocates the pixel and symbol buffers and clears every pixel. Call it from the task that calls
`Show()`.

## Change the timing

```cpp
--8<-- "tests/docs/api_tasks.cpp:timings"
```

The values are RMT ticks of 100 ns. [Signal timing](timing.md) has the windows for the WS2812 and WS2812B.

## Animate

```cpp
--8<-- "tests/docs/api_tasks.cpp:animate"
```

| Effect | Each `Tick()` |
| --- | --- |
| `Off` | Clears the strip |
| `SolidColor` | Sets every LED to the colour |
| `Rainbow` | Spreads `ColorWheel()` over the strip and shifts it by one step |
| `Chase` | Lights every third LED and moves the pattern by one |
| `Blink` | Alternates between the colour and off |
| `Breath` | Ramps the strip brightness up and down in steps of 4 (overwrites `SetBrightness()`) |
| `Larson` | One lit LED that moves end to end and back |

Every `Tick()` ends with a `Show()`, so the tick rate is the frame rate; 20 ms (50 Hz) is smooth. `Tick()` ignores
the result of `Show()`; check transmit errors on the strip if you need them. `SetVirtualLength()` stretches an
effect over a longer imaginary strip, and `SetStep()` / `Step()` read and set its position.

## Drive several strips

```cpp
--8<-- "tests/docs/api_tasks.cpp:multi"
```

Each strip has its own RMT channel and one memory block, so the number of strips is the chip's number of transmit
channels ([Porting](porting.md#supported-chips)). One strip too many fails in `Begin()` with `ESP_ERR_NOT_FOUND`. The
strips are sent one after the other in each `Tick()`.

## Handle errors

```cpp
--8<-- "tests/docs/api_tasks.cpp:errors"
```

| Signal | Meaning | What to do |
| --- | --- | --- |
| `Begin()` returns an error | `rmt_new_tx_channel()`, `rmt_enable()` or the encoder failed: `ESP_ERR_NOT_FOUND` (no free channel), a bad GPIO, no memory | Fix the configuration, or free a channel and call `Begin()` again |
| `Show()` returns `ESP_ERR_INVALID_STATE` | The channel was never set up | Check `Begin()` |
| `Show()` returns another `esp_err_t` | The RMT driver refused the transfer | Log it with `esp_err_to_name()`; the frame was not sent |
| Wrong colours, all white, flicker | Timing, colour order or signal level, not a software error | [Troubleshooting](troubleshooting.md) |

## Send bytes directly

```cpp
--8<-- "tests/docs/api_tasks.cpp:raw"
```

`ws2812::RmtTx` can also be used on its own: `TransmitWs2812()` sends bytes as they are with the WS2812B timing,
`Transmit()` sends prepared symbols and `TransmitBytes()` takes your own 0 and 1 symbols. All three block until
the transfer is done, send no reset of their own, and return `ESP_ERR_INVALID_STATE` while `IsReady()` is false;
`InitStatus()` holds the set-up error and `Init()` retries it.

## The C interface

The C functions `ws2812_control_init()`, `ws2812_write_leds()` and `ws2812_set_brightness()` drive one strip whose
LED type, length and timing come from Kconfig. `struct led_state` holds one `0xRRGGBB` (or `0xWWRRGGBB`) value
per LED; `ws2812_write_leds()` scales them by the brightness, sends them green first with the Kconfig timing,
appends the 300 µs reset and blocks until done. It returns `ESP_ERR_INVALID_STATE` before
`ws2812_control_init()`, which can be called once (a second call returns `ESP_ERR_INVALID_STATE`) and cleans up
after itself when it fails. The `channel` argument is unused. Prefer `WS2812Strip` for more than one strip.

## Threads

The classes are not thread-safe. Give each strip one owner task, or guard every call on a strip (and on any
animator that uses it) with one mutex.
