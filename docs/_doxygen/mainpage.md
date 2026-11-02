# WS2812 RMT Driver API reference {#mainpage}

The classes and functions of the HardFOC WS2812 driver for ESP32, generated from the headers.

Start with WS2812Strip, one chain of LEDs on one GPIO, and WS2812Animator, the effects that draw into it.
WS2812MultiAnimator runs one effect over several strips; ws2812::RmtTx is the RMT transmit channel underneath.
The C functions in ws2812_control.h are the older single-strip interface.

The guides, with examples that compile, are on the documentation site: <a href="../">WS2812 RMT Driver
documentation</a>.
