---
hide:
  - navigation
  - toc
---

<div class="hf-hero" markdown>
<span class="hf-kicker">HardFOC driver · ESP32</span>

# WS2812 RMT Driver

<p>A C++ driver for WS2812 and WS2812B addressable LEDs (NeoPixels) on the ESP32 family. It keeps a pixel buffer,
scales brightness, encodes every bit as an RMT symbol and lets the RMT peripheral time the pulses, so the CPU never
bit-bangs the line. Animation effects and multi-strip coordination come with it.</p>

[Quickstart](quickstart.md){ .md-button .md-button--primary }
[API guide](api-guide.md){ .md-button }
[Source on GitHub](https://github.com/hardfoc/hf-ws2812-rmt-driver){ .md-button }
</div>

The driver is ESP-IDF code. Unlike the other HardFOC drivers it has no platform port: it calls ESP-IDF's RMT
driver directly, so it runs on ESP32 chips with an RMT peripheral and ESP-IDF v5.3 or later, and nowhere else.
[Porting](porting.md) explains what that means for other targets.

<div class="hf-chip" markdown>
<div class="hf-chip__head">
<span class="hf-chip__kicker">Addressable RGB LED with integrated driver</span>
<span class="hf-chip__name">WS2812B</span>
<span class="hf-chip__vendor">Worldsemi (also WS2812)</span>
</div>

Interface
:   One data line, NRZ at 800 kbit/s; 24 bits per LED, green-red-blue, MSB first

Bit timing
:   T0H 0.4 µs, T1H 0.8 µs, T0L 0.85 µs, T1L 0.45 µs, each ±150 ns; reset: low for more than 50 µs

Supply
:   VDD 3.5 V to 5.3 V

Input levels
:   High at 0.7 × VDD or more, low at 0.3 × VDD or less: 3.5 V and 1.5 V at VDD = 5 V

Package
:   5050 (5.0 mm × 5.0 mm), 4 pins: VDD, DOUT, VSS, DIN

What the driver uses
:   One RMT transmit channel at 10 MHz (100 ns per tick), one GPIO, a blocking transmit per frame

[WS2812B datasheet](https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf) ·
[WS2812 datasheet](https://cdn.sparkfun.com/datasheets/Components/LED/WS2812.pdf)
</div>

## What you get

- **One object per strip.** `WS2812Strip` owns an RMT channel, the pixel buffer and the timing; `SetPixel()`
  and `Show()` are the whole loop. See the [API guide](api-guide.md).
- **Hardware timing.** Each bit becomes one RMT symbol with the high and low times you configure, in 100 ns
  steps. See [Signal timing](timing.md).
- **Brightness without losing your colours.** `SetBrightness()` scales at send time; the stored pixels stay as
  you wrote them.
- **RGB and RGBW.** 24 or 32 bits per LED, chosen per strip.
- **Effects.** Rainbow, chase, blink, breath, Larson scanner and solid colour, one `Tick()` per frame, and
  `WS2812MultiAnimator` to keep several strips in step. See [Architecture](architecture.md).
- **Tested without hardware.** Host unit tests run the real driver sources against a recording fake of the RMT
  driver. See [Testing and CI](testing.md).

## Where to go next

<div class="grid cards" markdown>

-   :material-rocket-launch-outline: **[Quickstart](quickstart.md)**

    ---

    Wire a strip, add the component, light it in about 15 lines.

-   :material-chip: **[Hardware](hardware.md)**

    ---

    Pins, level shifting, power and the bit timing from the datasheet.

-   :material-layers-outline: **[Architecture](architecture.md)**

    ---

    From pixels to RMT symbols: the layers and what each one owns.

-   :material-code-braces: **[API guide](api-guide.md)**

    ---

    Create a strip, set pixels, animate, handle errors, task by task.

-   :material-sine-wave: **[Signal timing](timing.md)**

    ---

    Ticks, frame time, memory and the reset gap.

-   :material-book-open-variant: **[API reference](api-reference.md)**

    ---

    Every class and member, generated from the headers.

</div>
