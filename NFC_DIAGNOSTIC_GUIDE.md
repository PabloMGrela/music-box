# 🔍 PN532 NFC Reader Diagnostic Guide

## The test program is already loaded on your ESP32

### What does the test program do?

The program runs 4 automatic tests:

1. ✅ **Pin Configuration** - Verifies pins are correct
2. ✅ **Initialization** - Attempts to communicate with PN532
3. ✅ **Firmware Version** - Reads PN532 chip version
4. ✅ **SAM Configuration** - Configures module to read tags

### 📺 To view results:

#### Option 1: Use the test script
```bash
./test_nfc.sh
```

#### Option 2: Manual monitor
```bash
~/.platformio/penv/bin/platformio device monitor -e nfc_test
```

Then press the **RESET** (or **EN**) button on your ESP32.

---

## 🔧 If PN532 is NOT detected

You'll see this message:
```
✗ FAILED: Didn't find PN532 board
```

### Verification Steps:

#### 1. **Verify PN532 SPI Mode**
Most PN532 modules have **DIP switches** or **jumpers**:

**Correct configuration for SPI:**
- SW1: **ON** (or I0 = 0)
- SW2: **OFF** (or I1 = 1)

Other modes (incorrect):
- HSU/UART: SW1=OFF, SW2=OFF
- I2C: SW1=OFF, SW2=ON

#### 2. **Verify Physical Connections**

| ESP32 Pin | PN532 Pin | Cable/Function |
|-----------|-----------|----------------|
| 3.3V      | VCC       | Power (red) |
| GND       | GND       | Ground (black) |
| GPIO14    | SCK       | Clock (yellow/white) |
| GPIO12    | MISO      | Data IN (green) |
| GPIO27    | MOSI      | Data OUT (blue) |
| GPIO15    | SS/CS     | Chip Select (purple) |

**Important notes:**
- ⚠️ **DO NOT connect to 5V** - PN532 uses 3.3V
- Short cables (<10cm) work better
- MISO should have pull-up (most modules include it)

#### 3. **Verify Power Supply**

With a multimeter:
- Measure between VCC and GND of PN532
- Should read **3.3V** (between 3.2V and 3.4V is OK)
- If 0V: connection problem or ESP32 power issue
- If 5V: ⚠️ **DISCONNECT IMMEDIATELY** - Connected to 5V

**PN532 LED:**
- Should be on (red or green depending on model)
- If not lit: power supply problem

#### 4. **Check Soldering (if module without pins)**

If you soldered the pins yourself:
- Check each pin with magnifier
- Look for cold solder joints (dull, porous)
- Verify continuity with multimeter
- Check for no short circuits between pins

#### 5. **Try Shorter Cables**

SPI can be sensitive to long cables:
- Use Dupont cables <10cm
- Even better: solder directly to ESP32
- Keep cables away from noise sources

#### 6. **Check GPIO15 (Strapping Pin)**

GPIO15 is a special ESP32 pin:
- Must be HIGH during boot
- If you have boot problems, try another pin for CS:
  - GPIO5 is a good alternative
  - GPIO4 also works
  
To change CS pin:
1. Edit `include/config.h`
2. Change `#define NFC_SS 15` to `#define NFC_SS 5`
3. Reconnect PN532 CS cable to GPIO5

---

## ✅ If PN532 IS DETECTED

You'll see something like:
```
✓ PN532 Found!
Chip: PN532
Firmware Version: 1.6

Now scanning for NFC tags...
Place an NFC tag near the reader
```

### NFC Tag Test:

1. **Place an NFC tag near** (card, keychain, NFC phone)
2. You should see:
```
========================================
   NFC TAG DETECTED!
========================================
UID Length: 4 bytes
UID Value: AB CD EF 01
Card Type: Mifare Classic (4-byte UID)
========================================
```

3. **Test different tags:**
   - Public transport cards
   - Amiibos
   - Blank NFC tags
   - Smartphone with NFC (doesn't always work)

---

## 🔄 Return to Main Program

Once you confirm PN532 works:

```bash
# Upload main program again
~/.platformio/penv/bin/platformio run -e esp32dev --target upload

# View monitor
~/.platformio/penv/bin/platformio device monitor -e esp32dev
```

---

## 🆘 Common Problems

### "Can't find serial monitor"
```bash
# List available ports
ls /dev/cu.*

# Should show something like: /dev/cu.usbserial-XXXX
```

### "Permission denied when connecting"
```bash
# On macOS, grant permissions:
sudo chmod 666 /dev/cu.usbserial-*
```

### "ESP32 won't boot after connecting PN532"
- Disconnect GPIO15 (PN532 CS)
- Boot ESP32
- Reconnect GPIO15
- Or change CS to another pin (see point 6 above)

### "Detects tag but then doesn't respond"
- Cables too long
- Interference from other devices
- Insufficient power (try external power supply)

---

## 📊 Normal Voltage Readings

With multimeter on PN532:
- **VCC to GND**: 3.3V (±0.1V)
- **MISO at rest**: 3.3V (active pull-up)
- **MOSI at rest**: Variable
- **SCK at rest**: HIGH (~3.3V) or LOW (0V)
- **CS at rest**: HIGH (3.3V) - very important

---

## 📝 System Information

**ESP32 pins used:**
- GPIO14 (SCK) - Software SPI Clock
- GPIO12 (MISO) - Master In Slave Out
- GPIO27 (MOSI) - Master Out Slave In  
- GPIO15 (CS) - Chip Select

**Library:** Adafruit PN532 v1.3.4

**Protocol:** Software SPI (bit-banging)

---

## 🔗 Additional Resources

- [PN532 Datasheet](https://www.nxp.com/docs/en/nxp/data-sheets/PN532_C1.pdf)
- [Adafruit PN532 Guide](https://learn.adafruit.com/adafruit-pn532-rfid-nfc)
- [ESP32 Pinout](https://randomnerdtutorials.com/esp32-pinout-reference-gpios/)
