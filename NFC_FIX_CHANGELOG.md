# 🔧 Correcciones Aplicadas al Sistema NFC

## Problema Identificado

El sistema NFC no estaba detectando correctamente los tags después de la primera detección debido a un error en la lógica de debounce.

## Cambios Realizados

### 1. **Corrección en `nfc_reader.h`**
- **Agregado**: Variable `_lastTagTime` para rastrear correctamente cuándo se detectó un tag por última vez
- **Antes**: Solo se usaba `_lastReadTime` (tiempo de la última lectura del sensor)
- **Ahora**: Se usa `_lastTagTime` (tiempo de la última detección de tag)

### 2. **Corrección en `nfc_reader.cpp`**
- **Inicialización**: Se inicializa `_lastTagTime` en el constructor
- **Lógica de detección**: Se actualiza `_lastTagTime` cuando se detecta un tag
- **Debounce mejorado**: Ahora compara correctamente el tiempo desde la última detección de tag

### 3. **Mejora en `main.cpp`**
- **Lógica más clara**: Separada en variables booleanas para mejor legibilidad
- **Mensajes mejorados**: Ahora muestra símbolos (⚠, ♪, →, ✓, ✗) para mejor visualización
- **Flujo optimizado**: 
  - Mismo tag + tiempo corto + reproduciendo = PAUSA
  - Mismo tag + tiempo corto + pausado = RESUME
  - Tag diferente o tiempo largo = REPRODUCIR NUEVA CANCIÓN

## Comportamiento Correcto Esperado

### **Escenario 1: Primera detección de un tag**
```
--- NFC Tag Detected ---
UID: 042FDDA07A2681
♪ Linked song: cancion1.mp3
→ Action: Playing song
✓ Playback started successfully
------------------------
```

### **Escenario 2: Mismo tag mientras reproduce (dentro de 1.5s)**
```
--- NFC Tag Detected ---
UID: 042FDDA07A2681
♪ Linked song: cancion1.mp3
→ Action: Pausing playback
------------------------
```

### **Escenario 3: Mismo tag después de pausar**
```
--- NFC Tag Detected ---
UID: 042FDDA07A2681
♪ Linked song: cancion1.mp3
→ Action: Resuming playback
------------------------
```

### **Escenario 4: Tag diferente**
```
--- NFC Tag Detected ---
UID: 0A1B2C3D4E5F60
♪ Linked song: cancion2.mp3
→ Action: Playing song
✓ Playback started successfully
------------------------
```

### **Escenario 5: Tag no vinculado**
```
--- NFC Tag Detected ---
UID: AABBCCDDEEFF00
⚠ No song linked to this tag
→ Use the web interface to link a song
------------------------
```

## Cómo Probar

### **Test 1: Detección Básica**
1. Acerca un tag NFC vinculado
2. Deberías ver el mensaje de detección en el serial
3. Si tiene canción vinculada, debería intentar reproducir

### **Test 2: Pausa/Resume**
1. Con música reproduciéndose, acerca el mismo tag
2. Debería pausar
3. Vuelve a acercar el mismo tag
4. Debería resumir

### **Test 3: Cambio de Canción**
1. Con música reproduciéndose
2. Acerca un tag DIFERENTE (con otra canción vinculada)
3. Debería cambiar a la nueva canción

### **Test 4: Sin Vinculación**
1. Acerca un tag que NO esté vinculado
2. Debería mostrar mensaje de advertencia
3. NO debería reproducir nada

## Verificación por Serial Monitor

```bash
~/.platformio/penv/bin/platformio device monitor -e esp32dev
```

Presiona RESET en el ESP32 y deberías ver:
```
=================================
    MusicBox Initializing
=================================

CPU Frequency: 240 MHz

[1/5] Initializing SD Card...
✓ SD Card ready

[2/5] Initializing Audio Player...
✓ Audio Player ready

[3/5] Initializing NFC Reader...
Found chip PN532
Firmware ver. 1.6
NFC Reader initialized
✓ NFC Reader ready

[4/5] Starting WiFi Access Point...
✓ Access Point started
   SSID: MusicBox
   Password: musicbox123
   IP Address: 192.168.4.1
   Web Interface: http://192.168.4.1

[5/5] Starting Web Server...
✓ Web Server ready

=================================
    MusicBox Ready!
=================================

Connect to WiFi: MusicBox
Open browser: http://192.168.4.1

Waiting for NFC tags...
```

## Próximos Pasos

1. **Preparar SD Card**:
   - Formatear como FAT32
   - Crear carpeta `/music/`
   - Copiar archivos MP3

2. **Conectar al WiFi**:
   - Red: MusicBox
   - Contraseña: musicbox123

3. **Acceder a la Web**:
   - Abrir: http://192.168.4.1
   - Subir canciones
   - Vincular tags NFC

4. **Probar**:
   - Acercar tags y verificar comportamiento
   - Revisar mensajes en serial monitor

## Parámetros de Configuración

En `include/config.h`:
```cpp
#define NFC_POLL_INTERVAL 100    // ms entre lecturas (100ms = 10 lecturas/seg)
#define NFC_DEBOUNCE_TIME 1500   // ms para debounce (1.5 segundos)
```

**Ajustar si es necesario**:
- `NFC_POLL_INTERVAL` menor = más responsivo pero más CPU
- `NFC_DEBOUNCE_TIME` menor = más sensible a re-detección rápida
- `NFC_DEBOUNCE_TIME` mayor = menos sensible a tags repetidos

## Notas Técnicas

### Diferencia entre `_lastReadTime` y `_lastTagTime`

- **`_lastReadTime`**: Tiempo de la última lectura del sensor (se actualiza cada 100ms)
- **`_lastTagTime`**: Tiempo de la última detección exitosa de un tag (solo cuando hay tag)

Esta separación es crucial para el debounce correcto.

### Flujo de Detección

```
┌─────────────────────┐
│  NFC Reader Loop    │
│  (cada 100ms)       │
└──────────┬──────────┘
           │
           ▼
    ┌──────────────┐
    │ ¿Tag present?│
    └──────┬───────┘
           │
      ┌────┴────┐
     YES       NO
      │         │
      ▼         ▼
┌─────────┐  ┌──────────┐
│ Compare │  │ Clear    │
│ with    │  │ current  │
│ current │  │ UID      │
└────┬────┘  └──────────┘
     │
     ▼
┌─────────────┐
│ Different?  │
│ OR          │
│ Time > 1.5s?│
└──────┬──────┘
       │
      YES
       │
       ▼
┌──────────────────┐
│ Trigger callback │
│ Update times     │
└──────────────────┘
```

## Troubleshooting

### Si no detecta tags:
- Verificar conexiones (ver NFC_DIAGNOSTIC_GUIDE.md)
- Revisar que el LED del PN532 esté encendido
- Probar con el programa de test: `./test_nfc.sh`

### Si detecta pero no reproduce:
- Verificar que la tarjeta SD esté insertada
- Verificar que exista la carpeta `/music/`
- Verificar que el archivo MP3 exista
- Revisar mensajes de error en serial

### Si pausa/resume no funciona:
- Ajustar `NFC_DEBOUNCE_TIME` en config.h
- Verificar que estás usando el mismo tag
- Verificar timing (debe ser < 1.5s entre detecciones)

---

**Firmware actualizado**: ✅ Subido correctamente
**Estado**: Listo para probar
