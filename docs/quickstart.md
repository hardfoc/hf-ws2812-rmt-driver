# Quickstart

Light a WS2812B strip from an ESP32: wire three signals, add the component, write one loop.

!!! note "ESP32 and ESP-IDF only"
    The driver calls ESP-IDF's RMT driver directly. You need an ESP32-family chip with an RMT peripheral and
    ESP-IDF v5.3 or later; CI builds the examples for the ESP32-C6 with ESP-IDF v5.5.

## 1. Wire it

| ESP32 | Strip | Notes |
| --- | --- | --- |
| GPIO (GPIO8 in the example) | DIN of the first LED | Through a 3.3 V to 5 V level shifter; see below |
| GND | GND (VSS) | One ground for the ESP32, the strip and its supply |
| | 5 V (VDD) | From a supply sized for the strip: up to about 60 mA per LED at full white |

A WS2812B powered from 5 V needs at least 3.5 V (0.7 × VDD) on DIN to see a high. An ESP32 drives 3.3 V, which is
below that: it often works on a short wire, but it is out of specification. A 74AHCT125 or 74HCT245 powered from
5 V fixes it. [Hardware](hardware.md) has the pin table, the power budget and the protection parts.

## 2. Add the driver

Add the repository as a submodule and copy the ESP-IDF component wrapper into your project's `components/`
folder. [Installation](installation.md) has the details and the alternatives.

```bash
git submodule add https://github.com/hardfoc/hf-ws2812-rmt-driver.git external/hf-ws2812-rmt-driver
cp -r external/hf-ws2812-rmt-driver/examples/esp32/components/hf_ws2812_rmt components/
```

Tell the wrapper where the driver is, in your top-level `CMakeLists.txt` before the `project.cmake` include, and
add `hf_ws2812_rmt` to the `REQUIRES` of your `main` component:

```cmake
set(HF_WS2812_RMT_ROOT ${CMAKE_SOURCE_DIR}/external/hf-ws2812-rmt-driver CACHE PATH "WS2812 driver checkout")
```

## 3. Light it

```cpp title="main.cpp"
--8<-- "tests/docs/quickstart.cpp:quickstart"
```

What each step does:

1. The `WS2812Strip` constructor creates an RMT transmit channel on the GPIO at 10 MHz, so the four timing
   arguments are in ticks of 100 ns: 4, 8, 9 and 5 give 400, 800, 900 and 500 ns, inside the WS2812B's ±150 ns
   window. They are also the Kconfig defaults ([Configuration](configuration.md)).
2. `Begin()` reports whether the RMT channel was set up; a failure (no free channel, a bad GPIO) is returned, not
   an abort.
3. Colours are packed `0xRRGGBB`; the driver sends green first, as a WS2812B expects.
4. `SetPixel()` writes the buffer; nothing reaches the LEDs until `Show()`, which scales by the brightness, encodes
   24 symbols per LED, appends the reset that latches the frame and blocks until the RMT has sent them.

The host CI compiles this example against the driver's headers, so it matches the current API.

## If it does not work

| Symptom | First thing to check |
| --- | --- |
| Every LED white or full on | The timing arguments: ticks are 100 ns, so values in ns or µs are far too long |
| Red and green swapped | Colour packing: `SetPixel()` takes `0xRRGGBB`; drop any `0xGGRRBB` workaround from older code |
| Nothing lights | Ground shared with the strip, data into DIN (not DOUT), the GPIO number |
| First LED flickers or shows random colours | Data level: add the level shifter, keep the data wire short |
| `Begin()` fails | No free RMT transmit channel, or an invalid GPIO ([Troubleshooting](troubleshooting.md)) |

## Next

- [API guide](api-guide.md): brightness, resizing, effects, several strips, errors.
- [Signal timing](timing.md): how the ticks, frame time and reset gap work.
- [Hardware](hardware.md): level shifting, power injection, protection parts.
