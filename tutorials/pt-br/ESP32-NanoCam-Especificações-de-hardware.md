# Especificações de hardware do ESP32-NanoCam

> Fonte: esquemas NanoCamModule.pdf + NanoCamBASE.pdf.
> Arquitetura: a placa principal (ESP32-S3 + câmera + áudio) encaixa-se na placa base (alimentação USB + gravação via porta serial).

---

## 1. Visão geral do hardware

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

## 2. MCU e armazenamento

### 2.1 MCU principal

| Parâmetro | Especificação |
|---|---|
| Modelo | **ESP32-S3 N16R8** |
| Encapsulamento | QFN56 |
| CPU | Xtensa LX7 dual-core 240MHz |
| PSRAM | **8MB** (Octal SPI, sufixo R8) |
| Flash | **16MB** (sufixo N16, SPI Flash externa de 128M-bit) |
| Clock | Cristal passivo de 40MHz + 2× capacitores de casamento de 10pF |

### 2.2 Armazenamento Flash

| Parâmetro | Especificação |
|---|---|
| Modelo | **GD25Q128ESIG** |
| Capacidade | 128M-bit = **16MB** |
| Interface | SPI (padrão de quatro fios) |
| Fabricante | GigaDevice |

| Sinal Flash | GPIO do ESP32-S3 | Função |
|---|---|---|
| CS | IO29 | Seleção de chip |
| SO (DO) | IO31 | Saída de dados |
| SI (DI) | IO32 | Entrada de dados |
| SCLK | IO30 | Clock |
| WP | IO34 | Proteção contra escrita |
| HOLD | IO33 | Hold |

---

## 3. Subsistema de câmera

### 3.1 Interface DVP

| Parâmetro | Especificação |
|---|---|
| Tipo de interface | FPC 0.5 WS 24P (conector FPC de 24 pinos) |
| Sensores suportados | GC2145 (padrão) / OV2640 / OV5640 e outras câmeras DVP |
| Barramento I2C de configuração | SDA=IO41, SCL=IO42 (compartilhado com o I2C externo) |
| Formato de pixel | RGB565 / JPEG / YUV422 etc. |

### 3.2 Mapeamento completo dos pinos DVP

| Categoria do sinal | Nome do sinal | GPIO do ESP32 | Observações |
|---|---|---|---|
| **Clock do sistema** | XCLK | **IO9** | Saída de clock principal de 24MHz |
| **Clock de pixel** | PCLK | **IO6** | Entrada de clock de pixel da câmera |
| **Sincronismo de quadro** | VSYNC | **IO13** | Sincronismo vertical |
| **Sincronismo de linha** | HREF | **IO11** | Referência horizontal |
| **Controle** | PWDN | **IO12** | Controle de desligamento da câmera |
| **Controle** | RESET | **IO14** | Reset da câmera |
| **I2C** | SDA | **IO41** | Dados I2C da câmera (compartilhado com o ES8311) |
| **I2C** | SCL | **IO42** | Clock I2C da câmera (compartilhado com o ES8311) |
| **Dados D0** | Y2 | **IO4** | Dados de pixel bit0 |
| **Dados D1** | Y3 | **IO2** | Dados de pixel bit1 |
| **Dados D2** | Y4 | **IO1** | Dados de pixel bit2 |
| **Dados D3** | Y5 | **IO3** | Dados de pixel bit3 |
| **Dados D4** | Y6 | **IO5** | Dados de pixel bit4 |
| **Dados D5** | Y7 | **IO7** | Dados de pixel bit5 |
| **Dados D6** | Y8 | **IO8** | Dados de pixel bit6 |
| **Dados D7** | Y9 | **IO10** | Dados de pixel bit7 |

### 3.3 Resumo dos grupos de sinais da câmera

```
Grupo de sinais DVP da câmera:
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

## 4. Subsistema de áudio

### 4.1 Arquitetura de áudio

```
AP2718AT (mic MEMS analógico) → ES8311 Codec (ADC/DAC) → NS4150B (amp. analógico) → alto-falante
                               ↕ I2S (MCLK=39,BCLK=38,WS=47,DOUT=48,DIN=40) + I2C (41/42, addr=0x30)
                            ESP32-S3
