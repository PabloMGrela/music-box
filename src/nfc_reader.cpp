#include "nfc_reader.h"
#include "config.h"

NFCReader nfcReader;

NFCReader::NFCReader() 
    : _nfc(nullptr), _lastReadTime(0), _lastTagTime(0), _hasNewTag(false), _onTagDetected(nullptr) {}

bool NFCReader::begin() {
    // Initialize software SPI for PN532
    _nfc = new Adafruit_PN532(NFC_SCK, NFC_MISO, NFC_MOSI, NFC_SS);
    
    _nfc->begin();
    
    uint32_t versiondata = _nfc->getFirmwareVersion();
    if (!versiondata) {
        Serial.println("Didn't find PN532 board");
        return false;
    }
    
    Serial.print("Found chip PN5");
    Serial.println((versiondata >> 24) & 0xFF, HEX);
    Serial.print("Firmware ver. ");
    Serial.print((versiondata >> 16) & 0xFF, DEC);
    Serial.print('.');
    Serial.println((versiondata >> 8) & 0xFF, DEC);
    
    // Configure board to read RFID tags
    _nfc->SAMConfig();
    
    Serial.println("NFC Reader initialized");
    return true;
}

void NFCReader::loop() {
    unsigned long currentTime = millis();
    
    // Poll every NFC_POLL_INTERVAL ms
    if (currentTime - _lastReadTime < NFC_POLL_INTERVAL) {
        return;
    }
    
    _lastReadTime = currentTime;
    
    uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
    uint8_t uidLength;
    
    // Check for a tag
    bool success = _nfc->readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 50);
    
    if (success) {
        String uidStr = uidToString(uid, uidLength);
        
        // Check if this is a different tag or enough time has passed since last detection
        if (uidStr != _currentUID || 
            (currentTime - _lastTagTime > NFC_DEBOUNCE_TIME)) {
            
            _currentUID = uidStr;
            _lastUID = uidStr;
            _lastTagTime = currentTime;  // Update last tag detection time
            _hasNewTag = true;
            
            Serial.print("NFC Tag detected: ");
            Serial.println(uidStr);
            
            if (_onTagDetected) {
                _onTagDetected(uidStr);
            }
        }
    } else {
        // No tag present
        if (_currentUID != "") {
            Serial.println("NFC Tag removed");
            _currentUID = "";
        }
    }
}

String NFCReader::uidToString(uint8_t* uid, uint8_t uidLength) {
    String uidStr = "";
    for (uint8_t i = 0; i < uidLength; i++) {
        if (uid[i] < 0x10) {
            uidStr += "0";
        }
        uidStr += String(uid[i], HEX);
    }
    uidStr.toUpperCase();
    return uidStr;
}
