#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

class WebServerManager {
public:
    WebServerManager();
    
    bool begin();
    void loop();
    
private:
    AsyncWebServer* _server;
    String _linkRequestBody;  // Buffer for POST body
    
    // Route handlers
    void setupRoutes();
    
    // API endpoints - Songs
    void handleListSongs(AsyncWebServerRequest* request);
    void handleDeleteSong(AsyncWebServerRequest* request);
    void handleUploadSong(AsyncWebServerRequest* request, String filename, 
                         size_t index, uint8_t* data, size_t len, bool final);
    
    // API endpoints - NFC Tags
    void handleListTags(AsyncWebServerRequest* request);
    void handleLinkTag(AsyncWebServerRequest* request);
    void handleLinkTagBody(AsyncWebServerRequest* request, uint8_t *data, size_t len, size_t index, size_t total);
    void handleUnlinkTag(AsyncWebServerRequest* request);
    void handleScanTag(AsyncWebServerRequest* request);
    
    // API endpoints - Status
    void handleStatus(AsyncWebServerRequest* request);
    
    // Static files
    void handleRoot(AsyncWebServerRequest* request);
    void handleNotFound(AsyncWebServerRequest* request);
};

extern WebServerManager webServer;

#endif // WEB_SERVER_H
