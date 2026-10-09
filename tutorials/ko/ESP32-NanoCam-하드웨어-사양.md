# ESP32-S3 NanoCam 하드웨어 사양서

> 출처: NanoCamModule.pdf + NanoCamBASE.pdf 회로도.
> 아키텍처: 코어 보드(ESP32-S3 + 카메라 + 오디오)가 베이스 보드(USB 전원 공급 + 시리얼 플래싱)에 결합됩니다.

---

## 1. 하드웨어 개요

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

## 2. 메인 MCU와 저장소

### 2.1 메인 MCU

| 파라미터 | 사양 |
|---|---|
| 모델 | **ESP32-S3 N16R8** |
| 패키지 | QFN56 |
| CPU | Xtensa LX7 듀얼 코어 240MHz |
| PSRAM | **8MB** (Octal SPI, R8 접미사) |
| Flash | **16MB** (N16 접미사, 외장 128M-bit SPI Flash) |
| 클록 | 40MHz 수정 진동자 + 10pF×2 매칭 커패시터 |

### 2.2 Flash 저장소

| 파라미터 | 사양 |
|---|---|
| 모델 | **GD25Q128ESIG** |
| 용량 | 128M-bit = **16MB** |
| 인터페이스 | SPI (표준 4선) |
| 브랜드 | GigaDevice |

| Flash 신호 | ESP32-S3 GPIO | 기능 |
|---|---|---|
| CS | IO29 | 칩 선택 |
| SO (DO) | IO31 | 데이터 출력 |
| SI (DI) | IO32 | 데이터 입력 |
| SCLK | IO30 | 클록 |
| WP | IO34 | 쓰기 보호 |
| HOLD | IO33 | 홀드 |

---

## 3. 카메라 서브시스템

### 3.1 DVP 인터페이스

| 파라미터 | 사양 |
|---|---|
| 인터페이스 유형 | FPC 0.5 WS 24P (24Pin FPC 커넥터) |
| 지원 Sensor | GC2145(기본) / OV2640 / OV5640 등 DVP 카메라 |
| I2C 구성 버스 | SDA=IO41, SCL=IO42 (외부 I2C와 공유) |
| 픽셀 포맷 | RGB565 / JPEG / YUV422 등 |

### 3.2 DVP 핀 전체 매핑

| 신호 분류 | 신호명 | ESP32 GPIO | 비고 |
|---|---|---|---|
| **시스템 클록** | XCLK | **IO9** | 메인 클록 출력 24MHz |
| **픽셀 클록** | PCLK | **IO6** | 카메라 픽셀 클록 입력 |
| **프레임 동기** | VSYNC | **IO13** | 수직 동기 |
| **행 동기** | HREF | **IO11** | 수평 레퍼런스 |
| **제어** | PWDN | **IO12** | 카메라 전원 차단 제어 |
| **제어** | RESET | **IO14** | 카메라 리셋 |
| **I2C** | SDA | **IO41** | 카메라 I2C 데이터 (ES8311과 공유) |
| **I2C** | SCL | **IO42** | 카메라 I2C 클록 (ES8311과 공유) |
| **데이터 D0** | Y2 | **IO4** | 픽셀 데이터 bit0 |
| **데이터 D1** | Y3 | **IO2** | 픽셀 데이터 bit1 |
| **데이터 D2** | Y4 | **IO1** | 픽셀 데이터 bit2 |
| **데이터 D3** | Y5 | **IO3** | 픽셀 데이터 bit3 |
| **데이터 D4** | Y6 | **IO5** | 픽셀 데이터 bit4 |
| **데이터 D5** | Y7 | **IO7** | 픽셀 데이터 bit5 |
| **데이터 D6** | Y8 | **IO8** | 픽셀 데이터 bit6 |
| **데이터 D7** | Y9 | **IO10** | 픽셀 데이터 bit7 |

### 3.3 카메라 신호 그룹 요약

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

## 4. 오디오 서브시스템

### 4.1 오디오 아키텍처

```
AP2718AT (模拟MEMS麦) → ES8311 Codec (ADC/DAC) → NS4150B (模拟功放) → 扬声器
                               ↕ I2S (MCLK=39,BCLK=38,WS=47,DOUT=48,DIN=40) + I2C (41/42, addr=0x30)
                            ESP32-S3
```

