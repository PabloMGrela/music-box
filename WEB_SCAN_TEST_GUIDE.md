# 🔍 Guía Rápida: Probar Escaneo de Tags NFC desde la Web

## ✅ Mejoras Aplicadas

### 1. **Endpoint `/api/tags/scan` Mejorado**
- Ahora devuelve el último UID detectado sin importar si es "nuevo"
- Incluye flag `detected` para saber si hay un UID disponible
- Añadido logging en serial para debug: `[WEB] Returning scanned UID: XXXXXXXX`

### 2. **Interfaz Web Mejorada**
- **Indicador visual de estado** en el modal
- **Console logging** para debugging en el navegador
- **Feedback en tiempo real**: 
  - 🔍 Escaneando...
  - ✅ Tag detectado: [UID]
  - ⚠️ Error al escanear

### 3. **Mejor Persistencia del UID**
- El último UID se mantiene incluso después de retirar el tag
- Método `clearLastUID()` para limpiar manualmente si es necesario

---

## 🧪 Cómo Probar (Paso a Paso)

### **Paso 1: Verificar que el ESP32 esté funcionando**

Abre el serial monitor:
```bash
~/.platformio/penv/bin/platformio device monitor -e esp32dev
```

Presiona RESET y deberías ver:
```
=================================
    MusicBox Ready!
=================================
Connect to WiFi: MusicBox
Open browser: http://192.168.4.1
```

### **Paso 2: Detectar un Tag Primero (Importante)**

**Antes de abrir la web**, acerca un tag NFC al lector. Deberías ver en el serial:
```
NFC Tag detected: 042FDDA07A2681
⚠ No song linked to this tag
→ Use the web interface to link a song
```

Esto confirma que:
- ✅ El PN532 funciona
- ✅ El tag fue detectado
- ✅ El UID está guardado en memoria

### **Paso 3: Conectar a la Web**

1. **Conectar WiFi**: 
   - Red: `MusicBox`
   - Password: `musicbox123`

2. **Abrir navegador**: `http://192.168.4.1`

3. **Abrir consola del navegador**:
   - Chrome/Edge: `F12` o `Cmd+Option+I`
   - Safari: `Cmd+Option+C`
   - Firefox: `F12` o `Cmd+Option+K`

### **Paso 4: Vincular el Tag**

1. Click en **"Vincular Nuevo Tag"**

2. **Observa la consola del navegador**, deberías ver:
```javascript
Started NFC scanning...
Scan response: {uid: "042FDDA07A2681", detected: true}
Tag detected: 042FDDA07A2681
```

3. **Observa el serial del ESP32**, deberías ver:
```
[WEB] Returning scanned UID: 042FDDA07A2681
```

4. **En la interfaz web** deberías ver:
   - El campo de texto con el UID: `042FDDA07A2681`
   - El mensaje: `✅ Tag detectado: 042FDDA07A2681`

---

## 🐛 Troubleshooting

### **Problema 1: No aparece el UID en el campo**

**Revisar en la consola del navegador:**

```javascript
// Si ves esto:
Scan response: {uid: null, detected: false}
```

**Solución**: El tag no ha sido detectado aún.
- Acerca el tag al lector NFC
- Espera a ver el mensaje en el serial: `NFC Tag detected: ...`
- El escaneo web debería capturarlo en el siguiente poll (0.5s)

---

### **Problema 2: El UID aparece vacío después de retirar el tag**

**Esto NO debería pasar** con la nueva versión.

**Verificar en serial**:
```
NFC Tag detected: 042FDDA07A2681    ← UID guardado
NFC Tag removed                      ← Tag retirado
[WEB] Returning scanned UID: 042FDDA07A2681  ← UID sigue disponible
```

Si el UID desaparece, hay un problema con la persistencia.

---

### **Problema 3: Error 404 en `/api/tags/scan`**

**En la consola del navegador**:
```javascript
GET http://192.168.4.1/api/tags/scan 404 (Not Found)
```

**Solución**: El servidor web no está funcionando correctamente.
- Verificar en serial que diga: `✓ Web Server ready`
- Reiniciar el ESP32

---

### **Problema 4: El popup no se abre**

**Verificar en consola del navegador**:
```javascript
Uncaught ReferenceError: openLinkModal is not defined
```

**Solución**: Problema con el JavaScript. Refrescar la página (`Cmd+R` o `F5`).

---

## 📊 Flujo Completo Esperado

### **Escenario de Éxito:**

