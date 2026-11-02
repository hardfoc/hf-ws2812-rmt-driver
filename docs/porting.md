# Porting

What runs where: the driver is ESP-IDF code for the ESP32 family. This page lists the chips it supports, what
binds it to ESP-IDF, and what using it elsewhere would take.

## Why there is no port to write

The other HardFOC drivers talk to a chip over SPI, I2C or UART, so they ask you for a small class that moves
bytes, and run on any MCU. A WS2812 has no bus: it needs a pulse train timed to about 100 ns for the whole frame.
On an ESP32 the RMT peripheral generates it from a buffer of symbols. This driver is built around that
peripheral and calls ESP-IDF's RMT driver directly; there is no interface between the two for you to implement.

## Supported chips

The component manifest lists these targets. The channel and memory figures are ESP-IDF's (`soc_caps.h`, v5.5).

| Chip | Transmit channels | Symbols per block |
| --- | --- | --- |
| ESP32 | 8 (any channel) | 64 |
| ESP32-S2 | 4 (any channel) | 64 |
| ESP32-S3 | 4 | 48 |
| ESP32-C3 | 2 | 48 |
| ESP32-C6 | 2 | 48 |
| ESP32-H2 | 2 | 48 |

CI builds the examples for the ESP32-C6 with ESP-IDF v5.5; the other chips are listed by the manifest but not
built.

Each `WS2812Strip` asks ESP-IDF for one transmit channel with 64 symbols of memory. Where a block holds 48
symbols, that request takes two blocks, which can leave the next channel without memory: on the two-channel
chips, plan for one strip until you have checked a second one on hardware.

Chips without an RMT peripheral (the ESP32-C2, for example) cannot run it. ESP-IDF v5.3 or later is required,
because the component requires `esp_driver_rmt`.

## What is bound to ESP-IDF

| File | ESP-IDF dependency |
| --- | --- |
| `inc/rmt_wrapper.hpp` | The RMT TX and RX driver, encoders, `esp_check.h`, `esp_log.h`, FreeRTOS queues |
| `inc/ws2812_cpp.hpp`, `src/ws2812_cpp.cpp` | `gpio_num_t`, `esp_err_t`, `rmt_symbol_word_t`, Kconfig defaults (`sdkconfig.h`) |
| `inc/ws2812_effects.hpp`, `inc/ws2812_multi_animator.hpp` | Only through `WS2812Strip` |
| `inc/ws2812_control.h`, `src/*.c` | The RMT driver and Kconfig, directly |

The effects and the bit encoding in `WS2812Strip::Show()` are plain C++; the ESP-IDF parts are the channel in
`ws2812::RmtTx` and the types in the public headers.

## Using it on another MCU

There is no supported path. Doing it means:

1. Replacing `ws2812::RmtTx` with a transmitter for your MCU that sends a buffer of (high time, low time) pairs:
   a timer with DMA, an SPI peripheral clocked so that a few SPI bits make one WS2812 bit, or a PIO block.
2. Replacing the ESP-IDF types in the public headers (`gpio_num_t`, `esp_err_t`, `rmt_symbol_word_t`) and the
   Kconfig defaults.

The host build in `tests/stubs/` shows the full set of ESP-IDF names the driver uses: it is a starting point, not
a port. For a non-ESP32 project, a library written for your MCU's timing peripheral is usually less work.

## Using it on any ESP32

Nothing to write: the [component wrapper](installation.md#esp-idf-component) builds it. Check that the GPIO is
an output on your chip, that you have a free RMT transmit channel per strip, and pass WS2812B timing to the
constructor ([Configuration](configuration.md)).

## Threads

The driver is not thread-safe. Give each strip one owner task, or guard every call on a strip and its animators
with one mutex. `Show()` blocks its caller for the length of a frame; strips on different channels can be sent
from different tasks.
