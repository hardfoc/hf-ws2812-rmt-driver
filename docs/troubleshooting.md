# Troubleshooting

This guide helps you diagnose and resolve common issues when using the WS2812 driver.

## Common Error Messages

### Error: ESP_ERR_INVALID_ARG

**Symptoms:**
- `begin()` or `ws2812ControlInit()` returns `ESP_ERR_INVALID_ARG`
- Initialization fails

**Causes:**
- Invalid GPIO number
- Invalid RMT channel
- GPIO not configured for output

**Solutions:**
1. **Check GPIO number**:
   ```cpp
   // Use valid GPIO (e.g., GPIO_NUM_18)
   WS2812Strip strip(GPIO_NUM_18, 0, 30, LedType::RGB);
   ```

2. **Check RMT channel**:
   - ESP32: Channels 0-7
   - ESP32-C6: Channels 0-3 only
   - Use -1 for auto-selection

3. **Verify GPIO configuration**: Ensure GPIO is not used by another peripheral

---

### Error: ESP_ERR_NO_MEM

**Symptoms:**
- Initialization fails with out of memory error

**Causes:**
- Too many LEDs configured
- Insufficient RMT memory blocks

**Solutions:**
1. **Reduce LED count**: Lower `NUM_LEDS` in Kconfig
2. **Check RMT memory**: Ensure other RMT channels aren't using all memory

---

## Hardware Issues

### LEDs Not Lighting Up

**Symptoms:**
- No LEDs light up
- Strip appears dead

**Checklist:**
- [ ] Verify power supply is connected and providing 5V
- [ ] Check data line (DIN) is connected to correct GPIO
- [ ] Verify ground connection (common ground required!)
- [ ] Check power supply can provide enough current
- [ ] Test with multimeter: power supply voltage

**Solutions:**
1. **Check wiring**:
   - Data line to GPIO
   - Power (5V) to VCC
   - Ground to GND (must be common with ESP32!)

2. **Test power supply**:
   - Measure voltage at strip (should be ~5V)
   - Check current capacity (60mA per LED at full brightness)

3. **Verify first LED**:
   - If first LED works but others don't, check data line continuity

---

### Wrong Colors

**Symptoms:**
- LEDs show wrong colors
- Colors are shifted or incorrect

**Causes:**
- LED type mismatch (RGB vs RGBW)
- Color order incorrect
- Timing issues

**Solutions:**
1. **Check LED type**:
   ```cpp
   // Use correct type
   WS2812Strip strip(GPIO_NUM_18, 0, 30, LedType::RGB);  // For RGB LEDs
   WS2812Strip strip(GPIO_NUM_18, 0, 30, LedType::RGBW); // For RGBW LEDs
   ```

2. **Verify color format**:
   - RGB: 0xRRGGBB (bytes: B, G, R in memory)
   - RGBW: 0xRRGGBBWW

3. **Check Kconfig**: Ensure `WS2812_LED_TYPE` matches your LEDs

---

### Flickering or Timing Issues

**Symptoms:**
- LEDs flicker
- Colors are unstable
- Timing errors

**Causes:**
- Wi-Fi interference
- Long data line
- Incorrect timing parameters
- CPU frequency changes

**Solutions:**
1. **Pin task to Core 1** (if using Wi-Fi):
   ```cpp
   xTaskCreatePinnedToCore(ledTask, "led", 4096, nullptr, 5, nullptr, 1);
   ```

2. **Reduce data line length**: Keep data line short (< 1 meter)

3. **Adjust timings**:
   ```cpp
   strip.setTimings(14, 52, 52, 52); // Try different values
   ```

4. **Add capacitor**: Place 1000µF capacitor across power supply

---

### Only First LED Works

**Symptoms:**
- First LED lights up correctly
- Remaining LEDs don't respond

**Causes:**
- Data line broken after first LED
- Insufficient power for full strip
- Timing issues

**Solutions:**
1. **Check data line continuity**: Test with multimeter
2. **Verify power supply**: Ensure adequate current capacity
3. **Check strip integrity**: Test individual LED segments

---

## Software Issues

### Compilation Errors

**Error: "ws2812_control.h: No such file"**

**Solution:**
- Ensure component is in `components/` directory
- Check `CMakeLists.txt` includes the component
- Verify include paths

**Error: "Undefined reference"**

**Solution:**
- Ensure source files are included in `CMakeLists.txt`
- Check component is linked: `REQUIRES hf-ws2812-rmt`

---

### Runtime Errors

**Initialization Fails**

**Checklist:**
- [ ] GPIO is valid and available
- [ ] RMT channel is available
- [ ] Sufficient memory available
- [ ] Kconfig settings are correct

**LEDs Don't Update**

**Checklist:**
- [ ] `begin()` was called
- [ ] `show()` is called after setting pixels
- [ ] LED count matches configuration
- [ ] Colors are set before `show()`

---

## Debugging Tips

### Enable Debug Output

Add debug prints:

```cpp
esp_err_t ret = strip.begin();
if (ret != ESP_OK) {
    ESP_LOGE("WS2812", "begin() failed: %s", esp_err_to_name(ret));
}
```

### Check RMT Status

```cpp
// Verify RMT channel is configured
// Check ESP-IDF logs for RMT errors
```

### Test with Single LED

```cpp
// Start with one LED to verify basic functionality
WS2812Strip strip(GPIO_NUM_18, 0, 1, LedType::RGB);
strip.begin();
strip.setPixel(0, 0xFF0000); // Red
strip.show();
```

---

## FAQ

### Q: Why do my LEDs flicker when Wi-Fi is active?

**A:** Wi-Fi can interfere with RMT timing. Pin your LED task to Core 1:
```cpp
xTaskCreatePinnedToCore(ledTask, "led", 4096, nullptr, 5, nullptr, 1);
```

### Q: How many LEDs can I control?

**A:** Depends on:
- Available memory (each LED uses buffer space)
- Power supply capacity (~60mA per LED)
- Data line length (longer = more timing issues)

Typically: 30-100 LEDs work well, 200+ may need power injection.

### Q: Can I use 3.3V logic levels?

**A:** For short strips (< 1 meter), 3.3V often works. For reliability, use a level shifter to 5V.

### Q: Why are colors wrong?

**A:** Common causes:
1. LED type mismatch (RGB vs RGBW)
2. Color byte order
3. Timing issues

Check Kconfig `WS2812_LED_TYPE` matches your LEDs.

---

## Getting More Help

If you're still experiencing issues:

1. Check the [API Reference](api_reference.md) for method details
2. Review [Examples](examples.md) for working code
3. Search existing issues on GitHub
4. Open a new issue with:
   - Description of the problem
   - Steps to reproduce
   - Hardware setup details
   - Error messages/logs
   - Kconfig settings

---

**Navigation**
⬅️ [Examples](examples.md) | [Back to Index](index.md)

