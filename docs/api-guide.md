# API guide

How to do the common jobs with the driver: create a strip, set pixels, dim, resize, animate, drive several
strips and handle errors.

Every snippet on this page is compiled by the host CI from
[`tests/docs/api_tasks.cpp`](../tests/docs/api_tasks.cpp), where `Grb(r, g, b)` packs a colour as `0xGGRRBB`
(see [Colour order](timing.md#colour-order)). The [API reference](api-reference.md) lists every member.

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
| `SetPixel(index, colour)` | Writes one pixel of the buffer; out-of-range indices are ignored |
| `Show()` | Scales, encodes and sends the buffer; blocks until done; returns `esp_err_t` |
| `SetBrightness(0-255)` | Global scale applied at the next `Show()` |
| `SetNumLeds(n)` | Resizes and clears the buffer |
| `SetTimings(t0h, t1h, t0l, t1l)` | New bit timing in 100 ns ticks |
| `Length()` | The number of LEDs |
| `ColorWheel(0-255)` | A colour on a red-green-blue wheel, packed `0xRRGGBB` |
| `Begin()` | Nothing; returns `ESP_OK` (the constructor already set the channel up) |
| `GetDriverVersion()` | The version string, for example `"1.0.0"` |

## Create a strip

```cpp
--8<-- "tests/docs/api_tasks.cpp:create"
```

The constructor creates and enables the RMT channel at once, so create strips where an abort at start-up is
acceptable, typically in `app_main()` or a task's set-up. Keep the object alive as long as you use the LEDs: the
destructor releases the channel. The `channel` argument is accepted and ignored; ESP-IDF's RMT driver picks a
free transmit channel.

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

Each strip has its own RMT channel, so the number of strips is limited by the chip's transmit channels
([Porting](porting.md#supported-chips)). The strips are sent one after the other in each `Tick()`.

## Handle errors

```cpp
--8<-- "tests/docs/api_tasks.cpp:errors"
```

| Signal | Meaning | What to do |
| --- | --- | --- |
| Abort in the `RmtTx` constructor | `rmt_new_tx_channel()`, `rmt_enable()` or the encoder failed: no free channel, bad GPIO, no memory | Fix the configuration; it is not recoverable at run time |
| `Show()` returns an `esp_err_t` other than `ESP_OK` | The RMT driver refused the transfer | Log it with `esp_err_to_name()`; the frame was not sent |
| Wrong colours, all white, flicker | Timing, colour order or signal level, not a software error | [Troubleshooting](troubleshooting.md) |

## Send bytes directly

```cpp
--8<-- "tests/docs/api_tasks.cpp:raw"
```

`ws2812::RmtTx` can also be used on its own: `TransmitWs2812()` sends bytes as they are with the WS2812B timing,
`Transmit()` sends prepared symbols and `TransmitBytes()` takes your own 0 and 1 symbols. All three block until
the transfer is done.

## The C interface

!!! warning "Do not use `ws2812_control.h` in new code"
    The C functions `ws2812_control_init()`, `ws2812_write_leds()` and `ws2812_set_brightness()` drive one strip
    sized by Kconfig. In this version `ws2812_write_leds()` passes its buffer of RMT symbols to a byte encoder, so
    the strip receives the bytes of the symbols instead of the colours. Use `WS2812Strip`.

## Threads

The classes are not thread-safe. Give each strip one owner task, or guard every call on a strip (and on any
animator that uses it) with one mutex.
