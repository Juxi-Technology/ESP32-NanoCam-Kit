# Specifiche hardware ESP32-S3 NanoCam

> Fonte: schemi elettrici NanoCamModule.pdf + NanoCamBASE.pdf.
> Architettura: la scheda core (ESP32-S3 + fotocamera + audio) si innesta sulla scheda base (alimentazione USB + flashing seriale).

---

## 1. Panoramica hardware

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

## 2. MCU e memoria

### 2.1 Chip principale

| Parametro | Specifica |
|---|---|
| Modello | **ESP32-S3 N16R8** |
| Package | QFN56 |
| CPU | Xtensa LX7 dual-core 240MHz |
| PSRAM | **8MB** (Octal SPI, suffisso R8) |
| Flash | **16MB** (suffisso N16, SPI Flash esterna da 128M-bit) |
| Clock | cristallo passivo 40MHz + 2 condensatori di adattamento da 10pF |

### 2.2 Memoria Flash

| Parametro | Specifica |
|---|---|
| Modello | **GD25Q128ESIG** |
| Capacità | 128M-bit = **16MB** |
| Interfaccia | SPI (standard a quattro fili) |
| Marca | GigaDevice |

| Segnale Flash | GPIO ESP32-S3 | Funzione |
|---|---|---|
| CS | IO29 | Chip select |
| SO (DO) | IO31 | Uscita dati |
| SI (DI) | IO32 | Ingresso dati |
| SCLK | IO30 | Clock |
| WP | IO34 | Protezione da scrittura |
| HOLD | IO33 | Hold |

---

## 3. Sottosistema fotocamera

### 3.1 Interfaccia DVP

