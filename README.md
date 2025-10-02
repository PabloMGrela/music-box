# ESP32 MusicBox 🎵

An NFC-controlled music player for children based on ESP32. Screen-free design for simple, intuitive operation.

## 🎯 Features

- **NFC Playback Control**: Place an NFC tag near the reader to play its associated song
- **Simple Controls**: 
  - Place tag → play song
  - Place same tag while playing → pause/resume
  - Place different tag → switch to new song
- **Web Interface**: Configure songs and tags from any device
- **Screen-Free**: Designed for children without visual interaction required

## 🔧 Hardware Requirements

### Main Components
- **ESP32 DevKit** (WROOM or similar)
- **MAX98357A** (I2S Audio Amplifier)
- **PN532** (NFC Reader Module)
- **MicroSD Card Module** (for music storage)
- **Speaker** (4-8Ω, connected to MAX98357A)
- **NFC Tags** (Mifare Classic, NTAG213/215/216, etc.)

### Wiring Connections

#### SD Card (Hardware VSPI)
| Signal | ESP32 Pin |
|--------|-----------|
| CS     | GPIO 13   |
| SCK    | GPIO 18   |
| MISO   | GPIO 19   |
| MOSI   | GPIO 23   |

#### I2S Audio (MAX98357A)
| Signal | ESP32 Pin |
|--------|-----------|
| BCLK   | GPIO 26   |
| LRCK   | GPIO 25   |
| DATA   | GPIO 22   |

#### NFC PN532 (Software SPI)
| Signal | ESP32 Pin |
|--------|-----------|
| SCK    | GPIO 14   |
| MISO   | GPIO 12   |
| MOSI   | GPIO 27   |
| CS     | GPIO 15   |

### Wiring Diagram
```
                +------------------- ESP32 DEVKIT -------------------+
                |                                                    |
        3V3 ----+----------------------------------------------------+
        GND ----+--------------------------------------+-------------+
                              VSPI (microSD)           |   I2S
   SD_CS (GPIO13)  ---------[ microSD CS ]             |   BCLK GPIO26 ---> MAX98357A BCLK
   SCK   (GPIO18)  ---------[ microSD CLK ]            |   LRCK GPIO25 ---> MAX98357A LRCK
   MISO  (GPIO19)  <--------[ microSD DO ]             |   DATA GPIO22 ---> MAX98357A DIN
   MOSI  (GPIO23)  --------->[ microSD DI ]            |
                                                      |
                 PN532 (software bit-bang SPI)        |
   GPIO14 (SCK)   --------->[ PN532 SCK ]              |
   GPIO12 (MISO)  <--------[ PN532 MISO ]              |
   GPIO27 (MOSI)  --------->[ PN532 MOSI ]             |
   GPIO15 (CS)    --------->[ PN532 SS ]               |
                                                      |
                 WiFi AP (MusicBox / musicbox123)      |
                +--------------------------------------+
```

## 📦 Installation and Setup

### 1. Environment Setup

You need to have installed:
- [Visual Studio Code](https://code.visualstudio.com/)
- [PlatformIO IDE](https://platformio.org/install/ide?install=vscode)

### 2. Clone and Compile

```bash
# Open the project in VS Code
# PlatformIO will automatically install all dependencies

# Compile the project
pio run

# Upload to ESP32 (connected via USB)
pio run --target upload

# View serial monitor
pio device monitor
```

### 3. Prepare the SD Card

1. Format the SD card as **FAT32**
2. Create folder `/music/` in the root
3. (Optional) Copy initial MP3 files to `/music/`

### 4. First Connection

1. **Power the ESP32** (USB or 5V)
2. **Search for WiFi network**: `MusicBox`
3. **Connect** with password: `musicbox123`
4. **Open browser** at: `http://192.168.4.1`

## 🌐 Using the Web Interface

### Song Management

1. **Upload Song**: 
   - Click on the upload area
   - Select an MP3 file
   - Wait for upload to complete

2. **Delete Song**:
   - Click "Delete" next to the song

### NFC Tag Management

1. **Link Tag**:
   - Click "Link New Tag"
   - Place NFC tag near the reader
   - UID will be detected automatically
   - Select the song to link
   - Click "Link"

2. **Unlink Tag**:
   - Click "Unlink" next to the tag

## 🎵 Daily Usage

1. **Play**: Place a linked NFC tag near the reader
2. **Pause/Resume**: Place the same tag while playing
3. **Change song**: Place a different tag

## 📡 REST API

### Songs

```http
# List songs
GET /api/songs

# Upload song
POST /api/songs/upload
Content-Type: multipart/form-data

# Delete song
DELETE /api/songs/{filename}
```

### NFC Tags

```http
# List linked tags
GET /api/tags

# Link tag
POST /api/tags/link
Content-Type: application/json
{
  "uid": "A1B2C3D4",
  "song": "song.mp3"
}

# Unlink tag
DELETE /api/tags/{uid}

# Scan tag (polling)
GET /api/tags/scan
```

### Status

```http
# Player status
GET /api/status
```

## ⚙️ Advanced Configuration

### Change WiFi Credentials

Edit `include/config.h`:
```cpp
#define WIFI_SSID "MyMusicBox"
#define WIFI_PASSWORD "mypassword123"
```

### Adjust Default Volume

Edit `include/config.h`:
```cpp
#define DEFAULT_VOLUME 0.8f  // 0.0 to 1.0
```

### Change NFC Detection Timings

Edit `include/config.h`:
```cpp
#define NFC_POLL_INTERVAL 100    // ms between reads
#define NFC_DEBOUNCE_TIME 1500   // ms for debounce
```

## 🐛 Troubleshooting

### PN532 Not Detected
- Verify connections (especially MISO with pull-up)
- Ensure CS is HIGH when not in use
- Check stable 3.3V power supply

### SD Card Won't Mount
- Verify FAT32 format
- Check SPI connections
- Try a different SD card
- Shorten cables if they're too long

### Audio with Clicks or Noise
- Add 100µF capacitor near MAX98357A
- Verify stable 5V power supply
- Separate audio cables from other wires

### Won't Boot After Programming
- GPIO15 can interfere with boot
- Reassign PN532 CS to another pin (e.g., GPIO5)
- Keep GPIO15 HIGH during boot

## 📝 Data Persistence

- **Music**: MP3 files in `/music/` on SD card
- **NFC Links**: File `/nfc_links.json` on SD card

Example of `nfc_links.json`:
```json
{
  "links": [
    {"uid": "A1B2C3D4", "song": "song1.mp3"},
    {"uid": "E5F6A7B8", "song": "song2.mp3"}
  ]
}
```

## 🔒 Security

- WiFi network protected by password
- No Internet access (local AP only)
- Files only accessible from local network

## 📄 License

This project is open source and available under the MIT License.

## 🤝 Contributing

Contributions are welcome. Please:
1. Fork the project
2. Create a branch for your feature
3. Commit your changes
4. Push to the branch
5. Open a Pull Request

## 📞 Support

To report issues or suggestions, open an issue in the repository.

---

**Enjoy your MusicBox!** 🎵