| 부품 | 모델 | 유형 | 인터페이스 |
|---|---|---|---|
| 마이크 | **AP2718AT** | 아날로그 MEMS 실리콘 마이크 | 차동 아날로그 → ES8311 ADC |
| 코덱 | **ES8311** | 오디오 Codec (I2S+I2C) | I2S=MCLK=39,BCLK=38,WS=47,DOUT=48,DIN=40, I2C=41/42, addr=0x30 |
| 앰프 | **NS4150B** | D급 아날로그 앰프 | ES8311 DAC 출력 → 아날로그 입력 → 스피커 |

### 4.2 ES8311 Codec I2S 핀

| I2S 신호 | ESP32 GPIO | 기능 |
|---|---|---|
| MCLK | **IO39** | 메인 클록 |
| BCLK (SCLK) | **IO38** | 비트 클록 |
| LRCLK (WS) | **IO47** | 좌우 채널 클록 |
| DOUT (DAC) | **IO48** | 시리얼 데이터 출력 (재생) |
| DIN (ADC) | **IO40** | 시리얼 데이터 입력 (녹음) |

> 오디오는 I2S0 Standard Duplex 모드를 사용하며, ES8311 Codec은 녹음과 재생을 동시에 지원합니다.

### 4.3 ES8311 I2C 제어

| I2C 신호 | GPIO | 설명 |
|---|---|---|
| SDA | IO41 | 카메라와 I2C 버스 공유 |
| SCL | IO42 | 카메라와 I2C 버스 공유 |
| 주소 | **0x30** | ES8311 8-bit I2C 주소 |

---

## 5. 전원 시스템

### 5.1 전원 공급 체인

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

### 5.2 핵심 부품

| 칩 | 모델 | 기능 |
|---|---|---|
| LDO | **ME6217C33P5G** (SOT-89-5) | 5V→3.3V 레귤레이션 |
| LDO | **ME6211A18M3G** (SOT-23) | 5V→1.8V (카메라 코어) |
| LDO | **ME6211A28M3G** ×2 (SOT-23) | 5V→2.8V (U6=AVDD, U7=DOVDD) |
| U1 입력 필터 | C14 (10μF) + C15 (0.1μF) | ME6217C33 입력 |
| U1 출력 필터 | C1 (22μF) | ME6217C33 출력 |
| 역접속 보호 | **NCE3401** P-MOSFET | 베이스 보드 전원 입력 보호 |
| 게이트 저항 | R11 10KΩ | P-MOS 게이트 접지 |

---

## 6. 베이스 보드와 플래싱

### 6.1 USB-시리얼 변환

| 파라미터 | 사양 |
|---|---|
| 칩 | **CH340K** |
| USB 인터페이스 | USB-C (USB1) |
| 시리얼 연결 | TXD→U0RXD (IO44), RXD→U0TXD (IO43) (교차) |
| USB 매칭 저항 | R4, R5 22R/1% (D+/D- 직렬) |
| DTR/RTS | 원클릭 플래싱 시 BOOT + EN 자동 제어 |

### 6.2 원클릭 다운로드 회로

Q1, Q2 (S8050 NPN 트랜지스터)를 사용해 자동 플래싱을 구현합니다:

| CH340K 신호 | 제어 | 대상 핀 |
|---|---|---|
| DTR | → Q1 → | ESP32 **EN** (CHIP_PU) |
| RTS | → Q2 → | ESP32 **BOOT** (IO0) |

> 원리: USB 플래싱 도구가 DTR/RTS 토글을 통해 BOOT와 EN 타이밍을 자동 제어하므로 수동 버튼 조작이 필요 없습니다.

### 6.3 코어 보드↔베이스 보드 커넥터 핀표

| 베이스 보드 핀 헤더 | 코어 보드 핀 헤더 | 신호명 | ESP32 GPIO | 기능 |
|---|---|---|---|---|
| P1-1 | P1-1 | **VDD50** | — | 5V 전원 출력 |
| P1-2 | P1-2 | **VDD50** | — | 5V 전원 출력 |
| P1-3 | P1-3 | **GND** | — | 전원 접지 |
| P1-4 | P1-4 | **GND** | — | 전원 접지 |
| P1-5 | P1-5 | **SDA** | IO41 | I2C 데이터 (카메라/ES8311과 공유) |
| P1-6 | P1-6 | **SCL** | IO42 | I2C 클록 (카메라/ES8311과 공유) |
| P1-7 | P1-7 | **U0TXD** | IO43 | 시리얼 0 송신 (CH340K RXD에 연결) |
| P1-8 | P1-8 | **U0RXD** | IO44 | 시리얼 0 수신 (CH340K TXD에 연결) |
| **P2-1** | P2-1 | **ESP_P** | IO20 | USB 차동 데이터 정 (D+) |
| **P2-2** | P2-2 | **ESP_N** | IO19 | USB 차동 데이터 부 (D-) |
| P2-3 | P2-3 | **VDD33** | — | 3.3V 전원 출력 |
| P2-4 | P2-4 | **VDD33** | — | 3.3V 전원 출력 |
| P2-5 | P2-5 | **GND** | — | 전원 접지 |
| P2-6 | P2-6 | **GND** | — | 전원 접지 |
| P2-7 | P2-7 | **BOOT** | IO0 | 부팅 모드/플래싱 |
| P2-8 | P2-8 | **CHIP_PU** | EN | 칩 리셋 제어 |

