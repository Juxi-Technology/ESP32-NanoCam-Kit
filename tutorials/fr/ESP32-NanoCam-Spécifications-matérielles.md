# ESP32-NanoCam — Spécifications matérielles

> Source : schémas NanoCamModule.pdf + NanoCAMBASE.pdf.
> Architecture : la carte principale (ESP32-S3 + caméra + audio) s'enfiche sur la carte de base (alimentation USB + flashage série).

---

## 1. Vue d'ensemble du matériel

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

## 2. MCU et stockage

### 2.1 MCU

| Paramètre | Spécification |
|---|---|
| Modèle | **ESP32-S3 N16R8** |
| Boîtier | QFN56 |
| CPU | Xtensa LX7 double cœur 240MHz |
| PSRAM | **8MB** (Octal SPI, suffixe R8) |
| Flash | **16MB** (suffixe N16, SPI Flash externe 128M-bit) |
| Horloge | Quartz passif 40MHz + 2 condensateurs d'adaptation 10pF |

### 2.2 Stockage Flash

| Paramètre | Spécification |
|---|---|
| Modèle | **GD25Q128ESIG** |
| Capacité | 128M-bit = **16MB** |
| Interface | SPI (4 fils standard) |
| Marque | GigaDevice |

| Signal Flash | GPIO ESP32-S3 | Fonction |
|---|---|---|
| CS | IO29 | Sélection de puce |
| SO (DO) | IO31 | Sortie de données |
| SI (DI) | IO32 | Entrée de données |
| SCLK | IO30 | Horloge |
| WP | IO34 | Protection en écriture |
| HOLD | IO33 | Maintien (Hold) |

---

## 3. Sous-système caméra

### 3.1 Interface DVP

