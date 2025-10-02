#ifndef AUDIO_PLAYER_H
#define AUDIO_PLAYER_H

#include <Arduino.h>
// Forward declarations to avoid loading libraries globally
class AudioFileSourceSD;
class AudioFileSourceBuffer;
class AudioFileSourceID3;
class AudioGeneratorMP3;
class AudioOutputI2S;

enum PlayerState {
    STOPPED,
    PLAYING,
    PAUSED
};

class AudioPlayer {
public:
    AudioPlayer();
    ~AudioPlayer();
    
    bool begin();
    void loop();
    
    // Playback control
    bool play(const String& filepath);
    void pause();
    void resume();
    void stop();
    
    // State queries
    PlayerState getState() { return _state; }
    bool isPlaying() { return _state == PLAYING; }
    bool isPaused() { return _state == PAUSED; }
    String getCurrentSong() { return _currentSong; }
    
    // Volume control
    void setVolume(float volume);  // 0.0 to 1.0
    float getVolume() { return _volume; }
    
private:
    AudioGeneratorMP3* _mp3;
    AudioFileSourceSD* _file;
    AudioFileSourceBuffer* _buff;
    AudioFileSourceID3* _id3;
    AudioOutputI2S* _out;
    
    PlayerState _state;
    String _currentSong;
    float _volume;
    
    void cleanup();
};

extern AudioPlayer audioPlayer;

#endif // AUDIO_PLAYER_H
