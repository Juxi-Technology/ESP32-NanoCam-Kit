# Especificaciones de hardware del ESP32-S3 NanoCam

> Fuente: esquemáticos NanoCamModule.pdf + NanoCamBASE.pdf.
> Arquitectura: la placa central (ESP32-S3 + cámara + audio) se inserta sobre la placa base (alimentación por USB + grabación por puerto serie).

---

## 1. Visión general del hardware

```
┌─────────────────────────────────────────────────────────┐
│                    NanoCam 核心板                         │
│  ┌──────────┐  ┌──────────┐  ┌────────────┐            │
│  │ ESP32-S3 │  │ ES8311   │  │ NS4150B   │            │
│  │   R8     │  │ I2S 麦克风│  │ I2S 功放   │            │
│  │ (QFN56)  │  │          │  │            │            │
│  └────┬─────┘  └──────────┘  └────────────┘            │
│       │                                                 │
│  ┌────┴─────┐  ┌──────────┐  ┌────────────┐            │
│  │ GD25Q128 │  │  DVP     │  │ ME6217C33  │            │
│  │ 16MB Flash│ │ 摄像头   │  │ 3.3V LDO   │            │
│  └──────────┘  │ FPC-24P  │  └────────────┘            │
│                └──────────┘                             │
├─────────────────────────────────────────────────────────┤
│                    NanoCam 底板                          │
│  ┌──────────┐  ┌──────────┐  ┌────────────────────┐    │
│  │ USB-C    │  │ CH340K   │  │ 一键下载电路        │    │
│  │ 5V输入   │  │ USB-UART │  │ (DTR/RTS→BOOT/EN)  │    │
│  └──────────┘  └──────────┘  └────────────────────┘    │
│                                                         │
│  扩展接口: I2C ×1, UART ×1, 5V/GND/3.3V                 │
└─────────────────────────────────────────────────────────┘
```

---

## 2. MCU y almacenamiento

### 2.1 Chip MCU

| Parámetro | Especificación |
|---|---|
| Modelo | **ESP32-S3 N16R8** |
| Encapsulado | QFN56 |
| CPU | Xtensa LX7 doble núcleo 240MHz |
| PSRAM | **8MB** (Octal SPI, sufijo R8) |
| Flash | **16MB** (sufijo N16, SPI Flash externo de 128M-bit) |
| Reloj | Cristal pasivo de 40MHz + condensadores de adaptación 10pF×2 |

### 2.2 Almacenamiento Flash

| Parámetro | Especificación |
|---|---|
| Modelo | **GD25Q128ESIG** |
| Capacidad | 128M-bit = **16MB** |
| Interfaz | SPI (cuatro hilos estándar) |
| Marca | GigaDevice |

| Señal Flash | GPIO ESP32-S3 | Función |
|---|---|---|
| CS | IO29 | Selección de chip |
| SO (DO) | IO31 | Salida de datos |
| SI (DI) | IO32 | Entrada de datos |
| SCLK | IO30 | Reloj |
| WP | IO34 | Protección contra escritura |
| HOLD | IO33 | Retención (Hold) |

---

## 3. Subsistema de cámara

### 3.1 Interfaz DVP

| Parámetro | Especificación |
|---|---|
| Tipo de interfaz | FPC 0.5 WS 24P (zócalo FPC de 24 pines) |
| Sensor compatible | GC2145 (predeterminado) / OV2640 / OV5640 y otras cámaras DVP |
| Bus de configuración I2C | SDA=IO41, SCL=IO42 (compartido con el I2C externo) |
| Formato de píxel | RGB565 / JPEG / YUV422, etc. |

### 3.2 Mapeo completo de pines DVP

| Categoría de señal | Nombre de señal | GPIO ESP32 | Nota |
|---|---|---|---|
| **Reloj del sistema** | XCLK | **IO9** | Salida de reloj maestro 24MHz |
| **Reloj de píxel** | PCLK | **IO6** | Entrada de reloj de píxel de la cámara |
| **Sincronización de fotograma** | VSYNC | **IO13** | Sincronización vertical |
| **Sincronización de línea** | HREF | **IO11** | Referencia horizontal |
| **Control** | PWDN | **IO12** | Control de apagado de la cámara |
| **Control** | RESET | **IO14** | Reset de la cámara |
| **I2C** | SDA | **IO41** | Datos I2C de la cámara (compartido con ES8311) |
| **I2C** | SCL | **IO42** | Reloj I2C de la cámara (compartido con ES8311) |
| **Datos D0** | Y2 | **IO4** | Datos de píxel bit0 |
| **Datos D1** | Y3 | **IO2** | Datos de píxel bit1 |
| **Datos D2** | Y4 | **IO1** | Datos de píxel bit2 |
| **Datos D3** | Y5 | **IO3** | Datos de píxel bit3 |
| **Datos D4** | Y6 | **IO5** | Datos de píxel bit4 |
| **Datos D5** | Y7 | **IO7** | Datos de píxel bit5 |
| **Datos D6** | Y8 | **IO8** | Datos de píxel bit6 |
| **Datos D7** | Y9 | **IO10** | Datos de píxel bit7 |

