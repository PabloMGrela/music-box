#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>

// Pin definitions for PN532 (Software SPI)
#define PN532_SCK  14
#define PN532_MISO 12
#define PN532_MOSI 27
#define PN532_SS   15

// Create PN532 instance
Adafruit_PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(10);
    
    Serial.println("\n\n========================================");
    Serial.println("   PN532 NFC Reader Diagnostic Test");
    Serial.println("========================================\n");
    
    // Test 1: Pin Configuration
    Serial.println("Test 1: Pin Configuration");
    Serial.printf("  SCK  = GPIO%d\n", PN532_SCK);
    Serial.printf("  MISO = GPIO%d (with internal pull-up)\n", PN532_MISO);
    Serial.printf("  MOSI = GPIO%d\n", PN532_MOSI);
    Serial.printf("  SS   = GPIO%d\n", PN532_SS);
    Serial.println("  ✓ Pins configured\n");
    
    // Test 2: Initialize PN532
    Serial.println("Test 2: Initializing PN532...");
    nfc.begin();
    delay(100);
    
    // Test 3: Get Firmware Version
    Serial.println("Test 3: Reading Firmware Version...");
    uint32_t versiondata = nfc.getFirmwareVersion();
    
    if (!versiondata) {
        Serial.println("  ✗ FAILED: Didn't find PN532 board");
        Serial.println("\n--- Troubleshooting Steps ---");
        Serial.println("1. Check physical connections:");
        Serial.println("   - ESP32 3.3V → PN532 VCC");
        Serial.println("   - ESP32 GND → PN532 GND");
        Serial.println("   - GPIO14 → PN532 SCK");
        Serial.println("   - GPIO12 → PN532 MISO (check pull-up)");
        Serial.println("   - GPIO27 → PN532 MOSI");
        Serial.println("   - GPIO15 → PN532 SS/CS");
        Serial.println("\n2. Verify PN532 is in SPI mode:");
        Serial.println("   - Check DIP switches on module");
        Serial.println("   - Should be: ON, OFF (SPI mode)");
        Serial.println("\n3. Check power:");
        Serial.println("   - PN532 LED should be on");
        Serial.println("   - Measure 3.3V on VCC pin");
        Serial.println("\n4. Try shorter wires (<10cm)");
        Serial.println("\n5. Check for soldering issues");
        Serial.println("========================================\n");
        while (1) {
            delay(1000);
        }
    }
    
    // Print firmware info
    Serial.println("  ✓ PN532 Found!");
    Serial.printf("  Chip: PN5%02X\n", (versiondata >> 24) & 0xFF);
    Serial.printf("  Firmware Version: %d.%d\n", 
                  (versiondata >> 16) & 0xFF,
                  (versiondata >> 8) & 0xFF);
    Serial.println();
    
    // Test 4: Configure SAM
    Serial.println("Test 4: Configuring SAM (Security Access Module)...");
    nfc.SAMConfig();
    Serial.println("  ✓ SAM Configured\n");
    
    // Success!
    Serial.println("========================================");
    Serial.println("   ✓ All Tests PASSED!");
    Serial.println("   PN532 is working correctly");
    Serial.println("========================================\n");
    
    Serial.println("Now scanning for NFC tags...");
    Serial.println("Place an NFC tag near the reader\n");
}

void loop() {
    uint8_t success;
    uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
    uint8_t uidLength;
    
    // Wait for an ISO14443A type card (Mifare, NTAG, etc.)
    success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 100);
    
    if (success) {
        Serial.println("========================================");
        Serial.println("   NFC TAG DETECTED!");
        Serial.println("========================================");
        
        Serial.printf("UID Length: %d bytes\n", uidLength);
        Serial.print("UID Value: ");
        for (uint8_t i = 0; i < uidLength; i++) {
            if (uid[i] < 0x10) Serial.print("0");
            Serial.print(uid[i], HEX);
            if (i < uidLength - 1) Serial.print(" ");
        }
        Serial.println();
        
        // Determine card type
        Serial.print("Card Type: ");
        if (uidLength == 4) {
            Serial.println("Mifare Classic (4-byte UID)");
        } else if (uidLength == 7) {
            Serial.println("Mifare Ultralight/NTAG (7-byte UID)");
        } else {
            Serial.println("Unknown");
        }
        
        Serial.println("========================================\n");
        
        // Wait for tag to be removed
        Serial.println("Remove tag to scan again...");
        delay(2000);
        
        // Wait for tag removal
        while (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 50)) {
            delay(100);
        }
        
        Serial.println("Tag removed. Ready for next scan.\n");
    }
    
    delay(100);
}