### 6.4 베이스 보드 확장 인터페이스

| 인터페이스 | 신호 | 용도 |
|---|---|---|
| P3 (I2C, 4Pin) | 5V1, GND, SDA, SCL | 외부 I2C 기기 |
| P4 (UART, 4Pin) | 5V1, GND, ESP_P, ESP_N | 외부 USB/시리얼 기기 |

---

## 7. 전체 GPIO 점유표

> **합계**: 33개의 GPIO가 사용 중이며, 사용 가능한 GPIO는 약 8개입니다.

| GPIO | 기능 | 신호 | 비고 |
|---|---|---|---|
| **IO0** | BOOT | BOOT | 부팅 모드/플래싱 (S2 버튼, 10K 풀업 + 0.1μF 디바운스) |
| **IO1** | DVP D2 | CAM_Y4 | 픽셀 데이터 bit2 |
| **IO2** | DVP D1 | CAM_Y3 | 픽셀 데이터 bit1 |
| **IO3** | DVP D3 | CAM_Y5 | 픽셀 데이터 bit3 |
| **IO4** | DVP D0 | CAM_Y2 | 픽셀 데이터 bit0 |
| **IO5** | DVP D4 | CAM_Y6 | 픽셀 데이터 bit4 |
| **IO6** | DVP PCLK | CAM_PCLK | 픽셀 클록 입력 |
| **IO7** | DVP D5 | CAM_Y7 | 픽셀 데이터 bit5 |
| **IO8** | DVP D6 | CAM_Y8 | 픽셀 데이터 bit6 |
| **IO9** | DVP XCLK | CAM_XCLK | 카메라 메인 클록 (24MHz) |
| **IO10** | DVP D7 | CAM_Y9 | 픽셀 데이터 bit7 |
| **IO11** | DVP HREF | CAM_HREF | 행 동기 |
| **IO12** | DVP PWDN | CAM_PWDN | 카메라 전원 차단 제어 |
| **IO13** | DVP VSYNC | CAM_VSYNC | 프레임 동기 |
| **IO14** | DVP RESET | CAM_RESET | 카메라 리셋 |
| **IO18** | WS2812 | RGB_LED_DIN | WS2812 RGB LED 데이터 입력 |
| **IO19** | USB 차동 | D- (ESP_N) | USB 차동 데이터 부 (D-) |
| **IO20** | USB 차동 | D+ (ESP_P) | USB 차동 데이터 정 (D+) |
| **IO21** | WS2812 DOUT | RGB_LED_DOUT | WS2812 캐스케이드 출력 (캐스케이드하지 않을 때 사용 가능) |
| **IO29** | SPI CS | Flash_CS | 외장 Flash 칩 선택 |
| **IO30** | SPI SCLK | Flash_CLK | 외장 Flash 클록 |
| **IO31** | SPI SO | Flash_DO | 외장 Flash 데이터 출력 |
| **IO32** | SPI SI | Flash_DI | 외장 Flash 데이터 입력 |
| **IO33** | SPI HOLD | Flash_HOLD | 외장 Flash Hold |
| **IO34** | SPI WP | Flash_WP | 외장 Flash 쓰기 보호 |
| **IO38** | I2S BCLK | I2S_BCLK | ES8311 비트 클록 |
| **IO39** | I2S MCLK | I2S_MCLK | ES8311 메인 클록 |
| **IO40** | I2S DIN | I2S_DIN | ES8311 ADC 데이터 입력 |
| **IO41** | I2C SDA | I2C_SDA | 카메라 + ES8311 + 외부 I2C 공유 |
| **IO42** | I2C SCL | I2C_SCL | 카메라 + ES8311 + 외부 I2C 공유 |
| **IO43** | UART0 TX | UTXD | 시리얼 0 송신 (베이스 보드 CH340K) |
| **IO44** | UART0 RX | URXD | 시리얼 0 수신 (베이스 보드 CH340K) |
| **IO47** | I2S WS | I2S_WS | ES8311 워드 셀렉트 (LRCK) |
| **IO48** | I2S DOUT | I2S_DOUT | ES8311 DAC 데이터 출력 |
| **EN** | CHIP_PU | RST/EN | 인에이블/리셋 (S1 버튼, 10K 풀업) |

