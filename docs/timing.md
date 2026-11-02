# Signal timing

How the driver turns pixels into pulses: ticks, the time a frame takes, the memory it needs and the reset gap.

## Ticks

`WS2812Strip` runs its RMT channel at 10 MHz, so every duration it sends is a whole number of 100 ns ticks. The
four timing arguments (`t0h`, `t1h`, `t0l`, `t1l`) are those tick counts, used as they are: the driver does not
convert from nanoseconds.

| Pulse | Driver value | WS2812B window |
| --- | --- | --- |
| 0 bit high, `t0h` | 4: 400 ns | 250-550 ns |
| 0 bit low, `t0l` | 9: 900 ns | 700-1000 ns |
| 1 bit high, `t1h` | 8: 800 ns | 650-950 ns |
| 1 bit low, `t1l` | 5: 500 ns | 300-600 ns |

Any value inside the window works; the LED samples the line roughly in the middle of the bit, so the difference
between a short and a long high pulse is what matters. Values from the [Kconfig](configuration.md) defaults
(14, 52, 52, 52) are 1.4 µs and 5.2 µs: every bit reads as a 1.

`ws2812::RmtTx::TransmitWs2812()`, the lower-level byte path, has its own fixed timing for 10 MHz: 400 / 900 ns
for a 0 and 800 / 500 ns for a 1, the same as the table.

## Frame time

Each LED takes 24 bits (32 for RGBW). With the timing above every bit lasts 1.3 µs:

| LEDs | RGB frame | Highest refresh rate |
| --- | --- | --- |
| 8 | 0.25 ms | about 3300 Hz |
| 60 | 1.9 ms | about 520 Hz |
| 144 | 4.5 ms | about 220 Hz |
| 300 | 9.4 ms | about 106 Hz |

An RGBW frame is a third longer (32 bits instead of 24).

The refresh rate allows for the frame and the 50 µs reset gap. These numbers are derived from the bit time, not
measured; `Show()` also spends CPU time building the symbols before the transfer starts.

`Show()` blocks the calling task until the last symbol is out (`rmt_tx_wait_all_done()` with no timeout). Call it
from a task that can afford the wait, never from an interrupt.

## Memory

| Buffer | Bytes per LED |
| --- | --- |
| Pixels (`uint32_t`) | 4 |
| RMT symbols, RGB (24 × 4 bytes) | 96 |
| RMT symbols, RGBW (32 × 4 bytes) | 128 |

A 60-LED RGB strip needs 6000 bytes.

Both are `std::vector`s allocated when the strip is created or resized. The RMT channel itself has 64 symbols of
hardware memory and no DMA; ESP-IDF refills it from an interrupt while a frame is sent. A refill delayed by a
long critical section stretches a low period, which the LEDs read as a reset or a wrong bit. If you see glitches
under heavy interrupt load, keep strips short or send frames from a high-priority task.

## The reset gap

A WS2812B shows the new colours once its data line has stayed low for more than 50 µs. `WS2812Strip::Show()`
does not append a reset pulse: after the last bit the RMT leaves the line low, and the LEDs latch when the next
`Show()` starts late enough. Any frame loop with a delay (an animation tick of a few milliseconds) gives that gap
on its own. Two `Show()` calls back to back on a short strip may not; put at least 50 µs between them.

The C interface's encoder (`led_strip_encoder.c`) appends a 50 µs low symbol after each frame.

## Colour order

`Show()` sends the packed 24-bit value most significant bit first: bits 23-16, then 15-8, then 7-0. A WS2812B
reads green, red, blue, so pack colours as `0xGGRRBB`, as the `Grb()` helper in the
[Quickstart](quickstart.md#3-light-it) does.

For an RGBW strip the white byte (bits 31-24) goes first, then bits 23-0 in the same order. `ColorWheel()` and
the effects pack `0xRRGGBB`, so on a WS2812B their red and green are swapped; the rainbow is still a rainbow, at
a different hue offset.