```
┌─────────────────────────────────────────────────────────────┐
│ 1. Usuario acerca tag NFC al lector                        │
└──────────────────┬──────────────────────────────────────────┘
                   │
                   ▼
┌─────────────────────────────────────────────────────────────┐
│ 2. ESP32 detecta tag                                        │
│    Serial: "NFC Tag detected: 042FDDA07A2681"              │
│    Memoria: _lastUID = "042FDDA07A2681"                    │
└──────────────────┬──────────────────────────────────────────┘
                   │
                   ▼
┌─────────────────────────────────────────────────────────────┐
│ 3. Usuario abre web y click en "Vincular Nuevo Tag"        │
└──────────────────┬──────────────────────────────────────────┘
                   │
                   ▼
┌─────────────────────────────────────────────────────────────┐
│ 4. JavaScript inicia polling cada 500ms                     │
│    Fetch: GET /api/tags/scan                                │
└──────────────────┬──────────────────────────────────────────┘
                   │
                   ▼
┌─────────────────────────────────────────────────────────────┐
│ 5. ESP32 responde con último UID                            │
│    Response: {uid: "042FDDA07A2681", detected: true}        │
│    Serial: "[WEB] Returning scanned UID: 042FDDA07A2681"   │
└──────────────────┬──────────────────────────────────────────┘
                   │
                   ▼
┌─────────────────────────────────────────────────────────────┐
│ 6. JavaScript muestra UID en el campo                       │
│    Campo: "042FDDA07A2681"                                  │
│    Estado: "✅ Tag detectado: 042FDDA07A2681"              │
└──────────────────┬──────────────────────────────────────────┘
                   │
                   ▼
┌─────────────────────────────────────────────────────────────┐
│ 7. Usuario selecciona canción y hace click en "Vincular"   │
└──────────────────┬──────────────────────────────────────────┘
                   │
                   ▼
┌─────────────────────────────────────────────────────────────┐
│ 8. POST /api/tags/link                                      │
│    Body: {uid: "042FDDA07A2681", song: "cancion.mp3"}      │
└──────────────────┬──────────────────────────────────────────┘
                   │
                   ▼
┌─────────────────────────────────────────────────────────────┐
│ 9. ESP32 guarda vinculación en /nfc_links.json             │
│    ✓ Tag vinculado correctamente                           │
└─────────────────────────────────────────────────────────────┘
```

---

## 🎯 Puntos Críticos de Verificación

### **✅ Checkpoint 1: PN532 funciona**
```bash
# En serial monitor, acerca tag:
NFC Tag detected: XXXXXXXX
```

### **✅ Checkpoint 2: Web Server responde**
```bash
# En navegador, abrir: http://192.168.4.1
# Debería cargar la página
```

### **✅ Checkpoint 3: Endpoint `/api/tags/scan` funciona**
```bash
# En navegador, ir directamente a:
http://192.168.4.1/api/tags/scan

# Debería mostrar JSON:
{"uid":"042FDDA07A2681","detected":true}
```

### **✅ Checkpoint 4: JavaScript captura el UID**
```javascript
// En consola del navegador (F12):
Scan response: {uid: "042FDDA07A2681", detected: true}
Tag detected: 042FDDA07A2681
```

### **✅ Checkpoint 5: UI se actualiza**
- Campo de texto muestra el UID
- Mensaje de estado: ✅ Tag detectado

---

## 🔧 Debugging Avanzado

### **Ver todas las requests en el navegador:**

En la consola (F12), pestaña **Network**:
- Filtrar por `scan`
- Deberías ver requests cada 500ms
- Click en cualquiera para ver la respuesta

### **Ver logs del ESP32 en tiempo real:**

```bash
# Terminal 1: Monitor serial
~/.platformio/penv/bin/platformio device monitor -e esp32dev

# Mantén abierto mientras usas la web
# Verás cada request que llegue:
[WEB] Returning scanned UID: 042FDDA07A2681
```

### **Test manual del endpoint:**

```bash
# Desde terminal (con el ESP32 conectado a WiFi MusicBox):
curl http://192.168.4.1/api/tags/scan

# Debería devolver:
{"uid":"042FDDA07A2681","detected":true}
```

---

## 📝 Notas Importantes

1. **El UID se mantiene** incluso después de retirar el tag
2. **El polling es cada 500ms** (2 requests por segundo)
3. **El último tag detectado** es el que aparecerá, no solo tags "nuevos"
4. **Los mensajes de consola** son cruciales para debugging

---

## 🆘 Si Nada Funciona

### **Reset completo:**

1. **Cerrar el navegador** completamente
2. **Presionar RESET** en el ESP32
3. **Esperar 10 segundos**
4. **Acercar un tag** al lector (verificar en serial)
5. **Reconectar WiFi** MusicBox
6. **Abrir navegador** con consola (F12)
7. **Ir a** http://192.168.4.1
8. **Click en** "Vincular Nuevo Tag"
9. **Observar consola** y serial monitor

Si después de esto no funciona, dame el output completo de:
- Serial monitor
- Consola del navegador
- Pestaña Network del navegador

---

**Firmware actualizado**: ✅ Subido correctamente
**Mejoras activas**: ✅ Escaneo mejorado, mejor debugging, feedback visual
