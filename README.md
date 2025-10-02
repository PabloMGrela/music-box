# MusicBox - Reproductor de Música NFC para Niños

Sistema de reproducción de música basado en ESP32 que utiliza tags NFC para controlar la reproducción. Diseñado para niños, sin pantallas.

## 🎯 Características

- **Reproducción por NFC**: Acerca un tag NFC para reproducir la canción asociada
- **Control Simple**: 
  - Acercar tag → reproduce canción
  - Acercar mismo tag mientras suena → pausa/resume
  - Acercar tag diferente → cambia a nueva canción
- **Interfaz Web**: Configura canciones y tags desde cualquier dispositivo
- **Sin Pantallas**: Diseñado para uso infantil sin necesidad de interacción visual

## 🔧 Hardware Requerido

### Componentes Principales
- **ESP32 DevKit** (WROOM)
- **MAX98357A** (Amplificador I2S)
- **PN532** (Lector NFC)
- **MicroSD Card Module** (almacenamiento de música)
- **Altavoz** (conectado al MAX98357A)
- **Tags NFC** (Mifare Classic, NTAG, etc.)

### Conexiones

#### SD Card (VSPI)
| Señal | Pin ESP32 |
|-------|-----------|
| CS    | GPIO 13   |
| SCK   | GPIO 18   |
| MISO  | GPIO 19   |
| MOSI  | GPIO 23   |

#### Audio I2S (MAX98357A)
| Señal | Pin ESP32 |
|-------|-----------|
| BCLK  | GPIO 26   |
| LRCK  | GPIO 25   |
| DATA  | GPIO 22   |

#### NFC PN532 (Software SPI)
| Señal | Pin ESP32 |
|-------|-----------|
| SCK   | GPIO 14   |
| MISO  | GPIO 12   |
| MOSI  | GPIO 27   |
| CS    | GPIO 15   |

### Esquema de Conexión
```
                +------------------- ESP32 DEVKIT -------------------+
                |                                                    |
        3V3 ----+----------------------------------------------------+
        GND ----+--------------------------------------+-------------+
                              VSPI (microSD)           |   I2S
   SD_CS (GPIO13)  ---------[ microSD CS ]             |   BCLK GPIO26 ---> MAX98357A BCLK
   SCK   (GPIO18)  ---------[ microSD CLK ]            |   LRCK GPIO25 ---> MAX98357A LRCK
   MISO  (GPIO19)  <--------[ microSD DO ]             |   DATA GPIO22 ---> MAX98357A DIN
   MOSI  (GPIO23)  --------->[ microSD DI ]            |
                                                      |
                 PN532 (software bit-bang SPI)        |
   GPIO14 (SCK)   --------->[ PN532 SCK ]              |
   GPIO12 (MISO)  <--------[ PN532 MISO ]              |
   GPIO27 (MOSI)  --------->[ PN532 MOSI ]             |
   GPIO15 (CS)    --------->[ PN532 SS ]               |
                                                      |
                 WiFi AP (MusicBox / musicbox123)      |
                +--------------------------------------+
```

## 📦 Instalación y Configuración

### 1. Preparar el Entorno