| Parametro | Specifica |
|---|---|
| Tipo di interfaccia | FPC 0.5 WS 24P (connettore FPC a 24 pin) |
| Sensori supportati | GC2145 (predefinito) / OV2640 / OV5640 e altre fotocamere DVP |
| Bus di configurazione I2C | SDA=IO41, SCL=IO42 (condiviso con l'I2C esterno) |
| Formato pixel | RGB565 / JPEG / YUV422 ecc. |

### 3.2 Mappatura completa dei pin DVP

| Categoria | Nome segnale | GPIO ESP32 | Note |
|---|---|---|---|
| **Clock di sistema** | XCLK | **IO9** | Uscita clock principale 24MHz |
| **Clock pixel** | PCLK | **IO6** | Ingresso clock pixel della fotocamera |
| **Sincronizzazione frame** | VSYNC | **IO13** | Sincronizzazione verticale |
| **Sincronizzazione riga** | HREF | **IO11** | Riferimento orizzontale |
| **Controllo** | PWDN | **IO12** | Controllo spegnimento fotocamera |
| **Controllo** | RESET | **IO14** | Reset della fotocamera |
| **I2C** | SDA | **IO41** | Dati I2C fotocamera (condivisi con ES8311) |
| **I2C** | SCL | **IO42** | Clock I2C fotocamera (condiviso con ES8311) |
| **Dati D0** | Y2 | **IO4** | Dati pixel bit0 |
| **Dati D1** | Y3 | **IO2** | Dati pixel bit1 |
| **Dati D2** | Y4 | **IO1** | Dati pixel bit2 |
| **Dati D3** | Y5 | **IO3** | Dati pixel bit3 |
| **Dati D4** | Y6 | **IO5** | Dati pixel bit4 |
| **Dati D5** | Y7 | **IO7** | Dati pixel bit5 |
| **Dati D6** | Y8 | **IO8** | Dati pixel bit6 |
| **Dati D7** | Y9 | **IO10** | Dati pixel bit7 |

### 3.3 Riepilogo dei gruppi di segnali della fotocamera

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

## 4. Sottosistema audio

### 4.1 Architettura audio

```
AP2718AT (模拟MEMS麦) → ES8311 Codec (ADC/DAC) → NS4150B (模拟功放) → 扬声器
                               ↕ I2S (MCLK=39,BCLK=38,WS=47,DOUT=48,DIN=40) + I2C (41/42, addr=0x30)
                            ESP32-S3
```

| Componente | Modello | Tipo | Interfaccia |
|---|---|---|---|
| Microfono | **AP2718AT** | MEMS in silicio analogico | Analogico differenziale → ADC ES8311 |
| Codec | **ES8311** | Codec audio (I2S+I2C) | I2S=MCLK=39,BCLK=38,WS=47,DOUT=48,DIN=40, I2C=41/42, addr=0x30 |
| Amplificatore | **NS4150B** | Amplificatore analogico in classe D | Uscita DAC ES8311 → ingresso analogico → altoparlante |

### 4.2 Pin I2S del codec ES8311

| Segnale I2S | GPIO ESP32 | Funzione |
|---|---|---|
| MCLK | **IO39** | Clock principale |
| BCLK (SCLK) | **IO38** | Bit clock |
| LRCLK (WS) | **IO47** | Clock dei canali sinistro/destro |
| DOUT (DAC) | **IO48** | Uscita dati seriale (riproduzione) |
| DIN (ADC) | **IO40** | Ingresso dati seriale (registrazione) |

> L'audio utilizza la modalità I2S0 Standard Duplex; il codec ES8311 supporta contemporaneamente registrazione e riproduzione.

### 4.3 Controllo I2C dell'ES8311

| Segnale I2C | GPIO | Descrizione |
|---|---|---|
| SDA | IO41 | Condiviso con il bus I2C della fotocamera |
| SCL | IO42 | Condiviso con il bus I2C della fotocamera |
| Indirizzo | **0x30** | Indirizzo I2C a 8 bit dell'ES8311 |

---

## 5. Sistema di alimentazione

### 5.1 Catena di alimentazione

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

### 5.2 Componenti principali

| Chip | Modello | Funzione |
|---|---|---|
| LDO | **ME6217C33P5G** (SOT-89-5) | Regolazione 5V→3.3V |
| LDO | **ME6211A18M3G** (SOT-23) | 5V→1.8V (core della fotocamera) |
| LDO | **ME6211A28M3G** ×2 (SOT-23) | 5V→2.8V (U6=AVDD, U7=DOVDD) |
| Filtro d'ingresso U1 | C14 (10μF) + C15 (0.1μF) | Ingresso ME6217C33 |
| Filtro d'uscita U1 | C1 (22μF) | Uscita ME6217C33 |
| Protezione da inversione di polarità | P-MOSFET **NCE3401** | Protezione dell'ingresso di alimentazione della scheda base |
| Resistenza di gate | R11 10KΩ | Gate del P-MOS collegato a massa |

---

## 6. Scheda base e flashing

### 6.1 Da USB a seriale

| Parametro | Specifica |
|---|---|
| Chip | **CH340K** |
| Interfaccia USB | USB-C (USB1) |
| Collegamento seriale | TXD→U0RXD (IO44), RXD→U0TXD (IO43) (incrociato) |
| Resistenze di terminazione USB | R4, R5 22R/1% (in serie su D+/D-) |
| DTR/RTS | Flashing automatico con controllo di BOOT + EN |

### 6.2 Circuito di download automatico

Utilizza Q1, Q2 (transistor NPN S8050) per il flashing automatico:

| Segnale CH340K | Controllo | Pin di destinazione |
|---|---|---|
| DTR | → Q1 → | ESP32 **EN** (CHIP_PU) |
| RTS | → Q2 → | ESP32 **BOOT** (IO0) |

> Principio: lo strumento di flashing USB, invertendo DTR/RTS, controlla automaticamente la sequenza di BOOT e EN, senza bisogno di premere pulsanti manualmente.

### 6.3 Tabella dei pin del connettore scheda core ↔ scheda base

| Header scheda base | Header scheda core | Nome segnale | GPIO ESP32 | Funzione |
|---|---|---|---|---|
| P1-1 | P1-1 | **VDD50** | — | Uscita alimentazione 5V |
| P1-2 | P1-2 | **VDD50** | — | Uscita alimentazione 5V |
| P1-3 | P1-3 | **GND** | — | Massa di alimentazione |
| P1-4 | P1-4 | **GND** | — | Massa di alimentazione |
| P1-5 | P1-5 | **SDA** | IO41 | Dati I2C (condivisi con fotocamera/ES8311) |
| P1-6 | P1-6 | **SCL** | IO42 | Clock I2C (condiviso con fotocamera/ES8311) |
| P1-7 | P1-7 | **U0TXD** | IO43 | Trasmissione seriale 0 (collegato a CH340K RXD) |
| P1-8 | P1-8 | **U0RXD** | IO44 | Ricezione seriale 0 (collegato a CH340K TXD) |
| **P2-1** | P2-1 | **ESP_P** | IO20 | Dati differenziali USB positivi (D+) |
| **P2-2** | P2-2 | **ESP_N** | IO19 | Dati differenziali USB negativi (D-) |
| P2-3 | P2-3 | **VDD33** | — | Uscita alimentazione 3.3V |
| P2-4 | P2-4 | **VDD33** | — | Uscita alimentazione 3.3V |
| P2-5 | P2-5 | **GND** | — | Massa di alimentazione |
| P2-6 | P2-6 | **GND** | — | Massa di alimentazione |
| P2-7 | P2-7 | **BOOT** | IO0 | Modalità di avvio/flashing |
| P2-8 | P2-8 | **CHIP_PU** | EN | Controllo reset del chip |

### 6.4 Interfacce di espansione della scheda base

| Interfaccia | Segnali | Utilizzo |
|---|---|---|
| P3 (I2C, 4Pin) | 5V1, GND, SDA, SCL | Dispositivi I2C esterni |
| P4 (UART, 4Pin) | 5V1, GND, ESP_P, ESP_N | Dispositivi USB/seriali esterni |

---

## 7. Tabella completa dell'occupazione dei GPIO

> **Totale**: 33 GPIO occupati, circa 8 GPIO disponibili.

| GPIO | Funzione | Segnale | Note |
|---|---|---|---|
| **IO0** | BOOT | BOOT | Modalità di avvio/flashing (pulsante S2, pull-up 10K + antirimbalzo 0.1μF) |
| **IO1** | DVP D2 | CAM_Y4 | Dati pixel bit2 |
| **IO2** | DVP D1 | CAM_Y3 | Dati pixel bit1 |
| **IO3** | DVP D3 | CAM_Y5 | Dati pixel bit3 |
| **IO4** | DVP D0 | CAM_Y2 | Dati pixel bit0 |
| **IO5** | DVP D4 | CAM_Y6 | Dati pixel bit4 |
| **IO6** | DVP PCLK | CAM_PCLK | Ingresso clock pixel |
| **IO7** | DVP D5 | CAM_Y7 | Dati pixel bit5 |
| **IO8** | DVP D6 | CAM_Y8 | Dati pixel bit6 |
| **IO9** | DVP XCLK | CAM_XCLK | Clock principale fotocamera (24MHz) |
| **IO10** | DVP D7 | CAM_Y9 | Dati pixel bit7 |
| **IO11** | DVP HREF | CAM_HREF | Sincronizzazione riga |
| **IO12** | DVP PWDN | CAM_PWDN | Controllo spegnimento fotocamera |
| **IO13** | DVP VSYNC | CAM_VSYNC | Sincronizzazione frame |
| **IO14** | DVP RESET | CAM_RESET | Reset fotocamera |
| **IO18** | WS2812 | RGB_LED_DIN | Ingresso dati LED RGB WS2812 |
| **IO19** | USB differenziale | D- (ESP_N) | Dati differenziali USB negativi (D-) |
| **IO20** | USB differenziale | D+ (ESP_P) | Dati differenziali USB positivi (D+) |
| **IO21** | WS2812 DOUT | RGB_LED_DOUT | Uscita a cascata WS2812 (utilizzabile senza cascata) |
| **IO29** | SPI CS | Flash_CS | Chip select della Flash esterna |
| **IO30** | SPI SCLK | Flash_CLK | Clock della Flash esterna |
| **IO31** | SPI SO | Flash_DO | Uscita dati della Flash esterna |
| **IO32** | SPI SI | Flash_DI | Ingresso dati della Flash esterna |
| **IO33** | SPI HOLD | Flash_HOLD | Hold della Flash esterna |
| **IO34** | SPI WP | Flash_WP | Protezione da scrittura della Flash esterna |
| **IO38** | I2S BCLK | I2S_BCLK | Bit clock ES8311 |
| **IO39** | I2S MCLK | I2S_MCLK | Clock principale ES8311 |
| **IO40** | I2S DIN | I2S_DIN | Ingresso dati ADC ES8311 |
| **IO41** | I2C SDA | I2C_SDA | Condiviso tra fotocamera + ES8311 + I2C esterno |
| **IO42** | I2C SCL | I2C_SCL | Condiviso tra fotocamera + ES8311 + I2C esterno |
| **IO43** | UART0 TX | UTXD | Trasmissione seriale 0 (CH340K sulla scheda base) |
| **IO44** | UART0 RX | URXD | Ricezione seriale 0 (CH340K sulla scheda base) |
| **IO47** | I2S WS | I2S_WS | Word select ES8311 (LRCK) |
| **IO48** | I2S DOUT | I2S_DOUT | Uscita dati DAC ES8311 |
| **EN** | CHIP_PU | RST/EN | Enable/reset (pulsante S1, pull-up 10K) |

### GPIO disponibili (non occupati, espandibili)

| GPIO | Caratteristica | Uso consigliato |
|---|---|---|
| IO15 | IO generico | Espansione |
| IO16 | IO generico | Espansione |
| IO17 | IO generico | Espansione |
| IO35 | IO generico | Espansione |
| IO36 | IO generico | Espansione |
| IO37 | IO generico | Espansione |
| IO45 | IO generico | Espansione |
| IO46 | IO generico | Espansione |

> ⚠️ GPIO occupati: 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,18,19,20,21,29,30,31,32,33,34,38,39,40,41,42,43,44,47,48
Quantità disponibile: 8 GPIO

---

## 8. Vincoli di progettazione principali

### 8.1 Configurazione delle periferiche I2S

- L'ESP32-S3 dispone di due controller I2S:
  - **I2S0**: già assegnato al codec ES8311 (MCLK=IO39, BCLK=IO38, WS=IO47, DOUT=IO48, DIN=IO40), I2S Duplex bidirezionale, supporta contemporaneamente registrazione e riproduzione
- Controllo I2C dell'ES8311: SDA=IO41, SCL=IO42, indirizzo del dispositivo 0x30

### 8.2 Coesistenza di DVP e DMA I2S

- DVP utilizza il canale DMA della periferica **LCD_CAM**
- ES8311 utilizza il DMA di I2S0 (interfaccia I2S singola bidirezionale)
- I due non sono in conflitto, ma occorre considerare la larghezza di banda della PSRAM (gli 8MB di PSRAM sono sufficienti)
- GPIO45/46 non sono collegati all'audio e possono essere usati per altri scopi; GPIO47 è utilizzato per I2S WS

### 8.3 Condivisione del bus I2C

- IO41 (SDA) + IO42 (SCL) sono collegati contemporaneamente alla fotocamera e all'interfaccia I2C esterna
- Resistenze di pull-up I2C: R12 (SDA, 4.7K/1%), R13 (SCL, 4.7K/1%), verso VDD33
- Occorre assicurarsi che l'indirizzo I2C della fotocamera non entri in conflitto con i dispositivi esterni
- Indirizzo I2C predefinito dell'OV2640: 0x60 (scrittura) / 0x61 (lettura) — nessun conflitto con ES8311 (0x30)

### 8.4 Capacità Flash sufficiente

- I **16MB di Flash** offrono spazio abbondante, in grado di supportare:
  - schema OTA a doppia partizione (factory + ota_0 + ota_1)
  - SPIFFS per il pannello di amministrazione Web (~1MB)
  - pacchetti di risorse multilingue di XiaoZhi AI (file .p3)
  - partizione di configurazione NVS
  - spazio riservato per gli aggiornamenti
- Non c'è da preoccuparsi per la dimensione del firmware: l'IA multi-agente può essere contenuta per intero

### 8.5 Vincoli sui pin di ingresso della fotocamera DVP

- D0-D7 e VSYNC/HREF/PCLK sono tutti pin di ingresso, pilotati dal sensore della fotocamera
- Nello sviluppo del driver questi pin devono essere configurati esclusivamente in modalità INPUT
- I pin I2S audio (38,39,40,47,48) possono essere configurati in modalità output (supportato dall'ESP32-S3)

---

## 9. Suggerimenti di configurazione ESP-IDF

### 9.1 Voci chiave di sdkconfig

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

# Telecamera
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

# Audio (codec ES8311, I2S0)
# ES8311 I2S: MCLK=39, BCLK=38, WS=47, DOUT=48, DIN=40
# ES8311 I2C: SDA=41, SCL=42, addr=0x30
```

### 9.2 Configurazione della fotocamera

```c
// Configurazione dei pin di esp32-camera
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
    .pixel_format = PIXFORMAT_RGB565,  // Oppure PIXFORMAT_JPEG
    .frame_size   = FRAMESIZE_QVGA,    // 320x240
    .jpeg_quality = 12,
    .fb_count     = 2,
    .fb_location  = CAMERA_FB_IN_PSRAM,
    .grab_mode    = CAMERA_GRAB_WHEN_EMPTY,
};
```