### 3.3 Resumen de grupos de señales de la cámara

```
Camera DVP 信号组:
  XCLK  = IO9     # Master clock out (24MHz)
  PCLK  = IO6     # Pixel clock in
  VSYNC = IO13    # Vertical sync
  HREF  = IO11    # Horizontal ref
  PWDN  = IO12    # Power down (active low)
  RESET = IO14    # Hardware reset

  D0(Y2) = IO4    D1(Y3) = IO2
  D2(Y4) = IO1    D3(Y5) = IO3
  D4(Y6) = IO5    D5(Y7) = IO7
  D6(Y8) = IO8    D7(Y9) = IO10

  SIOD(SDA) = IO41
  SIOC(SCL) = IO42
```

---

## 4. Subsistema de audio

### 4.1 Arquitectura de audio

```
AP2718AT (模拟MEMS麦) → ES8311 Codec (ADC/DAC) → NS4150B (模拟功放) → 扬声器
                               ↕ I2S (MCLK=39,BCLK=38,WS=47,DOUT=48,DIN=40) + I2C (41/42, addr=0x30)
                            ESP32-S3
```

| Componente | Modelo | Tipo | Interfaz |
|---|---|---|---|
| Micrófono | **AP2718AT** | Micrófono MEMS de silicio analógico | Analógico diferencial → ADC del ES8311 |
| Códec | **ES8311** | Códec de audio (I2S+I2C) | I2S=MCLK=39,BCLK=38,WS=47,DOUT=48,DIN=40, I2C=41/42, addr=0x30 |
| Amplificador | **NS4150B** | Amplificador analógico clase D | Salida DAC del ES8311 → entrada analógica → altavoz |

### 4.2 Pines I2S del códec ES8311

| Señal I2S | GPIO ESP32 | Función |
|---|---|---|
| MCLK | **IO39** | Reloj maestro |
| BCLK (SCLK) | **IO38** | Reloj de bits |
| LRCLK (WS) | **IO47** | Reloj de canal izquierdo/derecho |
| DOUT (DAC) | **IO48** | Salida de datos serie (reproducción) |
| DIN (ADC) | **IO40** | Entrada de datos serie (grabación) |

> El audio usa el modo I2S0 Standard Duplex; el códec ES8311 admite grabación y reproducción a la vez.

### 4.3 Control I2C del ES8311

| Señal I2C | GPIO | Descripción |
|---|---|---|
| SDA | IO41 | Bus I2C compartido con la cámara |
| SCL | IO42 | Bus I2C compartido con la cámara |
| Dirección | **0x30** | Dirección I2C de 8 bits del ES8311 |

---

## 5. Sistema de alimentación

### 5.1 Cadena de alimentación

```
USB-C (5V) ──→ 底板 NCE3401 P-MOSFET 反接保护 ──→ VDD50
                                              │
                    ┌─────────────────────────┤
                    ↓                         ↓
            核心板 ME6217C33 LDO            NS4150B 功放
                    │                  (5V 直供)
                    ↓
                 VDD33 (3.3V)
                    │
        ┌───────────┼───────────┐
        ↓           ↓           ↓
    ESP32-S3    ES8311 Codec   GD25Q128 Flash
    DVP摄像头   AP2718AT 麦    ME6211A18/28 LDO
```

### 5.2 Componentes clave

| Chip | Modelo | Función |
|---|---|---|
| LDO | **ME6217C33P5G** (SOT-89-5) | Regulación 5V→3.3V |
| LDO | **ME6211A18M3G** (SOT-23) | 5V→1.8V (núcleo de la cámara) |
| LDO | **ME6211A28M3G** ×2 (SOT-23) | 5V→2.8V (U6=AVDD, U7=DOVDD) |
| Filtro de entrada de U1 | C14 (10μF) + C15 (0.1μF) | Entrada del ME6217C33 |
| Filtro de salida de U1 | C1 (22μF) | Salida del ME6217C33 |
| Protección contra polaridad inversa | **NCE3401** P-MOSFET | Protección de la entrada de alimentación de la placa base |
| Resistencia de puerta | R11 10KΩ | Puerta del P-MOS a tierra |

