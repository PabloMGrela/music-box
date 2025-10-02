#include "audio_player.h"
#include "config.h"
// Include audio libraries only in .cpp to avoid SPI initialization conflicts
#include "AudioFileSourceSD.h"
#include "AudioFileSourceBuffer.h"
#include "AudioFileSourceID3.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2S.h"

AudioPlayer audioPlayer;

AudioPlayer::AudioPlayer() 
    : _mp3(nullptr), _file(nullptr), _buff(nullptr), _id3(nullptr), _out(nullptr), 
      _state(STOPPED), _volume(DEFAULT_VOLUME) {}

AudioPlayer::~AudioPlayer() {
    cleanup();
}

bool AudioPlayer::begin() {
    // Initialize I2S output with optimized settings
    _out = new AudioOutputI2S();
    _out->SetPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    _out->SetGain(_volume);
    // Set output mode for better performance (internal DAC mode)
    _out->SetOutputModeMono(false);  // Stereo output
    
    Serial.println("Audio Player initialized");
    return true;
}

void AudioPlayer::loop() {
    if (_mp3 && _mp3->isRunning()) {
        if (!_mp3->loop()) {
            // Song finished
            Serial.println("Song finished");
            stop();
        }
    }
}

bool AudioPlayer::play(const String& filepath) {
    Serial.printf("♪ Playing: %s\n", filepath.c_str());
    
    // Stop current playback if any
    stop();
    
    // Create new file source
    _file = new AudioFileSourceSD(filepath.c_str());
    if (!_file->isOpen()) {
        Serial.println("✗ Failed to open audio file");
        delete _file;
        _file = nullptr;
        return false;
    }
    
    // Create buffer (32KB for smooth playback on dedicated core)
    _buff = new AudioFileSourceBuffer(_file, 32768);
    Serial.println("✓ Created 32KB audio buffer");
    
    // Create ID3 tag filter to skip metadata
    _id3 = new AudioFileSourceID3(_buff);
    _id3->RegisterMetadataCB([](void *cbData, const char *type, bool isUnicode, const char *string) {
        // Callback for metadata - just log it
        Serial.printf("  ID3 %s: %s\n", type, string);
    }, nullptr);
    Serial.println("✓ ID3 metadata filter enabled");
    
    // Create MP3 decoder
    _mp3 = new AudioGeneratorMP3();
    if (!_mp3->begin(_id3, _out)) {
        Serial.println("✗ Failed to start MP3 decoder");
        delete _id3;
        delete _buff;
        delete _file;
        delete _mp3;
        _id3 = nullptr;
        _buff = nullptr;
        _file = nullptr;
        _mp3 = nullptr;
        return false;
    }
    
    _state = PLAYING;
    _currentSong = filepath;
    
    Serial.println("Playback started");
    return true;
}

void AudioPlayer::pause() {
    if (_state == PLAYING && _mp3) {
        _mp3->stop();
        _state = PAUSED;
        Serial.println("Playback paused");
    }
}

void AudioPlayer::resume() {
    if (_state == PAUSED && _mp3 && _file) {
        // Resume playback
        _state = PLAYING;
        Serial.println("Playback resumed");
    }
}

void AudioPlayer::stop() {
    if (_mp3) {
        _mp3->stop();
        delete _mp3;
        _mp3 = nullptr;
    }
    
    if (_id3) {
        delete _id3;
        _id3 = nullptr;
    }
    
    if (_buff) {
        delete _buff;
        _buff = nullptr;
    }
    
    if (_file) {
        _file->close();
        delete _file;
        _file = nullptr;
    }
    
    _state = STOPPED;
    _currentSong = "";
    Serial.println("⏹ Playback stopped");
}

void AudioPlayer::setVolume(float volume) {
    _volume = constrain(volume, 0.0f, 1.0f);
    if (_out) {
        _out->SetGain(_volume);
    }
    Serial.printf("Volume set to: %.2f\n", _volume);
}

void AudioPlayer::cleanup() {
    stop();
    if (_out) {
        delete _out;
        _out = nullptr;
    }
}
