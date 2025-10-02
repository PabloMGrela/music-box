#include "storage.h"
#include "config.h"
#include <SPI.h>

Storage storage;

Storage::Storage() : _mounted(false) {}

bool Storage::begin() {
    // Explicitly initialize SPI for SD card with correct pins
    SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
    delay(100);  // Give SPI time to stabilize
    
    if (!SD.begin(SD_CS, SPI)) {
        Serial.println("SD Card Mount Failed");
        return false;
    }
    
    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE) {
        Serial.println("No SD card attached");
        return false;
    }
    
    Serial.println("SD Card mounted successfully");
    _mounted = true;
    
    ensureMusicDirectory();
    loadNFCLinks();
    
    return true;
}

void Storage::ensureMusicDirectory() {
    if (!SD.exists(MUSIC_DIR)) {
        if (SD.mkdir(MUSIC_DIR)) {
            Serial.println("Created /music directory");
        } else {
            Serial.println("Failed to create /music directory");
        }
    }
}

std::vector<String> Storage::listMusicFiles() {
    std::vector<String> files;
    
    File root = SD.open(MUSIC_DIR);
    if (!root || !root.isDirectory()) {
        Serial.println("Failed to open music directory");
        return files;
    }
    
    File file = root.openNextFile();
    while (file) {
        if (!file.isDirectory()) {
            String filename = String(file.name());
            // Only include MP3 files
            if (filename.endsWith(".mp3") || filename.endsWith(".MP3")) {
                files.push_back(filename);
            }
        }
        file = root.openNextFile();
    }
    
    return files;
}

bool Storage::deleteMusicFile(const String& filename) {
    String path = getMusicPath(filename);
    if (SD.remove(path)) {
        Serial.printf("Deleted file: %s\n", path.c_str());
        return true;
    }
    return false;
}

bool Storage::musicFileExists(const String& filename) {
    String path = getMusicPath(filename);
    return SD.exists(path);
}

String Storage::getMusicPath(const String& filename) {
    if (filename.startsWith("/")) {
        return filename;
    }
    return String(MUSIC_DIR) + "/" + filename;
}

bool Storage::loadNFCLinks() {
    _nfcLinks.clear();
    
    if (!SD.exists(NFC_LINKS_FILE)) {
        Serial.println("NFC links file doesn't exist, creating new one");
        return saveNFCLinks();
    }
    
    File file = SD.open(NFC_LINKS_FILE, FILE_READ);
    if (!file) {
        Serial.println("Failed to open NFC links file");
        return false;
    }
    
    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    
    if (error) {
        Serial.printf("Failed to parse NFC links: %s\n", error.c_str());
        return false;
    }
    
    JsonArray links = doc["links"].as<JsonArray>();
    for (JsonObject link : links) {
        NFCLink nfcLink;
        nfcLink.uid = link["uid"].as<String>();
        nfcLink.songPath = link["song"].as<String>();
        _nfcLinks.push_back(nfcLink);
    }
    
    Serial.printf("Loaded %d NFC links\n", _nfcLinks.size());
    return true;
}

bool Storage::saveNFCLinks() {
    DynamicJsonDocument doc(2048);
    JsonArray links = doc.createNestedArray("links");
    
    for (const auto& link : _nfcLinks) {
        JsonObject obj = links.createNestedObject();
        obj["uid"] = link.uid;
        obj["song"] = link.songPath;
    }
    
    File file = SD.open(NFC_LINKS_FILE, FILE_WRITE);
    if (!file) {
        Serial.println("Failed to open NFC links file for writing");
        return false;
    }
    
    if (serializeJson(doc, file) == 0) {
        Serial.println("Failed to write NFC links");
        file.close();
        return false;
    }
    
    file.close();
    Serial.println("NFC links saved successfully");
    return true;
}

bool Storage::linkNFC(const String& uid, const String& songPath) {
    // Remove existing link if present
    unlinkNFC(uid);
    
    NFCLink link;
    link.uid = uid;
    link.songPath = songPath;
    _nfcLinks.push_back(link);
    
    return saveNFCLinks();
}

bool Storage::unlinkNFC(const String& uid) {
    auto it = std::remove_if(_nfcLinks.begin(), _nfcLinks.end(),
        [&uid](const NFCLink& link) { return link.uid == uid; });
    
    if (it != _nfcLinks.end()) {
        _nfcLinks.erase(it, _nfcLinks.end());
        return saveNFCLinks();
    }
    
    return true;
}

String Storage::getSongForNFC(const String& uid) {
    for (const auto& link : _nfcLinks) {
        if (link.uid == uid) {
            return link.songPath;
        }
    }
    return "";
}

std::vector<NFCLink> Storage::getAllLinks() {
    return _nfcLinks;
}