```

| Componente | Modelo | Tipo | Interface |
|---|---|---|---|
| Microfone | **AP2718AT** | Microfone MEMS de silício analógico | Analógico diferencial → ADC do ES8311 |
| Codec | **ES8311** | Codec de áudio (I2S+I2C) | I2S=MCLK=39,BCLK=38,WS=47,DOUT=48,DIN=40, I2C=41/42, addr=0x30 |
| Amplificador | **NS4150B** | Amplificador analógico classe D | Saída DAC do ES8311 → entrada analógica → alto-falante |

### 4.2 Pinos I2S do codec ES8311

| Sinal I2S | GPIO do ESP32 | Função |
|---|---|---|
| MCLK | **IO39** | Clock principal |
| BCLK (SCLK) | **IO38** | Clock de bits |
| LRCLK (WS) | **IO47** | Clock dos canais esquerdo/direito |
| DOUT (DAC) | **IO48** | Saída de dados serial (reprodução) |
| DIN (ADC) | **IO40** | Entrada de dados serial (gravação) |

> O áudio usa o modo I2S0 Standard Duplex; o codec ES8311 suporta gravação e reprodução simultâneas.

### 4.3 Controle I2C do ES8311

| Sinal I2C | GPIO | Descrição |
|---|---|---|
| SDA | IO41 | Compartilha o barramento I2C com a câmera |
| SCL | IO42 | Compartilha o barramento I2C com a câmera |
| Endereço | **0x30** | Endereço I2C de 8 bits do ES8311 |

---

## 5. Sistema de alimentação

### 5.1 Cadeia de alimentação

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

### 5.2 Componentes principais

| Chip | Modelo | Função |
|---|---|---|
| LDO | **ME6217C33P5G** (SOT-89-5) | Regulação 5V→3.3V |
| LDO | **ME6211A18M3G** (SOT-23) | 5V→1.8V (núcleo da câmera) |
| LDO | **ME6211A28M3G** ×2 (SOT-23) | 5V→2.8V (U6=AVDD, U7=DOVDD) |
| Filtro de entrada do U1 | C14 (10μF) + C15 (0.1μF) | Entrada do ME6217C33 |
| Filtro de saída do U1 | C1 (22μF) | Saída do ME6217C33 |
| Proteção contra polaridade inversa | **NCE3401** P-MOSFET | Proteção da entrada de alimentação da placa base |
| Resistor de gate | R11 10KΩ | Gate do P-MOS aterrado |

---

## 6. Placa base e gravação

### 6.1 Conversor USB-serial

| Parâmetro | Especificação |
|---|---|
| Chip | **CH340K** |
| Interface USB | USB-C (USB1) |
| Conexão serial | TXD→U0RXD (IO44), RXD→U0TXD (IO43) (cruzadas) |
| Resistores de casamento USB | R4, R5 22R/1% (em série com D+/D-) |
| DTR/RTS | Gravação automática com um clique controla BOOT + EN |

### 6.2 Circuito de gravação com um clique

Usa Q1 e Q2 (transistores NPN S8050) para gravação automática:

| Sinal do CH340K | Controle | Pino de destino |
|---|---|---|
| DTR | → Q1 → | ESP32 **EN** (CHIP_PU) |
| RTS | → Q2 → | ESP32 **BOOT** (IO0) |

> Princípio: a ferramenta de gravação USB alterna DTR/RTS para controlar automaticamente a sequência de BOOT e EN, sem necessidade de pressionar botões.

### 6.3 Tabela de pinos do conector placa principal ↔ placa base

| Pino da placa base | Pino da placa principal | Nome do sinal | GPIO do ESP32 | Função |
|---|---|---|---|---|
| P1-1 | P1-1 | **VDD50** | — | Saída de alimentação 5V |
| P1-2 | P1-2 | **VDD50** | — | Saída de alimentação 5V |
| P1-3 | P1-3 | **GND** | — | Terra de alimentação |
| P1-4 | P1-4 | **GND** | — | Terra de alimentação |
| P1-5 | P1-5 | **SDA** | IO41 | Dados I2C (compartilhado com câmera/ES8311) |
| P1-6 | P1-6 | **SCL** | IO42 | Clock I2C (compartilhado com câmera/ES8311) |
| P1-7 | P1-7 | **U0TXD** | IO43 | Transmissão da serial 0 (para o RXD do CH340K) |
| P1-8 | P1-8 | **U0RXD** | IO44 | Recepção da serial 0 (para o TXD do CH340K) |
| **P2-1** | P2-1 | **ESP_P** | IO20 | Dados diferenciais USB positivo (D+) |
| **P2-2** | P2-2 | **ESP_N** | IO19 | Dados diferenciais USB negativo (D-) |
| P2-3 | P2-3 | **VDD33** | — | Saída de alimentação 3.3V |
| P2-4 | P2-4 | **VDD33** | — | Saída de alimentação 3.3V |
| P2-5 | P2-5 | **GND** | — | Terra de alimentação |
| P2-6 | P2-6 | **GND** | — | Terra de alimentação |
| P2-7 | P2-7 | **BOOT** | IO0 | Modo de boot/gravação |
| P2-8 | P2-8 | **CHIP_PU** | EN | Controle de reset do chip |

### 6.4 Interfaces de expansão da placa base

| Interface | Sinais expostos | Uso |
|---|---|---|
| P3 (I2C, 4Pin) | 5V1, GND, SDA, SCL | Dispositivos I2C externos |
| P4 (UART, 4Pin) | 5V1, GND, ESP_P, ESP_N | Dispositivos USB/seriais externos |

---

## 7. Tabela completa de ocupação de GPIO

> **Total**: 33 GPIOs ocupados, cerca de 8 GPIOs disponíveis.

| GPIO | Função | Sinal | Observações |
|---|---|---|---|
| **IO0** | BOOT | BOOT | Modo de boot/gravação (botão S2, pull-up de 10K + debounce de 0.1μF) |
| **IO1** | DVP D2 | CAM_Y4 | Dados de pixel bit2 |
| **IO2** | DVP D1 | CAM_Y3 | Dados de pixel bit1 |
| **IO3** | DVP D3 | CAM_Y5 | Dados de pixel bit3 |
| **IO4** | DVP D0 | CAM_Y2 | Dados de pixel bit0 |
| **IO5** | DVP D4 | CAM_Y6 | Dados de pixel bit4 |
| **IO6** | DVP PCLK | CAM_PCLK | Entrada de clock de pixel |
| **IO7** | DVP D5 | CAM_Y7 | Dados de pixel bit5 |
| **IO8** | DVP D6 | CAM_Y8 | Dados de pixel bit6 |
| **IO9** | DVP XCLK | CAM_XCLK | Clock principal da câmera (24MHz) |
| **IO10** | DVP D7 | CAM_Y9 | Dados de pixel bit7 |
| **IO11** | DVP HREF | CAM_HREF | Sincronismo de linha |
| **IO12** | DVP PWDN | CAM_PWDN | Controle de desligamento da câmera |
| **IO13** | DVP VSYNC | CAM_VSYNC | Sincronismo de quadro |
| **IO14** | DVP RESET | CAM_RESET | Reset da câmera |
| **IO18** | WS2812 | RGB_LED_DIN | Entrada de dados do LED RGB WS2812 |
| **IO19** | Par diferencial USB | D- (ESP_N) | Dados diferenciais USB negativo (D-) |
| **IO20** | Par diferencial USB | D+ (ESP_P) | Dados diferenciais USB positivo (D+) |
| **IO21** | WS2812 DOUT | RGB_LED_DOUT | Saída em cascata do WS2812 (disponível quando não há cascata) |
| **IO29** | SPI CS | Flash_CS | Seleção de chip da Flash externa |
| **IO30** | SPI SCLK | Flash_CLK | Clock da Flash externa |
| **IO31** | SPI SO | Flash_DO | Saída de dados da Flash externa |
| **IO32** | SPI SI | Flash_DI | Entrada de dados da Flash externa |
| **IO33** | SPI HOLD | Flash_HOLD | Hold da Flash externa |
| **IO34** | SPI WP | Flash_WP | Proteção contra escrita da Flash externa |
| **IO38** | I2S BCLK | I2S_BCLK | Clock de bits do ES8311 |
| **IO39** | I2S MCLK | I2S_MCLK | Clock principal do ES8311 |
| **IO40** | I2S DIN | I2S_DIN | Entrada de dados do ADC do ES8311 |
| **IO41** | I2C SDA | I2C_SDA | Compartilhado entre câmera + ES8311 + I2C externo |
| **IO42** | I2C SCL | I2C_SCL | Compartilhado entre câmera + ES8311 + I2C externo |
| **IO43** | UART0 TX | UTXD | Transmissão da serial 0 (CH340K da placa base) |
| **IO44** | UART0 RX | URXD | Recepção da serial 0 (CH340K da placa base) |
| **IO47** | I2S WS | I2S_WS | Seleção de palavra do ES8311 (LRCK) |
| **IO48** | I2S DOUT | I2S_DOUT | Saída de dados do DAC do ES8311 |
| **EN** | CHIP_PU | RST/EN | Enable/reset (botão S1, pull-up de 10K) |

### GPIOs disponíveis (não utilizados, expansíveis)

| GPIO | Característica | Uso sugerido |
|---|---|---|
| IO15 | IO de uso geral | Expansão |
| IO16 | IO de uso geral | Expansão |
| IO17 | IO de uso geral | Expansão |
| IO35 | IO de uso geral | Expansão |
| IO36 | IO de uso geral | Expansão |
| IO37 | IO de uso geral | Expansão |
| IO45 | IO de uso geral | Expansão |
| IO46 | IO de uso geral | Expansão |

> ⚠️ GPIOs ocupados: 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,18,19,20,21,29,30,31,32,33,34,38,39,40,41,42,43,44,47,48
Quantidade disponível: 8 GPIOs

---

## 8. Restrições de projeto importantes

### 8.1 Configuração do periférico I2S

- O ESP32-S3 tem dois controladores I2S:
  - **I2S0**: atribuído ao codec ES8311 (MCLK=IO39, BCLK=IO38, WS=IO47, DOUT=IO48, DIN=IO40), I2S Duplex bidirecional, com suporte simultâneo a gravação e reprodução
- Controle I2C do ES8311: SDA=IO41, SCL=IO42, endereço do dispositivo 0x30

### 8.2 Coexistência entre DVP e DMA do I2S

- O DVP usa o canal DMA do periférico **LCD_CAM**
- O ES8311 usa o DMA do I2S0 (interface I2S única bidirecional)
- Os dois não entram em conflito, mas atenção à largura de banda da PSRAM (os 8MB de PSRAM são suficientes)
- GPIO45/46 não têm conexão com o áudio e podem ter outros usos; GPIO47 é usado para I2S WS

### 8.3 Compartilhamento do barramento I2C

- IO41(SDA) + IO42(SCL) conectam simultaneamente a câmera e a interface I2C externa
- Resistores de pull-up I2C: R12 (SDA, 4.7K/1%), R13 (SCL, 4.7K/1%), com pull-up para VDD33
- Garanta que o endereço I2C da câmera não conflite com os dispositivos externos
- Endereço I2C padrão do OV2640: 0x60 (escrita) / 0x61 (leitura) — não conflita com o ES8311 (0x30)

### 8.4 Capacidade de Flash suficiente

- **16MB de Flash**: espaço de sobra, com suporte a:
  - Esquema de partição dupla OTA (factory + ota_0 + ota_1)
  - SPIFFS para o painel de administração web (~1MB)
  - Pacotes de recursos multilíngues do XiaoZhi AI (arquivos .p3)
  - Partição de configuração NVS
  - Espaço reservado para atualizações
- Sem preocupação com o tamanho do firmware; a IA multiagente cabe por completo

### 8.5 Restrições dos pinos de entrada da câmera DVP

- D0-D7 e VSYNC/HREF/PCLK são pinos de entrada, controlados pelo sensor da câmera
- Ao desenvolver o driver, garanta que esses pinos sejam configurados apenas como modo INPUT
- Os pinos de áudio I2S (38,39,40,47,48) podem ser configurados como modo de saída (suportado pelo ESP32-S3)

---

## 9. Recomendações de configuração do ESP-IDF

### 9.1 Itens principais do sdkconfig

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

# Câmera
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

# Áudio (ES8311 Codec, I2S0)
# ES8311 I2S: MCLK=39, BCLK=38, WS=47, DOUT=48, DIN=40
# ES8311 I2C: SDA=41, SCL=42, addr=0x30
```

### 9.2 Configuração da câmera

```c
// Configuração de pinos do esp32-camera
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