Necesitas tener instalado:
- [Visual Studio Code](https://code.visualstudio.com/)
- [PlatformIO IDE](https://platformio.org/install/ide?install=vscode)

### 2. Clonar y Compilar

```bash
# Abrir el proyecto en VS Code
# PlatformIO instalará automáticamente todas las dependencias

# Compilar el proyecto
pio run

# Subir a ESP32 (conectado por USB)
pio run --target upload

# Ver monitor serial
pio device monitor
```

### 3. Preparar la Tarjeta SD

1. Formatear la tarjeta SD como **FAT32**
2. Crear carpeta `/music/` en la raíz
3. (Opcional) Copiar archivos MP3 iniciales a `/music/`

### 4. Primera Conexión

1. **Alimentar el ESP32** (USB o 5V)
2. **Buscar la red WiFi**: `MusicBox`
3. **Conectarse** con password: `musicbox123`
4. **Abrir navegador** en: `http://192.168.4.1`

## 🌐 Uso de la Interfaz Web

### Gestión de Canciones

1. **Subir Canción**: 
   - Haz clic en el área de subida
   - Selecciona un archivo MP3
   - Espera a que termine la carga

2. **Eliminar Canción**:
   - Haz clic en "Eliminar" junto a la canción

### Gestión de Tags NFC

1. **Vincular Tag**:
   - Clic en "Vincular Nuevo Tag"
   - Acerca el tag NFC al lector
   - El UID se detectará automáticamente
   - Selecciona la canción a vincular
   - Clic en "Vincular"

2. **Desvincular Tag**:
   - Clic en "Desvincular" junto al tag

## 🎵 Uso Diario

1. **Reproducir**: Acerca un tag NFC vinculado
2. **Pausar/Reanudar**: Acerca el mismo tag mientras suena
3. **Cambiar canción**: Acerca un tag diferente

## 📡 API REST

### Canciones

```http
# Listar canciones
GET /api/songs

# Subir canción
POST /api/songs/upload
Content-Type: multipart/form-data

# Eliminar canción
DELETE /api/songs/{filename}
```

### Tags NFC

```http
# Listar tags vinculados
GET /api/tags

# Vincular tag
POST /api/tags/link
Content-Type: application/json
{
  "uid": "A1B2C3D4",
  "song": "cancion.mp3"
}

# Desvincular tag
DELETE /api/tags/{uid}

# Escanear tag (polling)
GET /api/tags/scan
```

### Estado

```http
# Estado del reproductor
GET /api/status
```

## ⚙️ Configuración Avanzada

### Cambiar Credenciales WiFi

Editar `include/config.h`:
```cpp
#define WIFI_SSID "MiMusicBox"
#define WIFI_PASSWORD "mipassword123"
```

### Ajustar Volumen Predeterminado

Editar `include/config.h`:
```cpp
#define DEFAULT_VOLUME 0.8f  // 0.0 a 1.0
```

### Cambiar Tiempos de Detección NFC

Editar `include/config.h`:
```cpp
#define NFC_POLL_INTERVAL 100    // ms entre lecturas
#define NFC_DEBOUNCE_TIME 1500   // ms para debounce
```

## 🐛 Solución de Problemas

### PN532 no detectado
- Verifica las conexiones (especialmente MISO con pull-up)
- Asegura que CS está en HIGH cuando no se usa
- Comprueba alimentación 3.3V estable

### SD Card no monta
- Verifica formato FAT32
- Revisa conexiones SPI
- Prueba con otra tarjeta SD
- Acorta cables si son muy largos

### Audio con clicks o ruido
- Añade capacitor 100µF cerca del MAX98357A
- Verifica alimentación 5V estable
- Separa cables de audio de otros cables

### No arranca después de programar
- GPIO15 puede interferir en boot
- Reasigna CS del PN532 a otro pin (ej: GPIO5)
- Mantén GPIO15 en HIGH durante boot

## 📝 Persistencia de Datos

- **Música**: Archivos MP3 en `/music/` de la SD
- **Enlaces NFC**: Archivo `/nfc_links.json` en la SD

Ejemplo de `nfc_links.json`:
```json
{
  "links": [
    {"uid": "A1B2C3D4", "song": "cancion1.mp3"},
    {"uid": "E5F6A7B8", "song": "cancion2.mp3"}
  ]
}
```

## 🔒 Seguridad

- Red WiFi protegida por contraseña
- Sin acceso a Internet (AP local)
- Archivos solo accesibles desde la red local

## 📄 Licencia

Este proyecto es de código abierto y está disponible bajo la licencia MIT.

## 🤝 Contribuciones

Las contribuciones son bienvenidas. Por favor:
1. Haz fork del proyecto
2. Crea una rama para tu feature
3. Commit tus cambios
4. Push a la rama
5. Abre un Pull Request

## 📞 Soporte

Para reportar problemas o sugerencias, abre un issue en el repositorio.

---

**¡Disfruta de tu MusicBox!** 🎵
