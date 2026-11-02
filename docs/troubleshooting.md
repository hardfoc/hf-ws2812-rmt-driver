# Troubleshooting

Symptoms you may see when bringing up a WS2812 strip with this driver, their causes and the fix for each.

Start with the cheapest check: a common ground, data into DIN of the first LED, and the timing arguments.
Most faults are one of those three.

## Every LED is white, or full on in one colour

**Cause.** The bit timing. `WS2812Strip` counts in ticks of 100 ns; with the Kconfig defaults (14, 52, 52, 52)
every high pulse lasts at least 1.4 µs, and the LEDs read every bit as a 1.

**Fix.** Pass WS2812B timing to the constructor (`4, 8, 9, 5`) or put it in `sdkconfig.defaults`
([Configuration](configuration.md)).

## Red and green are swapped

**Cause.** Colour order. The driver sends the packed value most significant byte first; a WS2812B expects green,
then red, then blue.

**Fix.** Pack colours as `0xGGRRBB` ([Colour order](timing.md#colour-order)). The built-in effects and
`ColorWheel()` pack `0xRRGGBB`, so their colours are swapped the same way.

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

**Cause.** The reset gap. `Show()` does not send a reset pulse; the LEDs latch only after the line has been low
for more than 50 µs. Two `Show()` calls in a tight loop may not leave that gap.

**Fix.** Delay at least 50 µs between frames; any animation tick does ([Signal timing](timing.md#the-reset-gap)).

## Colours fade or shift towards the far end

**Cause.** Voltage drop along the strip at high current.

**Fix.** Lower the brightness, or feed the supply in at both ends or every metre or so
([Hardware](hardware.md#power)).

## The program aborts when a strip is created

**Cause.** `ws2812::RmtTx` creates its channel with `ESP_ERROR_CHECK`: an RMT error aborts with a backtrace that
names `rmt_new_tx_channel`, `rmt_enable` or `rmt_new_copy_encoder`. Usual reasons: no free transmit channel
(each strip takes one, and on chips with 48-symbol blocks one strip can take two blocks), an invalid GPIO, or
out of memory.

**Fix.** Create fewer strips, check the GPIO, and see [Supported chips](porting.md#supported-chips) for the
channel counts.

## Glitches while Wi-Fi or other interrupts are busy

**Cause.** The RMT channel has 64 symbols of memory and no DMA; ESP-IDF refills it from an interrupt during a
frame. A late refill stretches a low period.

**Fix.** Keep strips short, send frames from a high-priority task, and avoid long critical sections while
`Show()` runs ([Signal timing](timing.md#memory)).

## The C functions in `ws2812_control.h` send garbage

**Cause.** A defect in this version: `ws2812_write_leds()` passes its RMT symbol buffer to a byte encoder.

**Fix.** Use `WS2812Strip` ([API guide](api-guide.md#the-c-interface)).
