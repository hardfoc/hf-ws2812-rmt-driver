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
between a short and a long high pulse is what matters. The [Kconfig](configuration.md) defaults are the values in
the table (4, 8, 9, 5), so a strip created without timing arguments is right for a WS2812B.

`ws2812::RmtTx::TransmitWs2812()`, the lower-level byte path, has its own fixed timing for 10 MHz: 400 / 900 ns
for a 0 and 800 / 500 ns for a 1, the same as the table.

## Frame time

Each LED takes 24 bits (32 for RGBW). With the timing above every bit lasts 1.3 µs:

| LEDs | RGB frame | Highest refresh rate |
| --- | --- | --- |
| 8 | 0.25 ms | about 1800 Hz |
| 60 | 1.9 ms | about 460 Hz |
| 144 | 4.5 ms | about 210 Hz |
| 300 | 9.4 ms | about 104 Hz |

An RGBW frame is a third longer (32 bits instead of 24).

The refresh rate allows for the frame and the 300 µs reset `Show()` sends after it. These numbers are derived from the
bit time, not measured; `Show()` also spends CPU time building the symbols before the transfer starts.

`Show()` blocks the calling task until the last symbol is out (`rmt_tx_wait_all_done()` with no timeout). Call it
from a task that can afford the wait, never from an interrupt.

## Memory

| Buffer | Bytes per LED |
| --- | --- |
| Pixels (`uint32_t`) | 4 |
| RMT symbols, RGB (24 × 4 bytes) | 96 |
| RMT symbols, RGBW (32 × 4 bytes) | 128 |

A 60-LED RGB strip needs 6004 bytes: the symbol buffer has one more symbol for the reset.

Both are `std::vector`s allocated when the strip is created or resized. The RMT channel itself has one memory
block of hardware memory (64 symbols on the ESP32 and ESP32-S2, 48 on the S3, C3, C6 and H2) and no DMA; ESP-IDF
refills it from an interrupt while a frame is sent. A refill delayed by a
long critical section stretches a low period, which the LEDs read as a reset or a wrong bit. If you see glitches
under heavy interrupt load, keep strips short or send frames from a high-priority task.

## The reset gap

A WS2812B shows the new colours once its data line has stayed low for the reset time: more than 50 µs in the
WS2812B datasheet, at least 280 µs for the WS2812B-V5 (80 µs for the SK6812). `WS2812Strip::Show()` ends every
frame with a low symbol of `WS2812Strip::kResetTimeUs` (300 µs), which covers all of them, and returns only after
it has been sent. `Show()` calls back to back are therefore safe.

The C interface's encoder (`led_strip_encoder.c`) appends the same 300 µs reset. `ws2812::RmtTx::TransmitWs2812()`
sends only the bytes you give it; leave 300 µs before the next call.

## Colour order

Pixels are packed `0xRRGGBB`: `SetPixel(i, 0xFF0000)` is red. A WS2812B reads green, red, blue
(datasheet "Composition of 24bit data": G7 ... G0, R7 ... R0, B7 ... B0), so `Show()` sends the green byte, then
red, then blue, each most significant bit first. `ColorWheel()` and the effects use the same packing.

For an RGBW strip pack `0xWWRRGGBB`; `Show()` sends green, red, blue, then white, the order of the SK6812RGBW.
