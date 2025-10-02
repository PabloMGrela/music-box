#include <Arduino.h>
#include <WiFi.h>
#include "config.h"
#include "storage.h"
#include "nfc_reader.h"
#include "audio_player.h"
#include "web_server.h"

// Last tag seen for debouncing
String lastTagUID = "";
unsigned long lastTagTime = 0;

void onTagDetected(String uid);

void setup() {
    // Initialize serial
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n\n=================================");
    Serial.println("    MusicBox Initializing");
    Serial.println("=================================\n");
    
    // Set CPU frequency to 240MHz for better audio performance
    setCpuFrequencyMhz(240);
    Serial.printf("CPU Frequency: %d MHz\n", getCpuFrequencyMhz());
    
    // Initialize SD card storage
    Serial.println("\n[1/5] Initializing SD Card...");
    if (!storage.begin()) {
        Serial.println("⚠ WARNING: SD Card initialization failed!");
        Serial.println("⚠ System will continue without music playback capability");
        Serial.println("⚠ Please check:");
        Serial.println("  - SD card is inserted correctly");
        Serial.println("  - SD card is formatted as FAT32");
        Serial.println("  - Wiring: CS=GPIO13, SCK=GPIO18, MISO=GPIO19, MOSI=GPIO23");
    } else {
        Serial.println("✓ SD Card ready");
    }
    
    // Initialize audio player
    Serial.println("\n[2/5] Initializing Audio Player...");
    if (!audioPlayer.begin()) {
        Serial.println("ERROR: Audio Player initialization failed!");
    } else {
        Serial.println("✓ Audio Player ready");
    }
    
    // Initialize NFC reader
    Serial.println("\n[3/5] Initializing NFC Reader...");
    if (!nfcReader.begin()) {
        Serial.println("WARNING: NFC Reader initialization failed!");
        Serial.println("Check wiring and connections.");
    } else {
        Serial.println("✓ NFC Reader ready");
        nfcReader.setOnTagDetected(onTagDetected);
    }
    
    // Initialize WiFi Access Point
    Serial.println("\n[4/5] Starting WiFi Access Point...");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(WIFI_SSID, WIFI_PASSWORD, WIFI_CHANNEL, 0, MAX_CONNECTIONS);
    
    IPAddress IP = WiFi.softAPIP();
    Serial.println("✓ Access Point started");
    Serial.printf("   SSID: %s\n", WIFI_SSID);
    Serial.printf("   Password: %s\n", WIFI_PASSWORD);
    Serial.printf("   IP Address: %s\n", IP.toString().c_str());
    Serial.println("   Web Interface: http://" + IP.toString());
    
    // Initialize web server
    Serial.println("\n[5/5] Starting Web Server...");
    if (!webServer.begin()) {
        Serial.println("ERROR: Web Server initialization failed!");
    } else {
        Serial.println("✓ Web Server ready");
    }
    
    Serial.println("\n=================================");
    Serial.println("    MusicBox Ready!");
    Serial.println("=================================\n");
    Serial.println("Connect to WiFi: " + String(WIFI_SSID));
    Serial.println("Open browser: http://" + IP.toString());
    Serial.println("\nWaiting for NFC tags...\n");
}

void loop() {
    // Update NFC reader
    nfcReader.loop();
    
    // Update audio player
    audioPlayer.loop();
    
    // Web server is handled by async callbacks
    webServer.loop();
    
    // Small delay to prevent watchdog issues
    delay(10);
}

void onTagDetected(String uid) {
    unsigned long currentTime = millis();
    
    Serial.println("\n--- NFC Tag Detected ---");
    Serial.printf("UID: %s\n", uid.c_str());
    
    // Check if we have a song linked to this tag
    String linkedSong = storage.getSongForNFC(uid);
    
    if (linkedSong.isEmpty()) {
        Serial.println("⚠ No song linked to this tag");
        Serial.println("→ Use the web interface to link a song");
        Serial.println("------------------------\n");
        return;
    }
    
    Serial.printf("♪ Linked song: %s\n", linkedSong.c_str());
    
    // Behavior logic:
    // 1. If same tag while playing -> pause/resume
    // 2. If different tag or stopped -> play song
    
    bool isSameTag = (uid == lastTagUID);
    bool withinDebounce = (currentTime - lastTagTime) < NFC_DEBOUNCE_TIME;
    
    if (isSameTag && withinDebounce && audioPlayer.isPlaying()) {
        // Same tag, recently detected, and playing -> PAUSE
        Serial.println("→ Action: Pausing playback");
        audioPlayer.pause();
    } else if (isSameTag && withinDebounce && audioPlayer.isPaused()) {
        // Same tag, recently detected, and paused -> RESUME
        Serial.println("→ Action: Resuming playback");
        audioPlayer.resume();
    } else {
        // Different tag OR enough time passed OR stopped -> PLAY NEW SONG
        String fullPath = storage.getMusicPath(linkedSong);
        
        if (!storage.musicFileExists(linkedSong)) {
            Serial.println("✗ ERROR: Song file not found!");
            Serial.println("------------------------\n");
            return;
        }
        
        Serial.println("→ Action: Playing song");
        if (audioPlayer.play(fullPath)) {
            Serial.println("✓ Playback started successfully");
        } else {
            Serial.println("✗ ERROR: Failed to start playback");
        }
    }
    
    lastTagUID = uid;
    lastTagTime = currentTime;
    Serial.println("------------------------\n");
}
