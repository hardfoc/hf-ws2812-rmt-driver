# Hardware Setup

![WS2812 NRZ bit-encoding and 24-bit pixel cascade](assets/ws2812-nrz.svg)

This guide covers the physical connections and hardware requirements for WS2812 LED strips.

## Pin Connections

### Basic Connections

```
ESP32              WS2812 Strip
─────────────────────────────────
GPIO (e.g., 18) ─── DIN (Data In)
5V/3.3V         ─── VCC (Power)
GND             ─── GND (Ground)
```

### Connection Details

| ESP32 Pin | WS2812 Pin | Description | Notes |
|-----------|------------|-------------|-------|
| GPIO (configurable) | DIN | Data input | Any output-capable GPIO |
| 5V or 3.3V | VCC | Power supply | See power requirements below |
| GND | GND | Ground | Common ground required |

**Important**: The data line (DIN) must be connected to a GPIO capable of output. Most GPIOs on ESP32 support this.

## Power Requirements

### Power Supply Considerations

WS2812 LEDs can draw significant current:
- **Per LED**: ~60mA at full brightness white
- **30 LEDs**: ~1.8A at full brightness
- **100 LEDs**: ~6A at full brightness

### Power Supply Options

1. **Small Strips (< 20 LEDs)**: Can use ESP32's 5V pin (if available) or 3.3V with level shifter
2. **Medium Strips (20-50 LEDs)**: Use external 5V power supply (2-3A recommended)
3. **Large Strips (> 50 LEDs)**: Use external 5V power supply (5A+ recommended)

### Power Wiring

For strips requiring external power:

```
External 5V Power Supply
    │
    ├── VCC ─── WS2812 Strip VCC
    │
    └── GND ───┬── WS2812 Strip GND
               │
               └── ESP32 GND (COMMON GROUND REQUIRED!)

ESP32 GPIO ─── WS2812 Strip DIN
```

**Critical**: Always connect grounds together! The ESP32 GND and power supply GND must be connected.

## Level Shifting (Optional)

WS2812 LEDs expect **5V logic levels**, but ESP32 outputs **3.3V**. For short strips (< 1 meter), 3.3V often works, but for reliability:

- Use a **level shifter** (e.g., 74AHCT125) for the data line
- Or use a **resistor divider** (not recommended for long strips)

## Physical Layout Recommendations

- **Keep data line short**: Minimize distance between ESP32 and first LED (< 1 meter ideal)
- **Add capacitor**: Place 1000µF capacitor across power supply near strip
- **Power injection**: For long strips, inject power at multiple points
- **Shielding**: Keep data line away from noise sources

## Example Wiring Diagram

![WS2812 wiring](assets/ws2812-wiring.svg)

### Small Strip (< 20 LEDs)

```
                    WS2812 Strip
                    ┌─────────────┐
ESP32 GPIO18 ───────┤ DIN         │
ESP32 5V     ───────┤ VCC         │
ESP32 GND    ───────┤ GND         │
                    └─────────────┘
```

### Large Strip (External Power)

```
External 5V PSU     WS2812 Strip
    │               ┌─────────────┐
    ├── VCC ────────┤ VCC         │
    │               │             │
    └── GND ────┬───┤ GND         │
                │   └─────────────┘
                │
ESP32 GPIO18 ───┼─── DIN
                │
ESP32 GND ──────┘
```

## GPIO Selection

**Recommended GPIOs**:
- GPIO 18 (default in Kconfig)
- GPIO 19, 21, 22, 23
- Any output-capable GPIO

**Avoid**:
- GPIO 0 (boot mode, may cause issues)
- GPIO 6-11 (SPI flash, not available)
- Input-only GPIOs

## Next Steps

- Verify connections with a multimeter
- Proceed to [Quick Start](quickstart.md) to test the connection
- Review [Configuration](configuration.md) for GPIO and timing settings

---

**Navigation**
⬅️ [Quick Start](quickstart.md) | [Next: Platform Integration ➡️](platform_integration.md) | [Back to Index](index.md)

