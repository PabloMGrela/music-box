#include "web_server.h"
#include "storage.h"
#include "audio_player.h"
#include "nfc_reader.h"
#include "config.h"
#include <ArduinoJson.h>

WebServerManager webServer;

// File upload handling
File uploadFile;

WebServerManager::WebServerManager() : _server(nullptr) {}

bool WebServerManager::begin() {
    _server = new AsyncWebServer(WEB_SERVER_PORT);
    
    setupRoutes();
    
    _server->begin();
    Serial.println("Web server started");
    
    return true;
}

void WebServerManager::loop() {
    // AsyncWebServer handles requests automatically
}

void WebServerManager::setupRoutes() {
    // Serve static HTML page
    _server->on("/", HTTP_GET, [this](AsyncWebServerRequest* request) {
        handleRoot(request);
    });
    
    // API Routes - Songs
    _server->on("/api/songs", HTTP_GET, [this](AsyncWebServerRequest* request) {
        handleListSongs(request);
    });
    
    _server->on("/api/songs/upload", HTTP_POST, 
        [](AsyncWebServerRequest* request) {
            request->send(200, "application/json", "{\"success\":true}");
        },
        [this](AsyncWebServerRequest* request, String filename, size_t index, 
               uint8_t* data, size_t len, bool final) {
            handleUploadSong(request, filename, index, data, len, final);
        }
    );
    
    _server->on("/api/songs/*", HTTP_DELETE, [this](AsyncWebServerRequest* request) {
        handleDeleteSong(request);
    });
    
    // API Routes - NFC Tags
    // IMPORTANT: Register specific routes BEFORE wildcard routes!
    _server->on("/api/tags/scan", HTTP_GET, [this](AsyncWebServerRequest* request) {
        handleScanTag(request);
    });
    
    // Clear last scanned UID (for fresh modal opening)
    _server->on("/api/tags/scan/clear", HTTP_POST, [this](AsyncWebServerRequest* request) {
        nfcReader.clearLastUID();
        Serial.println("[WEB] Cleared last scanned UID");
        request->send(200, "application/json", "{\"success\":true}");
    });
    
    _server->on("/api/tags/link", HTTP_POST, 
        [this](AsyncWebServerRequest* request) {
            // Handler called after body is received
        },
        NULL,
        [this](AsyncWebServerRequest* request, uint8_t *data, size_t len, size_t index, size_t total) {
            // Body handler
            handleLinkTagBody(request, data, len, index, total);
        }
    );
    
    _server->on("/api/tags", HTTP_GET, [this](AsyncWebServerRequest* request) {
        handleListTags(request);
    });
    
    _server->on("/api/tags/*", HTTP_DELETE, [this](AsyncWebServerRequest* request) {
        handleUnlinkTag(request);
    });
    
    // API Routes - Status
    _server->on("/api/status", HTTP_GET, [this](AsyncWebServerRequest* request) {
        handleStatus(request);
    });
    
    // 404 handler
    _server->onNotFound([this](AsyncWebServerRequest* request) {
        handleNotFound(request);
    });
}

