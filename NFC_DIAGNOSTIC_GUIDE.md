# 🔍 Guía de Diagnóstico del PN532 NFC Reader

## El programa de test ya está cargado en tu ESP32

### ¿Qué hace el programa de test?

El programa ejecuta 4 pruebas automáticas:

1. ✅ **Configuración de Pines** - Verifica que los pines están correctos
2. ✅ **Inicialización** - Intenta comunicarse con el PN532
3. ✅ **Versión de Firmware** - Lee la versión del chip PN532
4. ✅ **Configuración SAM** - Configura el módulo para leer tags

### 📺 Para ver los resultados:

#### Opción 1: Usar el script de test
```bash
./test_nfc.sh
```

#### Opción 2: Monitor manual
```bash
~/.platformio/penv/bin/platformio device monitor -e nfc_test
```

Luego presiona el botón **RESET** (o **EN**) en tu ESP32.

---

## 🔧 Si el PN532 NO se detecta

Verás este mensaje:
```
✗ FAILED: Didn't find PN532 board
```

### Pasos de Verificación:

#### 1. **Verificar Modo SPI del PN532**
La mayoría de módulos PN532 tienen **interruptores DIP** o **jumpers**:

**Configuración correcta para SPI:**
- SW1: **ON** (o I0 = 0)
- SW2: **OFF** (o I1 = 1)

Otros modos (incorrectos):
- HSU/UART: SW1=OFF, SW2=OFF
- I2C: SW1=OFF, SW2=ON

#### 2. **Verificar Conexiones Físicas**

| ESP32 Pin | PN532 Pin | Cable/Función |
|-----------|-----------|---------------|
| 3.3V      | VCC       | Alimentación (rojo) |
| GND       | GND       | Tierra (negro) |
| GPIO14    | SCK       | Clock (amarillo/blanco) |
| GPIO12    | MISO      | Datos IN (verde) |
| GPIO27    | MOSI      | Datos OUT (azul) |
| GPIO15    | SS/CS     | Chip Select (morado) |

**Notas importantes:**
- ⚠️ **NO conectar a 5V** - El PN532 usa 3.3V
- Cables cortos (<10cm) funcionan mejor
- MISO debe tener pull-up (la mayoría de módulos lo incluyen)

#### 3. **Verificar Alimentación**

Con un multímetro:
- Medir entre VCC y GND del PN532
- Debe leer **3.3V** (entre 3.2V y 3.4V está OK)
- Si es 0V: problema de conexión o alimentación ESP32
- Si es 5V: ⚠️ **DESCONECTAR INMEDIATAMENTE** - Conectado a 5V

**LED del PN532:**
- Debe estar encendido (rojo o verde según modelo)
- Si no enciende: problema de alimentación

#### 4. **Verificar Soldaduras (si es módulo sin pines)**

Si soldaste tú mismo los pines:
- Revisar cada pin con lupa
- Buscar soldaduras frías (opacas, porosas)
- Verificar continuidad con multímetro
- Verificar que no hay cortocircuitos entre pines

#### 5. **Probar con Cables Más Cortos**

SPI puede ser sensible a cables largos:
- Usar cables Dupont de <10cm
- Mejor aún: soldar directamente al ESP32
- Mantener cables alejados de fuentes de ruido

#### 6. **Verificar GPIO15 (Strapping Pin)**

GPIO15 es un pin especial del ESP32:
- Debe estar HIGH durante el boot
- Si tienes problemas de arranque, prueba otro pin para CS:
  - GPIO5 es una buena alternativa
  - GPIO4 también funciona
  
Para cambiar el pin CS:
1. Edita `include/config.h`
2. Cambia `#define NFC_SS 15` por `#define NFC_SS 5`
3. Reconecta el cable del PN532 CS a GPIO5

---

## ✅ Si el PN532 SE DETECTA

Verás algo como:
```
✓ PN532 Found!
Chip: PN532
Firmware Version: 1.6

Now scanning for NFC tags...
Place an NFC tag near the reader
```

### Prueba de Tags NFC:

1. **Acerca un tag NFC** (tarjeta, llavero, phone con NFC)
2. Deberías ver:
```
========================================
   NFC TAG DETECTED!
========================================
UID Length: 4 bytes
UID Value: AB CD EF 01
Card Type: Mifare Classic (4-byte UID)
========================================
```

3. **Prueba diferentes tags:**
   - Tarjetas de transporte público
   - Amiibos
   - Tags NFC vírgenes
   - Smartphone con NFC (no siempre funciona)

---

## 🔄 Volver al Programa Principal

Una vez que confirmes que el PN532 funciona:

```bash
# Subir de nuevo el programa principal
~/.platformio/penv/bin/platformio run -e esp32dev --target upload

# Ver el monitor
~/.platformio/penv/bin/platformio device monitor -e esp32dev
```

---

## 🆘 Problemas Comunes

### "No encuentro el monitor serial"
```bash
# Listar puertos disponibles
ls /dev/cu.*

# Debería aparecer algo como: /dev/cu.usbserial-XXXX
```

### "Permission denied al conectar"
```bash
# En macOS, dar permisos:
sudo chmod 666 /dev/cu.usbserial-*
```

### "El ESP32 no arranca después de conectar PN532"
- Desconecta GPIO15 (CS del PN532)
- Arranca el ESP32
- Vuelve a conectar GPIO15
- O cambia CS a otro pin (ver punto 6 arriba)

### "Detecta el tag pero luego no responde"
- Cables demasiado largos
- Interferencia de otros dispositivos
- Alimentación insuficiente (probar con fuente externa)

---

## 📊 Lecturas de Voltaje Normales

Con multímetro en el PN532:
- **VCC a GND**: 3.3V (±0.1V)
- **MISO en reposo**: 3.3V (pull-up activo)
- **MOSI en reposo**: Variable
- **SCK en reposo**: HIGH (~3.3V) o LOW (0V)
- **CS en reposo**: HIGH (3.3V) - muy importante

---

## 📝 Información del Sistema

**Pines ESP32 usados:**
- GPIO14 (SCK) - Software SPI Clock
- GPIO12 (MISO) - Master In Slave Out
- GPIO27 (MOSI) - Master Out Slave In  
- GPIO15 (CS) - Chip Select

**Librería:** Adafruit PN532 v1.3.4

**Protocolo:** Software SPI (bit-banging)

---

## 🔗 Recursos Adicionales

- [Datasheet PN532](https://www.nxp.com/docs/en/nxp/data-sheets/PN532_C1.pdf)
- [Adafruit PN532 Guide](https://learn.adafruit.com/adafruit-pn532-rfid-nfc)
- [ESP32 Pinout](https://randomnerdtutorials.com/esp32-pinout-reference-gpios/)