---

## 6. Placa base y grabación

### 6.1 USB a serie

| Parámetro | Especificación |
|---|---|
| Chip | **CH340K** |
| Interfaz USB | USB-C (USB1) |
| Conexión serie | TXD→U0RXD (IO44), RXD→U0TXD (IO43) (cruzada) |
| Resistencias de adaptación USB | R4, R5 22R/1% (en serie con D+/D-) |
| DTR/RTS | Control automático de BOOT + EN para la grabación con un clic |

### 6.2 Circuito de grabación automática con un clic

Para la grabación automática se usan Q1 y Q2 (transistores NPN S8050):

| Señal CH340K | Control | Pin de destino |
|---|---|---|
| DTR | → Q1 → | ESP32 **EN** (CHIP_PU) |
| RTS | → Q2 → | ESP32 **BOOT** (IO0) |

> Principio: la herramienta de grabación USB controla automáticamente la secuencia de BOOT y EN mediante la conmutación de DTR/RTS, sin necesidad de pulsar botones manualmente.

### 6.3 Tabla de pines del conector placa central ↔ placa base

| Pin placa base | Pin placa central | Nombre de señal | GPIO ESP32 | Función |
|---|---|---|---|---|
| P1-1 | P1-1 | **VDD50** | — | Salida de alimentación 5V |
| P1-2 | P1-2 | **VDD50** | — | Salida de alimentación 5V |
| P1-3 | P1-3 | **GND** | — | Tierra de alimentación |
| P1-4 | P1-4 | **GND** | — | Tierra de alimentación |
| P1-5 | P1-5 | **SDA** | IO41 | Datos I2C (compartido con cámara/ES8311) |
| P1-6 | P1-6 | **SCL** | IO42 | Reloj I2C (compartido con cámara/ES8311) |
| P1-7 | P1-7 | **U0TXD** | IO43 | Transmisión del puerto serie 0 (conecta al RXD del CH340K) |
| P1-8 | P1-8 | **U0RXD** | IO44 | Recepción del puerto serie 0 (conecta al TXD del CH340K) |
| **P2-1** | P2-1 | **ESP_P** | IO20 | Datos diferenciales USB positivos (D+) |
| **P2-2** | P2-2 | **ESP_N** | IO19 | Datos diferenciales USB negativos (D-) |
| P2-3 | P2-3 | **VDD33** | — | Salida de alimentación 3.3V |
| P2-4 | P2-4 | **VDD33** | — | Salida de alimentación 3.3V |
| P2-5 | P2-5 | **GND** | — | Tierra de alimentación |
| P2-6 | P2-6 | **GND** | — | Tierra de alimentación |
| P2-7 | P2-7 | **BOOT** | IO0 | Modo de arranque/grabación |
| P2-8 | P2-8 | **CHIP_PU** | EN | Control de reset del chip |

### 6.4 Interfaz de expansión de la placa base

| Interfaz | Señales expuestas | Uso |
|---|---|---|
| P3 (I2C, 4Pin) | 5V1, GND, SDA, SCL | Dispositivos I2C externos |
| P4 (UART, 4Pin) | 5V1, GND, ESP_P, ESP_N | Dispositivos USB/serie externos |

---

## 7. Tabla completa de uso de GPIO

> **Total**: 33 GPIO ocupados; alrededor de 8 GPIO disponibles.

