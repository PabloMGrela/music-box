#ifndef NFC_READER_H
#define NFC_READER_H

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>

class NFCReader {
public:
    NFCReader();
    
    bool begin();
    void loop();
    
    String getLastUID() { return _lastUID; }
    bool hasNewTag() { return _hasNewTag; }
    void clearNewTag() { _hasNewTag = false; }
    void clearLastUID() { _lastUID = ""; }  // Clear last UID manually
    
    // Callback when a tag is detected
    void setOnTagDetected(void (*callback)(String uid)) {
        _onTagDetected = callback;
    }
    
private:
    Adafruit_PN532* _nfc;
    String _lastUID;
    String _currentUID;
    unsigned long _lastReadTime;
    unsigned long _lastTagTime;  // Time when tag was last detected
    bool _hasNewTag;
    void (*_onTagDetected)(String uid);
    
    String uidToString(uint8_t* uid, uint8_t uidLength);
    bool readTag();
};

extern NFCReader nfcReader;

#endif // NFC_READER_H
