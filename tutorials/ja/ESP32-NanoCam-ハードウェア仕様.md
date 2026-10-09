# ESP32-S3 NanoCam ハードウェア仕様書

> 出典: NanoCamModule.pdf + NanoCamBASE.pdf の回路図。
> アーキテクチャ: コアボード(ESP32-S3 + カメラ + オーディオ)をベースボード(USB 給電 + シリアル書き込み)に接続する構成です。

---

## 一、ハードウェア概要

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

## 二、メイン MCU とストレージ

### 2.1 メイン MCU

| パラメータ | 仕様 |
|---|---|
| 型番 | **ESP32-S3 N16R8** |
| パッケージ | QFN56 |
| CPU | Xtensa LX7 デュアルコア 240MHz |
| PSRAM | **8MB** (Octal SPI, R8 サフィックス) |
| Flash | **16MB** (N16 サフィックス, 外付け 128M-bit SPI Flash) |
| クロック | 40MHz 水晶振動子 + 10pF×2 マッチングコンデンサ |

### 2.2 Flash ストレージ

| パラメータ | 仕様 |
|---|---|
| 型番 | **GD25Q128ESIG** |
| 容量 | 128M-bit = **16MB** |
| インターフェース | SPI (標準 4 線式) |
| ブランド | GigaDevice |

| Flash 信号 | ESP32-S3 GPIO | 機能 |
|---|---|---|
| CS | IO29 | チップセレクト |
| SO (DO) | IO31 | データ出力 |
| SI (DI) | IO32 | データ入力 |
| SCLK | IO30 | クロック |
| WP | IO34 | 書き込み保護 |
| HOLD | IO33 | ホールド |

---

## 三、カメラサブシステム

### 3.1 DVP インターフェース

| パラメータ | 仕様 |
|---|---|
| インターフェースタイプ | FPC 0.5 WS 24P (24Pin FPC コネクタ) |
| 対応 Sensor | GC2145(デフォルト) / OV2640 / OV5640 などの DVP カメラ |
| I2C 設定バス | SDA=IO41, SCL=IO42 (外部 I2C と共用) |
| ピクセルフォーマット | RGB565 / JPEG / YUV422 など |

### 3.2 DVP ピン全マッピング

| 信号カテゴリ | 信号名 | ESP32 GPIO | 備考 |
|---|---|---|---|
| **システムクロック** | XCLK | **IO9** | マスタークロック出力 24MHz |
| **ピクセルクロック** | PCLK | **IO6** | カメラピクセルクロック入力 |
| **フレーム同期** | VSYNC | **IO13** | 垂直同期 |
| **行同期** | HREF | **IO11** | 水平リファレンス |
| **制御** | PWDN | **IO12** | カメラ電源断制御 |
| **制御** | RESET | **IO14** | カメラリセット |
| **I2C** | SDA | **IO41** | カメラ I2C データ (ES8311 と共用) |
| **I2C** | SCL | **IO42** | カメラ I2C クロック (ES8311 と共用) |
| **データ D0** | Y2 | **IO4** | ピクセルデータ bit0 |
| **データ D1** | Y3 | **IO2** | ピクセルデータ bit1 |
| **データ D2** | Y4 | **IO1** | ピクセルデータ bit2 |
| **データ D3** | Y5 | **IO3** | ピクセルデータ bit3 |
| **データ D4** | Y6 | **IO5** | ピクセルデータ bit4 |
| **データ D5** | Y7 | **IO7** | ピクセルデータ bit5 |
| **データ D6** | Y8 | **IO8** | ピクセルデータ bit6 |
| **データ D7** | Y9 | **IO10** | ピクセルデータ bit7 |

### 3.3 カメラ信号グループ一覧

```
Camera DVP 信号グループ:
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

## 四、オーディオサブシステム

### 4.1 オーディオアーキテクチャ

```
AP2718AT (アナログMEMSマイク) → ES8311 Codec (ADC/DAC) → NS4150B (アナログアンプ) → スピーカー
                               ↕ I2S (MCLK=39,BCLK=38,WS=47,DOUT=48,DIN=40) + I2C (41/42, addr=0x30)
                            ESP32-S3
