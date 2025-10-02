#ifndef CONFIG_H
#define CONFIG_H

// ============================================================================
// PIN DEFINITIONS (Based on Hardware Reference Document)
// ============================================================================

// SD Card (VSPI)
#define SD_CS    13
#define SD_SCK   18
#define SD_MISO  19
#define SD_MOSI  23

// I2S Audio (MAX98357A)
#define I2S_BCLK 26
#define I2S_LRC  25
#define I2S_DOUT 22

// NFC PN532 (Software SPI)
#define NFC_SCK  14
#define NFC_MISO 12
#define NFC_MOSI 27
#define NFC_SS   15

// Optional Volume Pot (Future use)
#define POT_PIN  34

// ============================================================================
// WIFI CONFIGURATION
// ============================================================================
#define WIFI_SSID "MusicBox"
#define WIFI_PASSWORD "musicbox123"
#define WIFI_CHANNEL 6
#define MAX_CONNECTIONS 4

// ============================================================================
// STORAGE CONFIGURATION
// ============================================================================
#define MUSIC_DIR "/music"
#define NFC_LINKS_FILE "/nfc_links.json"
#define MAX_FILENAME_LENGTH 64

// ============================================================================
// AUDIO CONFIGURATION
// ============================================================================
#define AUDIO_SAMPLE_RATE 44100
#define AUDIO_BUFFER_SIZE 8192  // Increased from 2048 to reduce audio stuttering
#define DEFAULT_VOLUME 0.8f  // 0.0 to 1.0

// ============================================================================
// NFC CONFIGURATION
// ============================================================================
#define NFC_POLL_INTERVAL 500    // ms between NFC polls (increased to minimize CPU interference with audio)
#define NFC_DEBOUNCE_TIME 1500   // ms to debounce same tag
#define NFC_UID_MAX_LENGTH 7     // Maximum UID length

// ============================================================================
// WEB SERVER CONFIGURATION
// ============================================================================
#define WEB_SERVER_PORT 80
#define MAX_UPLOAD_SIZE (10 * 1024 * 1024)  // 10MB max file size

#endif // CONFIG_H
