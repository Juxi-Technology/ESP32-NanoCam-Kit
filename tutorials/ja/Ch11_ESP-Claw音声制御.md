# 第11章: ESP-Claw 音声制御

## 本章について

デバイスを `ai_mode:7` に切り替えると、NanoCam は ESP-Claw モードに入ります。XiaoZhi AI(`ai_mode:6`)と**同じファームウェア**(`nanocam_espclaw/`)を共用し、違いは ESP-Claw モードが音声対話に加えて 5 つのハードウェア制御ツールを追加登録する点だけです。

| 比較項目 | XiaoZhi AI (mode 6) | ESP-Claw (mode 7) |
|---|---|---|
| 音声対話 | ✅ ASR→LLM→TTS | ✅ 同じ音声パイプライン |
| LED 制御 | ❌ | ✅ 音声で色調整 / オンオフ |
| AI モード切り替え | ❌ | ✅ 音声で切り替え |
| 撮影 + AI 視覚分析 | ❌ | ✅ 撮影してマルチモーダル AI に画面を理解させる |

## 原理

ESP-Claw モードは音声パイプラインの上に、`RegisterMcpTools()` で 5 つの NanoCam 専用ツールを登録します:

```
ユーザーの音声「ライトを青にして」
  → ASR 音声認識(クラウド)
  → LLM が意図を理解 → self.led.set_color({"r":0, "g":0, "b":255}) を呼び出し
  → NanoCam の WS2812 LED が青に
  → TTS:「はい、ライトを青にしました」
```

## 手順

### 12.1 ファームウェアの書き込み

ESP-Claw は `nanocam_espclaw/` という独立したファームウェアプロジェクトを使用します:

```bash
cd nanocam_espclaw
idf.py set-target esp32s3
idf.py build
idf.py -p COMx flash
```

### 12.2 モード切り替え

起動後に ESP-Claw モードへ設定します:

```
ai_mode:7
```

デバイスは自動再起動後にモードに入ります。`ai_mode:6` で XiaoZhi AI モードに戻せます。

### 12.3 音声制御の例

ウェイクアップ後、そのまま要望を話します:

```
💬 "ライトをつけて"          → WS2812 が白色点灯
💬 "ライトを青にして"        → LED が青に
💬 "ライトを消して"          → LED 消灯
💬 "顔検出モードに切り替えて" → NVS に ai_mode:2 を保存 + 再起動
💬 "ここに何があるか見て"    → 撮影 + マルチモーダル AI 分析
💬 "私の前にカップはある?"   → マルチモーダル AI が画面を認識
```

### 12.4 撮影 + AI 視覚分析

ユーザーが「〜を見て」と話すと、ファームウェアは VGA RGB565 画像を 1 フレーム取得し、JPEG に圧縮して、サーバー側で設定されたマルチモーダル API に送信して分析し、結果を TTS 音声で読み上げます。
> マルチモーダル API の URL と token は、接続ハンドシェイク段階でサーバーから自動配信されるため、シリアルで手動入力する設定コマンドは不要です。

## 5 つの NanoCam 専用ツール

| ツール名 | 機能 | パラメータ |
|---|---|---|
| `self.led.set_color` | WS2812 RGB LED (GPIO18) を設定 | `r,g,b`: 0-255 |
| `self.led.turn_off` | LED を消灯 | なし |
| `self.camera.set_ai_mode` | AI モードを切り替え (NVS 保存 + 再起動) | `mode`: 0-7 |
| `self.camera.inspect_image` | 撮影 + マルチモーダル LLM 視覚分析 | `prompt`: 質問内容 |
| `self.get_device_info` | デバイス情報 JSON | なし |

## 設定ファイル

| 内容 | パス |
|---|---|
| MCP ツール登録 | `nanocam_espclaw/main/boards/nanocam/nanocam_board.cc` |
| Vision 送信ロジック | `nanocam_espclaw/main/boards/common/esp32_camera.cc` |
| SDK デフォルト設定 | `nanocam_espclaw/sdkconfig.defaults` |

> ESP-Claw ファームウェアは独立したプロジェクトで、`nanocam_vision` とはコードを共用しません。2 つのファームウェアはそれぞれビルド・書き込みが必要です。

## どのモードを選ぶか

| あなたのニーズ | 推奨モード |
|---|---|
| 音声チャット・Q&A だけしたい | mode 6 (XiaoZhi) |
| 音声で LED を制御したい | mode 7 (ESP-Claw) |
| 撮影 + AI に画面を「見て」もらいたい | mode 7 (ESP-Claw) |
| 音声で AI 検出モードを切り替えたい | mode 7 (ESP-Claw) |

> ESP-Claw の完全な使用方法(サーバー側設定、カスタム MCP ツール開発など)はまだ探索中で、ドキュメントは研究の進展に合わせて随時更新されます。
