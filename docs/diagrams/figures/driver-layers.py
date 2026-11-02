"""The layers of the driver, from your application down to the LEDs. There is no platform port: the driver is
ESP-IDF code.

Reads: From top to bottom. Your application, the only code you write, either runs an effect or sets pixels itself.
The effects (WS2812Animator and WS2812MultiAnimator, the Effect values listed from inc/ws2812_effects.hpp) draw
into a WS2812Strip. The strip (inc/ws2812_cpp.hpp, public methods listed) keeps the pixel buffer, applies the
brightness and turns every bit into one RMT symbol with the configured high and low times. ws2812::RmtTx
(inc/rmt_wrapper.hpp) owns one RMT transmit channel at 10 MHz and sends the symbols, blocking until they are out.
Below it is ESP-IDF's RMT driver (the calls listed from rmt_wrapper.hpp) and the RMT peripheral, which times the
pulses on the GPIO that feeds the first LED.
"""

import re
from pathlib import Path

from hardfoc_mkdocs_theme.figkit import Fig

ROOT = Path(__file__).resolve().parents[3]
EFFECTS = "inc/ws2812_effects.hpp"
STRIP = "inc/ws2812_cpp.hpp"
RMT = "inc/rmt_wrapper.hpp"


def data():
    eff = (ROOT / EFFECTS).read_text()
    effects = [e.strip() for e in re.search(r"enum class Effect \{([^}]*)\}", eff).group(1).split(",")]
    strip = (ROOT / STRIP).read_text()
    body = strip[strip.index("class WS2812Strip") : strip.index("private:")]
    methods = []
    for name in re.findall(r"^\s+(?:static\s+)?(?:esp_err_t|void|uint32_t)\s+(\w+)\(", body, re.M):
        if name not in methods:
            methods.append(name)
    rmt = (ROOT / RMT).read_text()
    tx = rmt[rmt.index("class RmtTx") : rmt.index("class RmtRx")]
    idf = []
    for name in ("rmt_new_tx_channel", "rmt_enable", "rmt_new_copy_encoder", "rmt_transmit", "rmt_tx_wait_all_done"):
        if re.search(rf"\b{name}\(", tx):
            idf.append(name)
    return {
        "effects": effects,
        "strip_methods": methods,
        "idf_calls": idf,
        "sources": [f"{EFFECTS}:WS2812Animator::Effect", f"{STRIP}:WS2812Strip", f"{RMT}:ws2812::RmtTx"],
    }


def build(d):
    f = Fig(
        "driver-layers",
        "From pixels to pulses, on ESP-IDF",
        alt="Layer diagram: your application drives the WS2812Animator effects or a WS2812Strip directly; the strip "
        "turns pixels into RMT symbols and hands them to ws2812::RmtTx, which calls the ESP-IDF RMT driver; the RMT "
        "peripheral drives one GPIO connected to the first LED of the WS2812 chain.",
        width=1200,
        kicker="Architecture",
        subtitle="There is no port to write: the driver calls ESP-IDF's RMT driver directly, and the RMT peripheral "
        "times every bit.",
        profile="overview",
        legend=[("brand", "this library"), ("teal", "you write"), ("slate", "ESP-IDF and hardware")],
    )
    W = f.body_w
    app = f.card(0, 0, W, "Your application", ["status lights, a task that ticks an effect, a UI ..."], tone="teal",
                 accent=True)
    top = app.b + 40
    chips_h = f.chips(16, f.region_inner_top(top), W - 32, d["effects"], tone="brand")
    eff = f.region(0, top, W, f.region_inner_top(top) - top + chips_h + 18, "Effects (optional)", tone="brand",
                   sub="WS2812Animator, WS2812MultiAnimator; inc/ws2812_effects.hpp")
    top2 = eff.b + 40
    chips2_h = f.chips(16, f.region_inner_top(top2), W - 32, [m + "()" for m in d["strip_methods"]], tone="brand")
    strip = f.region(0, top2, W, f.region_inner_top(top2) - top2 + chips2_h + 18, "WS2812Strip", tone="brand",
                     sub="pixel buffer, brightness, one RMT symbol per bit; inc/ws2812_cpp.hpp")
    rmt = f.card(0, strip.b + 40, W, "ws2812::RmtTx",
                 ["one RMT transmit channel at 10 MHz (100 ns ticks), copy encoder, blocking transmit;",
                  "inc/rmt_wrapper.hpp"], tone="brand", accent=True)
    idf = f.card(0, rmt.b + 40, W, "ESP-IDF RMT driver", ["  ".join(c + "()" for c in d["idf_calls"])],
                 tone="slate", mono=True)
    hw = f.card(0, idf.b + 40, W, "RMT peripheral, GPIO, WS2812 chain",
                ["one data line, 800 kbit/s NRZ, 24 bits per LED (32 for RGBW), latched by at least 50 µs of low"],
                tone="slate")
    for a, b in ((app, eff), (eff, strip), (strip, rmt), (rmt, idf), (idf, hw)):
        f.connect(a, b, "bottom", "top", tone="rail")
    return f