| Paramètre | Spécification |
|---|---|
| Type d'interface | FPC 0.5 WS 24P (connecteur FPC 24 broches) |
| Capteurs pris en charge | GC2145 (par défaut) / OV2640 / OV5640 et autres caméras DVP |
| Bus de configuration I2C | SDA=IO41, SCL=IO42 (partagé avec l'I2C externe) |
| Format de pixel | RGB565 / JPEG / YUV422, etc. |

### 3.2 Mappage complet des broches DVP

| Catégorie de signal | Nom du signal | GPIO ESP32 | Remarque |
|---|---|---|---|
| **Horloge système** | XCLK | **IO9** | Sortie d'horloge principale 24MHz |
| **Horloge pixel** | PCLK | **IO6** | Entrée d'horloge pixel caméra |
| **Synchro de trame** | VSYNC | **IO13** | Synchronisation verticale |
| **Synchro de ligne** | HREF | **IO11** | Référence horizontale |
| **Commande** | PWDN | **IO12** | Contrôle power-down caméra |
| **Commande** | RESET | **IO14** | Réinitialisation caméra |
| **I2C** | SDA | **IO41** | Données I2C caméra (partagées avec l'ES8311) |
| **I2C** | SCL | **IO42** | Horloge I2C caméra (partagée avec l'ES8311) |
| **Données D0** | Y2 | **IO4** | Bit 0 des données pixel |
| **Données D1** | Y3 | **IO2** | Bit 1 des données pixel |
| **Données D2** | Y4 | **IO1** | Bit 2 des données pixel |
| **Données D3** | Y5 | **IO3** | Bit 3 des données pixel |
| **Données D4** | Y6 | **IO5** | Bit 4 des données pixel |
| **Données D5** | Y7 | **IO7** | Bit 5 des données pixel |
| **Données D6** | Y8 | **IO8** | Bit 6 des données pixel |
| **Données D7** | Y9 | **IO10** | Bit 7 des données pixel |

### 3.3 Récapitulatif des groupes de signaux caméra

```
Groupe de signaux DVP caméra :
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

## 4. Sous-système audio

### 4.1 Architecture audio

```
AP2718AT (micro MEMS analogique) → ES8311 Codec (ADC/DAC) → NS4150B (ampli analogique) → haut-parleur
                               ↕ I2S (MCLK=39,BCLK=38,WS=47,DOUT=48,DIN=40) + I2C (41/42, addr=0x30)
                            ESP32-S3
```

| Composant | Modèle | Type | Interface |
|---|---|---|---|
| Microphone | **AP2718AT** | Micro MEMS analogique | Différentiel analogique → ADC ES8311 |
| Codec | **ES8311** | Codec audio (I2S+I2C) | I2S=MCLK=39,BCLK=38,WS=47,DOUT=48,DIN=40, I2C=41/42, addr=0x30 |
| Amplificateur | **NS4150B** | Ampli analogique classe D | Sortie DAC ES8311 → entrée analogique → haut-parleur |

### 4.2 Broches I2S du codec ES8311

| Signal I2S | GPIO ESP32 | Fonction |
|---|---|---|
| MCLK | **IO39** | Horloge principale |
| BCLK (SCLK) | **IO38** | Horloge binaire |
| LRCLK (WS) | **IO47** | Horloge de canal gauche/droite |
| DOUT (DAC) | **IO48** | Sortie de données série (lecture) |
| DIN (ADC) | **IO40** | Entrée de données série (enregistrement) |

> L'audio utilise le mode I2S0 Standard Duplex ; le codec ES8311 prend en charge simultanément l'enregistrement et la lecture.

### 4.3 Contrôle I2C de l'ES8311

| Signal I2C | GPIO | Description |
|---|---|---|
| SDA | IO41 | Bus I2C partagé avec la caméra |
| SCL | IO42 | Bus I2C partagé avec la caméra |
| Adresse | **0x30** | Adresse I2C 8 bits de l'ES8311 |

---

## 5. Système d'alimentation

### 5.1 Chaîne d'alimentation

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

### 5.2 Composants clés

| Puce | Modèle | Fonction |
|---|---|---|
| LDO | **ME6217C33P5G** (SOT-89-5) | Régulation 5V→3.3V |
| LDO | **ME6211A18M3G** (SOT-23) | 5V→1.8V (cœur caméra) |
| LDO | **ME6211A28M3G** ×2 (SOT-23) | 5V→2.8V (U6=AVDD, U7=DOVDD) |
| Filtrage d'entrée U1 | C14 (10μF) + C15 (0.1μF) | Entrée ME6217C33 |
| Filtrage de sortie U1 | C1 (22μF) | Sortie ME6217C33 |
| Protection contre l'inversion | **NCE3401** P-MOSFET | Protection de l'entrée d'alimentation de la carte de base |
| Résistance de grille | R11 10KΩ | Grille du P-MOS à la masse |

---

## 6. Carte de base et flashage

### 6.1 USB vers série

| Paramètre | Spécification |
|---|---|
| Puce | **CH340K** |
| Interface USB | USB-C (USB1) |
| Connexion série | TXD→U0RXD (IO44), RXD→U0TXD (IO43) (croisées) |
| Résistances d'adaptation USB | R4, R5 22R/1% (en série sur D+/D-) |
| DTR/RTS | Contrôle automatique BOOT + EN pour le flashage en un clic |

### 6.2 Circuit de flashage automatique

Utilise Q1, Q2 (transistors NPN S8050) pour le flashage automatique :

| Signal CH340K | Commande | Broche cible |
|---|---|---|
| DTR | → Q1 → | ESP32 **EN** (CHIP_PU) |
| RTS | → Q2 → | ESP32 **BOOT** (IO0) |

> Principe : l'outil de flashage USB bascule DTR/RTS pour contrôler automatiquement la séquence BOOT et EN, sans appui manuel sur les boutons.

### 6.3 Tableau des broches du connecteur carte principale ↔ carte de base

| Broche carte de base | Broche carte principale | Nom du signal | GPIO ESP32 | Fonction |
|---|---|---|---|---|
| P1-1 | P1-1 | **VDD50** | — | Sortie d'alimentation 5V |
| P1-2 | P1-2 | **VDD50** | — | Sortie d'alimentation 5V |
| P1-3 | P1-3 | **GND** | — | Masse d'alimentation |
| P1-4 | P1-4 | **GND** | — | Masse d'alimentation |
| P1-5 | P1-5 | **SDA** | IO41 | Données I2C (partagées avec caméra/ES8311) |
| P1-6 | P1-6 | **SCL** | IO42 | Horloge I2C (partagée avec caméra/ES8311) |
| P1-7 | P1-7 | **U0TXD** | IO43 | Émission UART0 (vers CH340K RXD) |
| P1-8 | P1-8 | **U0RXD** | IO44 | Réception UART0 (vers CH340K TXD) |
| **P2-1** | P2-1 | **ESP_P** | IO20 | Données différentielles USB positives (D+) |
| **P2-2** | P2-2 | **ESP_N** | IO19 | Données différentielles USB négatives (D-) |
| P2-3 | P2-3 | **VDD33** | — | Sortie d'alimentation 3.3V |
| P2-4 | P2-4 | **VDD33** | — | Sortie d'alimentation 3.3V |
| P2-5 | P2-5 | **GND** | — | Masse d'alimentation |
| P2-6 | P2-6 | **GND** | — | Masse d'alimentation |
| P2-7 | P2-7 | **BOOT** | IO0 | Mode de démarrage / flashage |
| P2-8 | P2-8 | **CHIP_PU** | EN | Contrôle de réinitialisation de la puce |

### 6.4 Interfaces d'extension de la carte de base

| Interface | Signaux | Utilisation |
|---|---|---|
| P3 (I2C, 4 broches) | 5V1, GND, SDA, SCL | Périphériques I2C externes |
| P4 (UART, 4 broches) | 5V1, GND, ESP_P, ESP_N | Périphériques USB/série externes |

---

## 7. Tableau complet d'occupation des GPIO

> **Total** : 33 GPIO occupés, environ 8 GPIO disponibles.

| GPIO | Fonction | Signal | Remarque |
|---|---|---|---|
| **IO0** | BOOT | BOOT | Mode de démarrage / flashage (bouton S2, pull-up 10K + anti-rebond 0.1μF) |
| **IO1** | DVP D2 | CAM_Y4 | Bit 2 des données pixel |
| **IO2** | DVP D1 | CAM_Y3 | Bit 1 des données pixel |
| **IO3** | DVP D3 | CAM_Y5 | Bit 3 des données pixel |
| **IO4** | DVP D0 | CAM_Y2 | Bit 0 des données pixel |
| **IO5** | DVP D4 | CAM_Y6 | Bit 4 des données pixel |
| **IO6** | DVP PCLK | CAM_PCLK | Entrée d'horloge pixel |
| **IO7** | DVP D5 | CAM_Y7 | Bit 5 des données pixel |
| **IO8** | DVP D6 | CAM_Y8 | Bit 6 des données pixel |
| **IO9** | DVP XCLK | CAM_XCLK | Horloge principale caméra (24MHz) |
| **IO10** | DVP D7 | CAM_Y9 | Bit 7 des données pixel |
| **IO11** | DVP HREF | CAM_HREF | Synchronisation de ligne |
| **IO12** | DVP PWDN | CAM_PWDN | Contrôle power-down caméra |
| **IO13** | DVP VSYNC | CAM_VSYNC | Synchronisation de trame |
| **IO14** | DVP RESET | CAM_RESET | Réinitialisation caméra |
| **IO18** | WS2812 | RGB_LED_DIN | Entrée de données de la LED RGB WS2812 |
| **IO19** | USB différentiel | D- (ESP_N) | Données différentielles USB négatives (D-) |
| **IO20** | USB différentiel | D+ (ESP_P) | Données différentielles USB positives (D+) |
| **IO21** | WS2812 DOUT | RGB_LED_DOUT | Sortie en cascade WS2812 (libre si non utilisée) |
| **IO29** | SPI CS | Flash_CS | Sélection de puce de la Flash externe |
| **IO30** | SPI SCLK | Flash_CLK | Horloge de la Flash externe |
| **IO31** | SPI SO | Flash_DO | Sortie de données de la Flash externe |
| **IO32** | SPI SI | Flash_DI | Entrée de données de la Flash externe |
| **IO33** | SPI HOLD | Flash_HOLD | Hold de la Flash externe |
| **IO34** | SPI WP | Flash_WP | Protection en écriture de la Flash externe |
| **IO38** | I2S BCLK | I2S_BCLK | Horloge binaire ES8311 |
| **IO39** | I2S MCLK | I2S_MCLK | Horloge principale ES8311 |
| **IO40** | I2S DIN | I2S_DIN | Entrée de données ADC ES8311 |
| **IO41** | I2C SDA | I2C_SDA | Partagé caméra + ES8311 + I2C externe |
| **IO42** | I2C SCL | I2C_SCL | Partagé caméra + ES8311 + I2C externe |
| **IO43** | UART0 TX | UTXD | Émission UART0 (CH340K de la carte de base) |
| **IO44** | UART0 RX | URXD | Réception UART0 (CH340K de la carte de base) |
| **IO47** | I2S WS | I2S_WS | Sélection de mot ES8311 (LRCK) |
| **IO48** | I2S DOUT | I2S_DOUT | Sortie de données DAC ES8311 |
| **EN** | CHIP_PU | RST/EN | Activation/réinitialisation (bouton S1, pull-up 10K) |

### GPIO disponibles (libres, extensibles)

| GPIO | Caractéristique | Utilisation suggérée |
|---|---|---|
| IO15 | IO générique | Extension |
| IO16 | IO générique | Extension |
| IO17 | IO générique | Extension |
| IO35 | IO générique | Extension |
| IO36 | IO générique | Extension |
| IO37 | IO générique | Extension |
| IO45 | IO générique | Extension |
| IO46 | IO générique | Extension |

> ⚠️ GPIO occupés : 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,18,19,20,21,29,30,31,32,33,34,38,39,40,41,42,43,44,47,48
Nombre disponible : 8 GPIO

---

## 8. Contraintes de conception clés

### 8.1 Configuration du périphérique I2S

- L'ESP32-S3 dispose de deux contrôleurs I2S :
  - **I2S0** : attribué au codec ES8311 (MCLK=IO39, BCLK=IO38, WS=IO47, DOUT=IO48, DIN=IO40), I2S Duplex bidirectionnel, enregistrement et lecture simultanés
- Contrôle I2C de l'ES8311 : SDA=IO41, SCL=IO42, adresse du composant 0x30

### 8.2 Coexistence du DVP et du DMA I2S

- Le DVP utilise le canal DMA du périphérique **LCD_CAM**
- L'ES8311 utilise le DMA I2S0 (une seule interface I2S bidirectionnelle)
- Les deux ne sont pas en conflit, mais attention à la bande passante PSRAM (8MB de PSRAM suffisent)
- GPIO45/46 ne sont pas connectés à l'audio et peuvent servir à d'autres usages ; GPIO47 est utilisé pour I2S WS

### 8.3 Partage du bus I2C

- IO41(SDA) + IO42(SCL) sont connectés à la fois à la caméra et à l'interface I2C externe
- Résistances de pull-up I2C : R12 (SDA, 4.7K/1%), R13 (SCL, 4.7K/1%), tirées vers VDD33
- S'assurer que l'adresse I2C de la caméra n'entre pas en conflit avec les périphériques externes
- Adresse I2C par défaut de l'OV2640 : 0x60 (écriture) / 0x61 (lecture) — pas de conflit avec l'ES8311 (0x30)

### 8.4 Capacité de la Flash suffisante

- **16MB de Flash** offrent un espace confortable, permettant de prendre en charge :
  - Schéma double partition OTA (factory + ota_0 + ota_1)
  - SPIFFS pour l'interface d'administration Web (~1MB)
  - Packs de ressources multilingues XiaoZhi AI (fichiers .p3)
  - Partition de configuration NVS
  - Espace réservé pour les mises à jour
- Aucune inquiétude sur la taille du firmware : les agents IA multiples peuvent être intégralement embarqués

### 8.5 Contraintes des broches d'entrée de la caméra DVP

- D0-D7 ainsi que VSYNC/HREF/PCLK sont des broches d'entrée, pilotées par le capteur caméra
- Lors du développement du pilote, veiller à ce que ces broches soient configurées uniquement en mode INPUT
- Les broches audio I2S (38,39,40,47,48) peuvent être configurées en mode sortie (pris en charge par l'ESP32-S3)

---

## 9. Recommandations de configuration ESP-IDF

### 9.1 Éléments clés du sdkconfig

```
# Puce
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

# Caméra
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

### 9.2 Configuration de la caméra

```c
// Configuration des broches esp32-camera
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
    .pixel_format = PIXFORMAT_RGB565,  // ou PIXFORMAT_JPEG
    .frame_size   = FRAMESIZE_QVGA,    // 320x240
    .jpeg_quality = 12,
    .fb_count     = 2,
    .fb_location  = CAMERA_FB_IN_PSRAM,
    .grab_mode    = CAMERA_GRAB_WHEN_EMPTY,
};
```
