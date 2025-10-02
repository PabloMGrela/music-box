#ifndef STORAGE_H
#define STORAGE_H

#include <Arduino.h>
#include <SD.h>
#include <ArduinoJson.h>
#include <vector>

struct NFCLink {
    String uid;
    String songPath;
};

class Storage {
public:
    Storage();
    
    // SD Card Management
    bool begin();
    bool isMounted() { return _mounted; }
    
    // Music File Management
    std::vector<String> listMusicFiles();
    bool deleteMusicFile(const String& filename);
    bool musicFileExists(const String& filename);
    String getMusicPath(const String& filename);
    
    // NFC Links Management
    bool loadNFCLinks();
    bool saveNFCLinks();
    bool linkNFC(const String& uid, const String& songPath);
    bool unlinkNFC(const String& uid);
    String getSongForNFC(const String& uid);
    std::vector<NFCLink> getAllLinks();
    
private:
    bool _mounted;
    std::vector<NFCLink> _nfcLinks;
    
    void ensureMusicDirectory();
    String uidToString(const uint8_t* uid, uint8_t length);
};

extern Storage storage;

#endif // STORAGE_H
