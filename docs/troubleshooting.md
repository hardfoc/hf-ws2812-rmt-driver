# Troubleshooting

Symptoms you may see when bringing up a WS2812 strip with this driver, their causes and the fix for each.

Start with the cheapest check: a common ground, data into DIN of the first LED, and the timing arguments.
Most faults are one of those three.

## Every LED is white, or full on in one colour

**Cause.** The bit timing. `WS2812Strip` counts in ticks of 100 ns; values in microseconds or nanoseconds, or a
project `sdkconfig` still holding the defaults of 1.0.0 and earlier (14, 52, 52, 52), make every high pulse
too long, and the LEDs read every bit as a 1.

**Fix.** Use WS2812B timing (`4, 8, 9, 5`, the Kconfig default): pass it to the constructor or fix the
`CONFIG_WS2812_T*` lines in `sdkconfig` ([Configuration](configuration.md)).

## Red and green are swapped

**Cause.** Colour packing. `SetPixel()` takes `0xRRGGBB` and the driver sends green first, as a WS2812B expects.
Code written for 1.0.0 and earlier packed `0xGGRRBB` to work around the old byte order; with this version
that swaps red and green. A strip whose LEDs read R, G, B (some clones) shows the same symptom.

**Fix.** Pack `0xRRGGBB` and drop any `Grb()`-style helper ([Colour order](timing.md#colour-order)). For an
RGB-order clone, swap the bytes before `SetPixel()`.

## Nothing lights

| Check | How |
| --- | --- |
| Ground | The ESP32, the strip and the strip's supply share one ground |
| Direction | Data goes into DIN of the first LED, not DOUT of the last |
| Supply | 3.5 V to 5.3 V at the strip; measure at the far end under load |
| GPIO | The pin passed to the constructor is the one wired; it is an output on your chip |
| Brightness | Not 0, and not left at 0 by a `Breath` effect you stopped |
| `Show()` | Called after `SetPixel()`, and its result is `ESP_OK` |

## The first LED flickers or shows random colours; the rest follow it

**Cause.** The data level or the wire. A 3.3 V high is below the WS2812B's 3.5 V threshold at 5 V, and a long or
unterminated data wire rings.

**Fix.** Add a 5 V buffer such as a 74AHCT125, keep the data wire short, and put a 300 Ω to 500 Ω resistor in
series with DIN at the strip ([Hardware](hardware.md#logic-levels)).

## The last LEDs keep old colours, or frames merge

**Cause.** Not the reset: `Show()` ends every frame with a 300 µs low, longer than any WS2812-family part needs.
The usual reasons are a frame shorter than the strip (`SetNumLeds()` smaller than the LED count), or a supply
or data problem at the point where the old colours start.

**Fix.** Set the strip length to the real LED count, then check the supply and data wire at that LED. If you send
raw bytes with `ws2812::RmtTx::TransmitWs2812()`, leave 300 µs between calls yourself
([Signal timing](timing.md#the-reset-gap)).

## Colours fade or shift towards the far end

**Cause.** Voltage drop along the strip at high current.

**Fix.** Lower the brightness, or feed the supply in at both ends or every metre or so
([Hardware](hardware.md#power)).

## `Begin()` fails, or `Show()` returns `ESP_ERR_INVALID_STATE`

**Cause.** The strip's RMT channel could not be set up; the log names `rmt_new_tx_channel`, `rmt_enable` or the
encoder. Usual reasons: no free transmit channel (`ESP_ERR_NOT_FOUND`: each strip takes one), an invalid GPIO,
or out of memory.

**Fix.** Create fewer strips, check the GPIO, and see [Supported chips](porting.md#supported-chips) for the
channel counts. `Begin()` retries, so it succeeds once a channel is free.

## Glitches while Wi-Fi or other interrupts are busy

**Cause.** The RMT channel has one memory block (48 or 64 symbols) and no DMA; ESP-IDF refills it from an interrupt
during a frame. A late refill stretches a low period.

**Fix.** Keep strips short, send frames from a high-priority task, and avoid long critical sections while
`Show()` runs ([Signal timing](timing.md#memory)).

## The C functions in `ws2812_control.h` send garbage

**Cause.** Version 1.0.0 and earlier passed the RMT symbol buffer of `ws2812_write_leds()` to a byte encoder. Fixed
since; with this version, garbage from the C interface means the Kconfig timing or LED type is wrong.

**Fix.** Check `CONFIG_WS2812_T*` and `CONFIG_WS2812_LED_TYPE_*` ([Configuration](configuration.md)), or use
`WS2812Strip` ([API guide](api-guide.md#the-c-interface)).