```

| 部品 | 型番 | タイプ | インターフェース |
|---|---|---|---|
| マイク | **AP2718AT** | アナログ MEMS シリコンマイク | 差動アナログ → ES8311 ADC |
| コーデック | **ES8311** | オーディオ Codec (I2S+I2C) | I2S=MCLK=39,BCLK=38,WS=47,DOUT=48,DIN=40, I2C=41/42, addr=0x30 |
| アンプ | **NS4150B** | D 級アナログアンプ | ES8311 DAC 出力 → アナログ入力 → スピーカー |

### 4.2 ES8311 Codec I2S ピン

| I2S 信号 | ESP32 GPIO | 機能 |
|---|---|---|
| MCLK | **IO39** | マスタークロック |
| BCLK (SCLK) | **IO38** | ビットクロック |
| LRCLK (WS) | **IO47** | 左右チャンネルクロック |
| DOUT (DAC) | **IO48** | シリアルデータ出力 (再生) |
| DIN (ADC) | **IO40** | シリアルデータ入力 (録音) |

> オーディオは I2S0 Standard Duplex モードを使用し、ES8311 Codec は録音と再生を同時にサポートします。

### 4.3 ES8311 I2C 制御

| I2C 信号 | GPIO | 説明 |
|---|---|---|
| SDA | IO41 | カメラと I2C バスを共用 |
| SCL | IO42 | カメラと I2C バスを共用 |
| アドレス | **0x30** | ES8311 8-bit I2C アドレス |

---

## 五、電源システム

### 5.1 給電経路

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

### 5.2 主要部品

| チップ | 型番 | 機能 |
|---|---|---|
| LDO | **ME6217C33P5G** (SOT-89-5) | 5V→3.3V 安定化 |
| LDO | **ME6211A18M3G** (SOT-23) | 5V→1.8V (カメラコア) |
| LDO | **ME6211A28M3G** ×2 (SOT-23) | 5V→2.8V (U6=AVDD, U7=DOVDD) |
| U1 入力フィルタ | C14 (10μF) + C15 (0.1μF) | ME6217C33 入力 |
| U1 出力フィルタ | C1 (22μF) | ME6217C33 出力 |
| 逆接続保護 | **NCE3401** P-MOSFET | ベースボード電源入力保護 |
| ゲート抵抗 | R11 10KΩ | P-MOS ゲート接地 |

---

## 六、ベースボードと書き込み

### 6.1 USB-シリアル変換

| パラメータ | 仕様 |
|---|---|
| チップ | **CH340K** |
| USB インターフェース | USB-C (USB1) |
| シリアル接続 | TXD→U0RXD (IO44), RXD→U0TXD (IO43) (クロス) |
| USB 整合抵抗 | R4, R5 22R/1% (D+/D- 直列) |
| DTR/RTS | ワンクリック書き込みで BOOT + EN を自動制御 |

### 6.2 ワンクリックダウンロード回路

Q1、Q2 (S8050 NPN トランジスタ) を使用して自動書き込みを実現します:

| CH340K 信号 | 制御 | ターゲットピン |
|---|---|---|
| DTR | → Q1 → | ESP32 **EN** (CHIP_PU) |
| RTS | → Q2 → | ESP32 **BOOT** (IO0) |

> 原理: USB 書き込みツールが DTR/RTS のトグルで BOOT と EN のタイミングを自動制御し、手動でのボタン操作が不要になります。

### 6.3 コアボード↔ベースボード コネクタピン表

| ベースボードピンヘッダ | コアボードピンヘッダ | 信号名 | ESP32 GPIO | 機能 |
|---|---|---|---|---|
| P1-1 | P1-1 | **VDD50** | — | 5V 電源出力 |
| P1-2 | P1-2 | **VDD50** | — | 5V 電源出力 |
| P1-3 | P1-3 | **GND** | — | 電源グラウンド |
| P1-4 | P1-4 | **GND** | — | 電源グラウンド |
| P1-5 | P1-5 | **SDA** | IO41 | I2C データ (カメラ/ES8311 と共用) |
| P1-6 | P1-6 | **SCL** | IO42 | I2C クロック (カメラ/ES8311 と共用) |
| P1-7 | P1-7 | **U0TXD** | IO43 | シリアル 0 送信 (CH340K RXD へ) |
| P1-8 | P1-8 | **U0RXD** | IO44 | シリアル 0 受信 (CH340K TXD へ) |
| **P2-1** | P2-1 | **ESP_P** | IO20 | USB 差動データ正 (D+) |
| **P2-2** | P2-2 | **ESP_N** | IO19 | USB 差動データ負 (D-) |
| P2-3 | P2-3 | **VDD33** | — | 3.3V 電源出力 |
| P2-4 | P2-4 | **VDD33** | — | 3.3V 電源出力 |
| P2-5 | P2-5 | **GND** | — | 電源グラウンド |
| P2-6 | P2-6 | **GND** | — | 電源グラウンド |
| P2-7 | P2-7 | **BOOT** | IO0 | ブートモード/書き込み |
| P2-8 | P2-8 | **CHIP_PU** | EN | チップリセット制御 |

### 6.4 ベースボード拡張インターフェース

| インターフェース | 引き出し信号 | 用途 |
|---|---|---|
| P3 (I2C, 4Pin) | 5V1, GND, SDA, SCL | 外部 I2C デバイス |
| P4 (UART, 4Pin) | 5V1, GND, ESP_P, ESP_N | 外部 USB/シリアルデバイス |

---

## 七、完全な GPIO 使用表

> **合計**: 33 個の GPIO が使用中で、使用可能な GPIO は約 8 個です。

| GPIO | 機能 | 信号 | 備考 |
|---|---|---|---|
| **IO0** | BOOT | BOOT | ブートモード/書き込み (S2 ボタン, 10K プルアップ + 0.1μF チャタリング防止) |
| **IO1** | DVP D2 | CAM_Y4 | ピクセルデータ bit2 |
| **IO2** | DVP D1 | CAM_Y3 | ピクセルデータ bit1 |
| **IO3** | DVP D3 | CAM_Y5 | ピクセルデータ bit3 |
| **IO4** | DVP D0 | CAM_Y2 | ピクセルデータ bit0 |
| **IO5** | DVP D4 | CAM_Y6 | ピクセルデータ bit4 |
| **IO6** | DVP PCLK | CAM_PCLK | ピクセルクロック入力 |
| **IO7** | DVP D5 | CAM_Y7 | ピクセルデータ bit5 |
| **IO8** | DVP D6 | CAM_Y8 | ピクセルデータ bit6 |
| **IO9** | DVP XCLK | CAM_XCLK | カメラマスタークロック (24MHz) |
| **IO10** | DVP D7 | CAM_Y9 | ピクセルデータ bit7 |
| **IO11** | DVP HREF | CAM_HREF | 行同期 |
| **IO12** | DVP PWDN | CAM_PWDN | カメラ電源断制御 |
| **IO13** | DVP VSYNC | CAM_VSYNC | フレーム同期 |
| **IO14** | DVP RESET | CAM_RESET | カメラリセット |
| **IO18** | WS2812 | RGB_LED_DIN | WS2812 RGB LED データ入力 |
| **IO19** | USB差分 | D- (ESP_N) | USB 差動データ負 (D-) |
| **IO20** | USB差分 | D+ (ESP_P) | USB 差動データ正 (D+) |
| **IO21** | WS2812 DOUT | RGB_LED_DOUT | WS2812 カスケード出力 (未使用時は利用可能) |
| **IO29** | SPI CS | Flash_CS | 外付け Flash チップセレクト |
| **IO30** | SPI SCLK | Flash_CLK | 外付け Flash クロック |
| **IO31** | SPI SO | Flash_DO | 外付け Flash データ出力 |
| **IO32** | SPI SI | Flash_DI | 外付け Flash データ入力 |
| **IO33** | SPI HOLD | Flash_HOLD | 外付け Flash ホールド |
| **IO34** | SPI WP | Flash_WP | 外付け Flash 書き込み保護 |
| **IO38** | I2S BCLK | I2S_BCLK | ES8311 ビットクロック |
| **IO39** | I2S MCLK | I2S_MCLK | ES8311 マスタークロック |
| **IO40** | I2S DIN | I2S_DIN | ES8311 ADC データ入力 |
| **IO41** | I2C SDA | I2C_SDA | カメラ + ES8311 + 外部 I2C 共用 |
| **IO42** | I2C SCL | I2C_SCL | カメラ + ES8311 + 外部 I2C 共用 |
| **IO43** | UART0 TX | UTXD | シリアル 0 送信 (ベースボード CH340K) |
| **IO44** | UART0 RX | URXD | シリアル 0 受信 (ベースボード CH340K) |
| **IO47** | I2S WS | I2S_WS | ES8311 ワードセレクト (LRCK) |
| **IO48** | I2S DOUT | I2S_DOUT | ES8311 DAC データ出力 |
| **EN** | CHIP_PU | RST/EN | イネーブル/リセット (S1 ボタン, 10K プルアップ) |

### 使用可能な GPIO(未使用、拡張可能)

| GPIO | 特性 | 推奨用途 |
|---|---|---|
| IO15 | 汎用 IO | 拡張 |
| IO16 | 汎用 IO | 拡張 |
| IO17 | 汎用 IO | 拡張 |
| IO35 | 汎用 IO | 拡張 |
| IO36 | 汎用 IO | 拡張 |
| IO37 | 汎用 IO | 拡張 |
| IO45 | 汎用 IO | 拡張 |
| IO46 | 汎用 IO | 拡張 |

> ⚠️ 使用中の GPIO: 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,18,19,20,21,29,30,31,32,33,34,38,39,40,41,42,43,44,47,48
使用可能数: 8 個の GPIO

---

## 八、主要な設計制約

### 8.1 I2S ペリフェラル設定

- ESP32-S3 には 2 つの I2S コントローラがあります:
  - **I2S0**: ES8311 Codec に割り当て済み (MCLK=IO39, BCLK=IO38, WS=IO47, DOUT=IO48, DIN=IO40)、双方向 I2S Duplex、録音と再生を同時にサポート
- ES8311 I2C 制御: SDA=IO41, SCL=IO42, デバイスアドレス 0x30

### 8.2 DVP と I2S DMA の共存

- DVP は **LCD_CAM** ペリフェラルの DMA チャンネルを使用
- ES8311 は I2S0 DMA を使用 (単一 I2S インターフェースの双方向)
- 両者は競合しませんが、PSRAM 帯域に注意が必要 (8MB PSRAM で十分)
- GPIO45/46 はオーディオに未接続のため他の用途に利用可能;GPIO47 は I2S WS に使用

### 8.3 I2C バス共有

- IO41(SDA) + IO42(SCL) はカメラと外部 I2C インターフェースの両方に接続
- I2C プルアップ抵抗: R12 (SDA, 4.7K/1%), R13 (SCL, 4.7K/1%)、VDD33 へプルアップ
- カメラの I2C アドレスが外部デバイスと競合しないことを確認する必要があります
- OV2640 のデフォルト I2C アドレス: 0x60 (書き込み) / 0x61 (読み取り) — ES8311 (0x30) とは競合しません

### 8.4 Flash 容量は十分

- **16MB Flash** は容量に余裕があり、以下をサポートできます:
  - OTA デュアルパーティション構成 (factory + ota_0 + ota_1)
  - SPIFFS に Web 管理バックエンドを格納 (~1MB)
  - 小智 AI 多言語リソースパック (.p3 ファイル)
  - NVS 設定パーティション
  - アップグレード用の予備領域
- ファームウェアサイズを心配する必要はなく、マルチ Agent AI も完全に格納できます

### 8.5 DVP カメラ入力ピンの制約

- D0-D7 および VSYNC/HREF/PCLK はすべて入力ピンで、カメラセンサーが駆動します
- ドライバ開発時は、これらのピンを INPUT モードのみに設定する必要があります
- I2S オーディオピン (38,39,40,47,48) は出力モードに設定可能 (ESP32-S3 が対応)

---

## 九、ESP-IDF 設定の推奨

### 9.1 sdkconfig の主要項目

```
# チップ
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

# カメラ
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

# オーディオ (ES8311 Codec, I2S0)
# ES8311 I2S: MCLK=39, BCLK=38, WS=47, DOUT=48, DIN=40
# ES8311 I2C: SDA=41, SCL=42, addr=0x30
```

### 9.2 カメラ設定

```c
// esp32-camera のピン配置
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
    .pixel_format = PIXFORMAT_RGB565,  // または PIXFORMAT_JPEG
    .frame_size   = FRAMESIZE_QVGA,    // 320x240
    .jpeg_quality = 12,
    .fb_count     = 2,
    .fb_location  = CAMERA_FB_IN_PSRAM,
    .grab_mode    = CAMERA_GRAB_WHEN_EMPTY,
};
```