void WebServerManager::handleRoot(AsyncWebServerRequest* request) {
    String html = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>MusicBox - Configuración</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body { 
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: #333;
            min-height: 100vh;
            padding: 20px;
        }
        .container {
            max-width: 1200px;
            margin: 0 auto;
            background: white;
            border-radius: 15px;
            box-shadow: 0 10px 40px rgba(0,0,0,0.2);
            overflow: hidden;
        }
        .header {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            padding: 30px;
            text-align: center;
        }
        .header h1 { font-size: 2.5em; margin-bottom: 10px; }
        .header p { opacity: 0.9; }
        .content { padding: 30px; }
        .section {
            margin-bottom: 40px;
            padding: 20px;
            background: #f8f9fa;
            border-radius: 10px;
        }
        .section h2 {
            color: #667eea;
            margin-bottom: 20px;
            padding-bottom: 10px;
            border-bottom: 2px solid #667eea;
        }
        .btn {
            background: #667eea;
            color: white;
            border: none;
            padding: 12px 24px;
            border-radius: 6px;
            cursor: pointer;
            font-size: 16px;
            transition: all 0.3s;
        }
        .btn:hover { background: #5568d3; transform: translateY(-2px); }
        .btn-danger { background: #e74c3c; }
        .btn-danger:hover { background: #c0392b; }
        .file-input {
            border: 2px dashed #667eea;
            padding: 20px;
            border-radius: 8px;
            text-align: center;
            cursor: pointer;
            transition: all 0.3s;
        }
        .file-input:hover { background: #f0f0f0; }
        .list-item {
            background: white;
            padding: 15px;
            margin: 10px 0;
            border-radius: 8px;
            display: flex;
            justify-content: space-between;
            align-items: center;
            box-shadow: 0 2px 5px rgba(0,0,0,0.1);
        }
        .list-item:hover { box-shadow: 0 4px 10px rgba(0,0,0,0.15); }
        .status {
            position: fixed;
            top: 20px;
            right: 20px;
            background: white;
            padding: 15px 20px;
            border-radius: 8px;
            box-shadow: 0 4px 10px rgba(0,0,0,0.2);
            max-width: 300px;
        }
        .status-playing { color: #27ae60; font-weight: bold; }
        .status-paused { color: #f39c12; font-weight: bold; }
        .modal {
            display: none;
            position: fixed;
            z-index: 1000;
            left: 0;
            top: 0;
            width: 100%;
            height: 100%;
            background: rgba(0,0,0,0.5);
        }
        .modal-content {
            background: white;
            margin: 10% auto;
            padding: 30px;
            border-radius: 10px;
            max-width: 500px;
            box-shadow: 0 10px 40px rgba(0,0,0,0.3);
        }
        .close {
            float: right;
            font-size: 28px;
            font-weight: bold;
            cursor: pointer;
        }
        .close:hover { color: #e74c3c; }
        input[type="text"], select {
            width: 100%;
            padding: 12px;
            margin: 10px 0;
            border: 2px solid #ddd;
            border-radius: 6px;
            font-size: 16px;
        }
        input[type="text"]:focus, select:focus {
            outline: none;
            border-color: #667eea;
        }
        .progress {
            width: 100%;
            height: 30px;
            background: #f0f0f0;
            border-radius: 15px;
            overflow: hidden;
            margin: 10px 0;
        }
        .progress-bar {
            height: 100%;
            background: linear-gradient(90deg, #667eea, #764ba2);
            transition: width 0.3s;
            display: flex;
            align-items: center;
            justify-content: center;
            color: white;
            font-weight: bold;
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>🎵 MusicBox</h1>
            <p>Configuración del Reproductor NFC</p>
        </div>
        
        <div class="status" id="status">
            <div>Estado: <span id="playerState">Detenido</span></div>
            <div id="currentSong"></div>
        </div>
        
        <div class="content">
            <!-- Songs Section -->
            <div class="section">
                <h2>📀 Gestión de Canciones</h2>
                <div class="file-input" onclick="document.getElementById('fileInput').click()">
                    <p>Haz clic para subir una canción MP3</p>
                    <input type="file" id="fileInput" accept=".mp3" style="display:none">
                </div>
                <div id="uploadProgress" style="display:none;">
                    <div class="progress">
                        <div class="progress-bar" id="progressBar">0%</div>
                    </div>
                </div>
                <div id="songsList"></div>
            </div>
            
            <!-- NFC Tags Section -->
            <div class="section">
                <h2>🏷️ Gestión de Tags NFC</h2>
                <button class="btn" onclick="openLinkModal()">Vincular Nuevo Tag</button>
                <div id="tagsList"></div>
            </div>
        </div>
    </div>
    
    <!-- Link Tag Modal -->
    <div id="linkModal" class="modal">
        <div class="modal-content">
            <span class="close" onclick="closeLinkModal()">&times;</span>
            <h2>Vincular Tag NFC</h2>
            <p id="scanStatus" style="color: #667eea; font-weight: bold;">🔍 Escaneando... Acerca el tag NFC al lector</p>
            <input type="text" id="tagUid" placeholder="UID del Tag" readonly style="font-family: monospace; font-size: 14px;">
            <select id="songSelect">
                <option value="">Selecciona una canción</option>
            </select>
            <button class="btn" onclick="linkTag()" style="margin-top: 15px;">Vincular</button>
        </div>
    </div>
    
    <script>
        let scanInterval = null;
        
        // Load initial data
        document.addEventListener('DOMContentLoaded', function() {
            loadSongs();
            loadTags();
            updateStatus();
            setInterval(updateStatus, 2000);
        });
        
        // File upload
        document.getElementById('fileInput').addEventListener('change', function(e) {
            const file = e.target.files[0];
            if (!file) return;
            
            const formData = new FormData();
            formData.append('file', file);
            
            const xhr = new XMLHttpRequest();
            
            xhr.upload.addEventListener('progress', function(e) {
                if (e.lengthComputable) {
                    const percent = (e.loaded / e.total) * 100;
                    document.getElementById('uploadProgress').style.display = 'block';
                    document.getElementById('progressBar').style.width = percent + '%';
                    document.getElementById('progressBar').textContent = Math.round(percent) + '%';
                }
            });
            
            xhr.addEventListener('load', function() {
                if (xhr.status === 200) {
                    alert('Canción subida correctamente');
                    loadSongs();
                } else {
                    alert('Error al subir la canción');
                }
                document.getElementById('uploadProgress').style.display = 'none';
                document.getElementById('fileInput').value = '';
            });
            
            xhr.open('POST', '/api/songs/upload');
            xhr.send(formData);
        });
        
        function loadSongs() {
            fetch('/api/songs')
                .then(r => r.json())
                .then(data => {
                    const list = document.getElementById('songsList');
                    list.innerHTML = '';
                    data.songs.forEach(song => {
                        const item = document.createElement('div');
                        item.className = 'list-item';
                        item.innerHTML = `
                            <span>🎵 ${song}</span>
                            <button class="btn btn-danger" onclick="deleteSong('${song}')">Eliminar</button>
                        `;
                        list.appendChild(item);
                    });
                    
                    // Update song select in modal
                    const select = document.getElementById('songSelect');
                    select.innerHTML = '<option value="">Selecciona una canción</option>';
                    data.songs.forEach(song => {
                        const option = document.createElement('option');
                        option.value = song;
                        option.textContent = song;
                        select.appendChild(option);
                    });
                });
        }
        
        function deleteSong(filename) {
            if (!confirm('¿Eliminar ' + filename + '?')) return;
            fetch('/api/songs/' + encodeURIComponent(filename), { method: 'DELETE' })
                .then(r => r.json())
                .then(() => loadSongs());
        }
        
        function loadTags() {
            fetch('/api/tags')
                .then(r => r.json())
                .then(data => {
                    const list = document.getElementById('tagsList');
                    list.innerHTML = '';
                    data.tags.forEach(tag => {
                        const item = document.createElement('div');
                        item.className = 'list-item';
                        item.innerHTML = `
                            <span>🏷️ ${tag.uid} → ${tag.song}</span>
                            <button class="btn btn-danger" onclick="unlinkTag('${tag.uid}')">Desvincular</button>
                        `;
                        list.appendChild(item);
                    });
                });
        }
        
        function openLinkModal() {
            document.getElementById('linkModal').style.display = 'block';
            document.getElementById('tagUid').value = '';
            document.getElementById('scanStatus').textContent = '🔍 Escaneando... Acerca el tag NFC al lector';
            document.getElementById('scanStatus').style.color = '#667eea';
            
            // Clear last scanned UID on server
            fetch('/api/tags/scan/clear', { method: 'POST' })
                .then(() => console.log('Cleared last scanned UID'))
                .catch(err => console.error('Error clearing UID:', err));
            
            startScanning();
        }
        
        function closeLinkModal() {
            document.getElementById('linkModal').style.display = 'none';
            stopScanning();
        }
        
        function startScanning() {
            console.log('Started NFC scanning...');
            scanInterval = setInterval(() => {
                fetch('/api/tags/scan')
                    .then(r => r.json())
                    .then(data => {
                        console.log('Scan response:', data);
                        if (data.uid && data.uid !== null) {
                            document.getElementById('tagUid').value = data.uid;
                            document.getElementById('scanStatus').textContent = '✅ Tag detectado: ' + data.uid;
                            document.getElementById('scanStatus').style.color = '#27ae60';
                            console.log('Tag detected:', data.uid);
                        }
                    })
                    .catch(err => {
                        console.error('Scan error:', err);
                        document.getElementById('scanStatus').textContent = '⚠️ Error al escanear';
                        document.getElementById('scanStatus').style.color = '#e74c3c';
                    });
            }, 500);
        }
        
        function stopScanning() {
            if (scanInterval) {
                clearInterval(scanInterval);
                scanInterval = null;
            }
        }
        
        function linkTag() {
            const uid = document.getElementById('tagUid').value;
            const song = document.getElementById('songSelect').value;
            
            if (!uid || !song) {
                alert('Debes escanear un tag y seleccionar una canción');
                return;
            }
            
            fetch('/api/tags/link', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ uid, song })
            })
            .then(r => r.json())
            .then(() => {
                alert('Tag vinculado correctamente');
                closeLinkModal();
                loadTags();
            });
        }
        
        function unlinkTag(uid) {
            if (!confirm('¿Desvincular tag?')) return;
            fetch('/api/tags/' + encodeURIComponent(uid), { method: 'DELETE' })
                .then(r => r.json())
                .then(() => loadTags());
        }
        
        function updateStatus() {
            fetch('/api/status')
                .then(r => r.json())
                .then(data => {
                    const stateEl = document.getElementById('playerState');
                    const songEl = document.getElementById('currentSong');
                    
                    stateEl.textContent = data.state;
                    stateEl.className = data.state === 'playing' ? 'status-playing' : 
                                       data.state === 'paused' ? 'status-paused' : '';
                    
                    if (data.currentSong) {
                        songEl.textContent = '♪ ' + data.currentSong;
                        songEl.style.display = 'block';
                    } else {
                        songEl.style.display = 'none';
                    }
                });
        }
    </script>
</body>
</html>
)rawliteral";
    
    request->send(200, "text/html", html);
}

void WebServerManager::handleListSongs(AsyncWebServerRequest* request) {
    std::vector<String> songs = storage.listMusicFiles();
    
    DynamicJsonDocument doc(2048);
    JsonArray songsArray = doc.createNestedArray("songs");
    
    for (const auto& song : songs) {
        songsArray.add(song);
    }
    
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
}

void WebServerManager::handleDeleteSong(AsyncWebServerRequest* request) {
    String path = request->url();
    String filename = path.substring(path.lastIndexOf('/') + 1);
    
    bool success = storage.deleteMusicFile(filename);
    
    DynamicJsonDocument doc(128);
    doc["success"] = success;
    
    String response;
    serializeJson(doc, response);
    request->send(success ? 200 : 404, "application/json", response);
}

void WebServerManager::handleUploadSong(AsyncWebServerRequest* request, String filename, 
                                       size_t index, uint8_t* data, size_t len, bool final) {
    if (!index) {
        Serial.printf("Upload Start: %s\n", filename.c_str());
        String filepath = storage.getMusicPath(filename);
        uploadFile = SD.open(filepath, FILE_WRITE);
        if (!uploadFile) {
            Serial.println("Failed to open file for writing");
            return;
        }
    }
    
    if (uploadFile) {
        uploadFile.write(data, len);
    }
    
    if (final) {
        if (uploadFile) {
            uploadFile.close();
        }
        Serial.printf("Upload Complete: %s (%d bytes)\n", filename.c_str(), index + len);
    }
}

void WebServerManager::handleListTags(AsyncWebServerRequest* request) {
    std::vector<NFCLink> links = storage.getAllLinks();
    
    DynamicJsonDocument doc(2048);
    JsonArray tagsArray = doc.createNestedArray("tags");
    
    for (const auto& link : links) {
        JsonObject obj = tagsArray.createNestedObject();
        obj["uid"] = link.uid;
        obj["song"] = link.songPath;
    }
    
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
}

void WebServerManager::handleLinkTagBody(AsyncWebServerRequest* request, uint8_t *data, size_t len, size_t index, size_t total) {
    // Accumulate body data
    if (index == 0) {
        _linkRequestBody = "";
    }
    
    for (size_t i = 0; i < len; i++) {
        _linkRequestBody += (char)data[i];
    }
    
    // When all data received, process the request
    if (index + len == total) {
        Serial.printf("[WEB] Link request body: %s\n", _linkRequestBody.c_str());
        
        DynamicJsonDocument doc(256);
        DeserializationError error = deserializeJson(doc, _linkRequestBody);
        
        if (error) {
            Serial.printf("[WEB] Failed to parse JSON: %s\n", error.c_str());
            request->send(400, "application/json", "{\"success\":false,\"error\":\"Invalid JSON\"}");
            return;
        }
        
        String uid = doc["uid"].as<String>();
        String song = doc["song"].as<String>();
        
        Serial.printf("[WEB] Linking UID '%s' to song '%s'\n", uid.c_str(), song.c_str());
        
        bool success = storage.linkNFC(uid, song);
        
        Serial.printf("[WEB] Link result: %s\n", success ? "SUCCESS" : "FAILED");
        
        DynamicJsonDocument responseDoc(128);
        responseDoc["success"] = success;
        
        String response;
        serializeJson(responseDoc, response);
        request->send(success ? 200 : 500, "application/json", response);
    }
}

void WebServerManager::handleLinkTag(AsyncWebServerRequest* request) {
    // This is now handled by handleLinkTagBody
    // Just send a response if called directly (shouldn't happen)
    request->send(400, "application/json", "{\"success\":false,\"error\":\"No body received\"}");
}

void WebServerManager::handleUnlinkTag(AsyncWebServerRequest* request) {
    String path = request->url();
    String uid = path.substring(path.lastIndexOf('/') + 1);
    
    bool success = storage.unlinkNFC(uid);
    
    DynamicJsonDocument doc(128);
    doc["success"] = success;
    
    String response;
    serializeJson(doc, response);
    request->send(success ? 200 : 404, "application/json", response);
}

void WebServerManager::handleScanTag(AsyncWebServerRequest* request) {
    DynamicJsonDocument doc(256);
    
    // Return the last detected UID (if any)
    String lastUID = nfcReader.getLastUID();
    
    Serial.printf("[WEB] /api/tags/scan called - Last UID: '%s' (length: %d)\n", 
                  lastUID.c_str(), lastUID.length());
    
    if (lastUID != "" && lastUID.length() > 0) {
        doc["uid"] = lastUID;
        doc["detected"] = true;
        Serial.printf("[WEB] → Returning scanned UID: %s\n", lastUID.c_str());
    } else {
        doc["uid"] = nullptr;
        doc["detected"] = false;
        Serial.println("[WEB] → No UID available");
    }
    
    String response;
    serializeJson(doc, response);
    Serial.printf("[WEB] → Response: %s\n", response.c_str());
    request->send(200, "application/json", response);
}

void WebServerManager::handleStatus(AsyncWebServerRequest* request) {
    DynamicJsonDocument doc(256);
    
    String state = "stopped";
    if (audioPlayer.isPlaying()) state = "playing";
    else if (audioPlayer.isPaused()) state = "paused";
    
    doc["state"] = state;
    doc["currentSong"] = audioPlayer.getCurrentSong();
    doc["volume"] = audioPlayer.getVolume();
    
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
}

void WebServerManager::handleNotFound(AsyncWebServerRequest* request) {
    request->send(404, "application/json", "{\"error\":\"Not found\"}");
}