| GPIO | Función | Señal | Nota |
|---|---|---|---|
| **IO0** | BOOT | BOOT | Modo de arranque/grabación (botón S2, pull-up de 10K + antirrebote de 0.1μF) |
| **IO1** | DVP D2 | CAM_Y4 | Datos de píxel bit2 |
| **IO2** | DVP D1 | CAM_Y3 | Datos de píxel bit1 |
| **IO3** | DVP D3 | CAM_Y5 | Datos de píxel bit3 |
| **IO4** | DVP D0 | CAM_Y2 | Datos de píxel bit0 |
| **IO5** | DVP D4 | CAM_Y6 | Datos de píxel bit4 |
| **IO6** | DVP PCLK | CAM_PCLK | Entrada de reloj de píxel |
| **IO7** | DVP D5 | CAM_Y7 | Datos de píxel bit5 |
| **IO8** | DVP D6 | CAM_Y8 | Datos de píxel bit6 |
| **IO9** | DVP XCLK | CAM_XCLK | Reloj maestro de la cámara (24MHz) |
| **IO10** | DVP D7 | CAM_Y9 | Datos de píxel bit7 |
| **IO11** | DVP HREF | CAM_HREF | Sincronización de línea |
| **IO12** | DVP PWDN | CAM_PWDN | Control de apagado de la cámara |
| **IO13** | DVP VSYNC | CAM_VSYNC | Sincronización de fotogramas |
| **IO14** | DVP RESET | CAM_RESET | Reset de la cámara |
| **IO18** | WS2812 | RGB_LED_DIN | Entrada de datos del LED RGB WS2812 |
| **IO19** | Diferencial USB | D- (ESP_N) | Datos diferenciales USB negativos (D-) |
| **IO20** | Diferencial USB | D+ (ESP_P) | Datos diferenciales USB positivos (D+) |
| **IO21** | WS2812 DOUT | RGB_LED_DOUT | Salida en cascada WS2812 (disponible si no hay cascada) |
| **IO29** | SPI CS | Flash_CS | Selección de chip del Flash externo |
| **IO30** | SPI SCLK | Flash_CLK | Reloj del Flash externo |
| **IO31** | SPI SO | Flash_DO | Salida de datos del Flash externo |
| **IO32** | SPI SI | Flash_DI | Entrada de datos del Flash externo |
| **IO33** | SPI HOLD | Flash_HOLD | Hold del Flash externo |
| **IO34** | SPI WP | Flash_WP | Protección contra escritura del Flash externo |
| **IO38** | I2S BCLK | I2S_BCLK | Reloj de bits del ES8311 |
| **IO39** | I2S MCLK | I2S_MCLK | Reloj maestro del ES8311 |
| **IO40** | I2S DIN | I2S_DIN | Entrada de datos del ADC del ES8311 |
| **IO41** | I2C SDA | I2C_SDA | Compartido entre cámara + ES8311 + I2C externo |
| **IO42** | I2C SCL | I2C_SCL | Compartido entre cámara + ES8311 + I2C externo |
| **IO43** | UART0 TX | UTXD | Transmisión del puerto serie 0 (CH340K de la placa base) |
| **IO44** | UART0 RX | URXD | Recepción del puerto serie 0 (CH340K de la placa base) |
| **IO47** | I2S WS | I2S_WS | Selección de palabra del ES8311 (LRCK) |
| **IO48** | I2S DOUT | I2S_DOUT | Salida de datos del DAC del ES8311 |
| **EN** | CHIP_PU | RST/EN | Habilitación/reset (botón S1, pull-up de 10K) |

### GPIO disponibles (no ocupados, ampliables)

| GPIO | Característica | Uso recomendado |
|---|---|---|
| IO15 | IO de propósito general | Expansión |
| IO16 | IO de propósito general | Expansión |
| IO17 | IO de propósito general | Expansión |
| IO35 | IO de propósito general | Expansión |
| IO36 | IO de propósito general | Expansión |
| IO37 | IO de propósito general | Expansión |
| IO45 | IO de propósito general | Expansión |
| IO46 | IO de propósito general | Expansión |

> ⚠️ GPIO ocupados: 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,18,19,20,21,29,30,31,32,33,34,38,39,40,41,42,43,44,47,48
Cantidad disponible: 8 GPIO

---

## 8. Restricciones clave de diseño

### 8.1 Configuración del periférico I2S

- El ESP32-S3 tiene dos controladores I2S:
  - **I2S0**: asignado al códec ES8311 (MCLK=IO39, BCLK=IO38, WS=IO47, DOUT=IO48, DIN=IO40), I2S Duplex bidireccional, compatible con grabación y reproducción a la vez
- Control I2C del ES8311: SDA=IO41, SCL=IO42, dirección del dispositivo 0x30

### 8.2 Coexistencia de DMA entre DVP e I2S

- El DVP usa un canal DMA del periférico **LCD_CAM**
- El ES8311 usa el DMA de I2S0 (interfaz I2S única bidireccional)
- Ambos no entran en conflicto, pero hay que tener en cuenta el ancho de banda de la PSRAM (8MB de PSRAM son suficientes)
- GPIO45/46 no están conectados al audio y pueden usarse para otros fines; GPIO47 se usa para I2S WS

### 8.3 Uso compartido del bus I2C

- IO41(SDA) + IO42(SCL) se conectan a la vez a la cámara y a la interfaz I2C externa
- Resistencias pull-up de I2C: R12 (SDA, 4.7K/1%), R13 (SCL, 4.7K/1%), con pull-up a VDD33
- Hay que asegurarse de que la dirección I2C de la cámara no entre en conflicto con los dispositivos externos
- Dirección I2C predeterminada del OV2640: 0x60 (escritura) / 0x61 (lectura) — no entra en conflicto con el ES8311 (0x30)

