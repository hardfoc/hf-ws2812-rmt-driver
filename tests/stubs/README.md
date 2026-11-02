# ESP-IDF stubs for the host build

The smallest subset of the ESP-IDF headers this driver includes (`driver/rmt_*.h`, `driver/gpio.h`, `esp_err.h`,
`esp_check.h`, `esp_log.h`, FreeRTOS, `sdkconfig.h`), with the same type and function names and layouts as
ESP-IDF v5.5. They let the host build compile the real driver sources, the unit tests and the documentation
snippets. `fake_rmt.cpp` implements the RMT calls by recording what the driver transmits; nothing here drives
hardware.

`sdkconfig.h` holds the Kconfig defaults from the repository's `Kconfig`.