### 사용 가능한 GPIO(미점유, 확장 가능)

| GPIO | 특성 | 권장 용도 |
|---|---|---|
| IO15 | 범용 IO | 확장 |
| IO16 | 범용 IO | 확장 |
| IO17 | 범용 IO | 확장 |
| IO35 | 범용 IO | 확장 |
| IO36 | 범용 IO | 확장 |
| IO37 | 범용 IO | 확장 |
| IO45 | 범용 IO | 확장 |
| IO46 | 범용 IO | 확장 |

> ⚠️ 사용 중인 GPIO: 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,18,19,20,21,29,30,31,32,33,34,38,39,40,41,42,43,44,47,48
사용 가능 수량: 8개 GPIO

---

## 8. 핵심 설계 제약 조건

### 8.1 I2S 주변장치 구성

- ESP32-S3에는 두 개의 I2S 컨트롤러가 있습니다:
  - **I2S0**: ES8311 Codec에 할당됨(MCLK=IO39, BCLK=IO38, WS=IO47, DOUT=IO48, DIN=IO40), 양방향 I2S Duplex, 녹음과 재생을 동시에 지원
- ES8311 I2C 제어: SDA=IO41, SCL=IO42, 소자 주소 0x30

### 8.2 DVP와 I2S DMA 공존

- DVP는 **LCD_CAM** 주변장치의 DMA 채널을 사용합니다
- ES8311은 I2S0 DMA를 사용합니다(단일 I2S 인터페이스 양방향)
- 둘은 충돌하지 않지만 PSRAM 대역폭에 유의해야 합니다(8MB PSRAM이면 충분)
- GPIO45/46은 오디오와 연결되어 있지 않아 다른 용도로 사용할 수 있으며, GPIO47은 I2S WS에 사용됩니다

### 8.3 I2C 버스 공유

- IO41(SDA) + IO42(SCL)은 카메라와 외부 I2C 인터페이스에 동시에 연결됩니다
- I2C 풀업 저항: R12 (SDA, 4.7K/1%), R13 (SCL, 4.7K/1%), VDD33으로 풀업
- 카메라 I2C 주소가 외부 기기와 충돌하지 않도록 해야 합니다
- OV2640 기본 I2C 주소: 0x60 (쓰기) / 0x61 (읽기) — ES8311 (0x30)과 충돌하지 않습니다

### 8.4 Flash 용량 충분

- **16MB Flash**는 공간이 충분하여 다음을 지원할 수 있습니다:
  - OTA 이중 파티션 방식 (factory + ota_0 + ota_1)
  - SPIFFS에 웹 관리자 페이지 저장 (~1MB)
  - 샤오즈 AI 다국어 리소스 패키지 (.p3 파일)
  - NVS 구성 파티션
  - 업그레이드 여유 공간 확보
- 펌웨어 크기를 걱정할 필요 없이 다중 Agent AI를 완전히 포함할 수 있습니다

### 8.5 DVP 카메라 입력 핀 제약

- D0-D7 및 VSYNC/HREF/PCLK는 모두 입력 핀으로 카메라 센서가 구동합니다
- 드라이버 개발 시 이 핀들을 반드시 INPUT 모드로만 구성해야 합니다
- I2S 오디오 핀(38,39,40,47,48)은 출력 모드로 구성할 수 있습니다(ESP32-S3 지원)

---

## 9. ESP-IDF 구성 권장 사항

### 9.1 sdkconfig 주요 항목

```
# 칩
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

# 카메라
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

# 오디오 (ES8311 Codec, I2S0)
# ES8311 I2S: MCLK=39, BCLK=38, WS=47, DOUT=48, DIN=40
# ES8311 I2C: SDA=41, SCL=42, addr=0x30
```

### 9.2 카메라 구성

```c
// esp32-camera 핀 설정
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
    .pixel_format = PIXFORMAT_RGB565,  // 또는 PIXFORMAT_JPEG
    .frame_size   = FRAMESIZE_QVGA,    // 320x240
    .jpeg_quality = 12,
    .fb_count     = 2,
    .fb_location  = CAMERA_FB_IN_PSRAM,
    .grab_mode    = CAMERA_GRAB_WHEN_EMPTY,
};
```