### 8.4 Capacidad de Flash suficiente

- Los **16MB de Flash** ofrecen espacio de sobra para:
  - Esquema OTA de doble partición (factory + ota_0 + ota_1)
  - SPIFFS para el panel de administración web (~1MB)
  - Paquete de recursos multilingües de XiaoZhi AI (archivos .p3)
  - Partición de configuración NVS
  - Espacio reservado para actualizaciones
- No hay que preocuparse por el tamaño del firmware; la IA multiagente cabe completa

### 8.5 Restricciones de los pines de entrada de la cámara DVP

- D0-D7 y VSYNC/HREF/PCLK son pines de entrada, controlados por el sensor de la cámara
- Al desarrollar el controlador hay que asegurarse de que estos pines se configuren únicamente en modo INPUT
- Los pines de audio I2S (38,39,40,47,48) pueden configurarse en modo salida (compatible con el ESP32-S3)

---

## 9. Recomendaciones de configuración de ESP-IDF

### 9.1 Elementos clave de sdkconfig

```
# Chip
CONFIG_IDF_TARGET_ESP32S3=y
CONFIG_ESP32S3_DEFAULT_CPU_FREQ_240=y

# Flash (16MB)
CONFIG_ESPTOOLPY_FLASHSIZE_16MB=y
CONFIG_ESPTOOLPY_FLASHMODE_QIO=y
CONFIG_ESPTOOLPY_FLASHFREQ_80M=y

# PSRAM (8MB Octal)
CONFIG_SPIRAM=y
CONFIG_SPIRAM_MODE_OCT=y
CONFIG_SPIRAM_SPEED_80M=y
CONFIG_SPIRAM_MALLOC_ALWAYSINTERNAL=4096
CONFIG_SPIRAM_MALLOC_RESERVE_INTERNAL=49152
CONFIG_MBEDTLS_EXTERNAL_MEM_ALLOC=y

# Cache
CONFIG_ESP32S3_INSTRUCTION_CACHE_32KB=y
CONFIG_ESP32S3_DATA_CACHE_64KB=y
CONFIG_ESP32S3_DATA_CACHE_LINE_64B=y

# Cámara
CONFIG_CAMERA_MODULE_CUSTOM=y
CONFIG_CAMERA_PIN_XCLK=9
CONFIG_CAMERA_PIN_PCLK=6
CONFIG_CAMERA_PIN_VSYNC=13
CONFIG_CAMERA_PIN_HREF=11
CONFIG_CAMERA_PIN_SIOD=41
CONFIG_CAMERA_PIN_SIOC=42
CONFIG_CAMERA_PIN_PWDN=12
CONFIG_CAMERA_PIN_RESET=14
CONFIG_CAMERA_PIN_Y2=4
CONFIG_CAMERA_PIN_Y3=2
CONFIG_CAMERA_PIN_Y4=1
CONFIG_CAMERA_PIN_Y5=3
CONFIG_CAMERA_PIN_Y6=5
CONFIG_CAMERA_PIN_Y7=7
CONFIG_CAMERA_PIN_Y8=8
CONFIG_CAMERA_PIN_Y9=10
CONFIG_CAMERA_XCLK_FREQ=24000000

# Audio (códec ES8311, I2S0)
# ES8311 I2S: MCLK=39, BCLK=38, WS=47, DOUT=48, DIN=40
# ES8311 I2C: SDA=41, SCL=42, addr=0x30
```

### 9.2 Configuración de la cámara

```c
// Configuración de pines de esp32-camera
camera_config_t config = {
    .pin_pwdn  = 12,
    .pin_reset = 14,
    .pin_xclk  = 9,
    .pin_siod  = 41,
    .pin_sioc  = 42,
    .pin_d7    = 10,
    .pin_d6    = 8,
    .pin_d5    = 7,
    .pin_d4    = 5,
    .pin_d3    = 3,
    .pin_d2    = 1,
    .pin_d1    = 2,
    .pin_d0    = 4,
    .pin_vsync = 13,
    .pin_href  = 11,
    .pin_pclk  = 6,
    .xclk_freq_hz = 24000000,
    .pixel_format = PIXFORMAT_RGB565,  // o PIXFORMAT_JPEG
    .frame_size   = FRAMESIZE_QVGA,    // 320x240
    .jpeg_quality = 12,
    .fb_count     = 2,
    .fb_location  = CAMERA_FB_IN_PSRAM,
    .grab_mode    = CAMERA_GRAB_WHEN_EMPTY,
};
```
