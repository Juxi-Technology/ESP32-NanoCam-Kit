# 第11章：ESP-Claw 語音控制

## 關於本章

當裝置切換為 `ai_mode:7` 時，NanoCam 進入 ESP-Claw 模式。它與 XiaoZhi AI（`ai_mode:6`）**共用同一份固件**（`nanocam_espclaw/`），差別僅在於：ESP-Claw 模式在語音對話的基礎上，額外註冊了 5 個硬體控制工具。

| 對比維度 | XiaoZhi AI（mode 6） | ESP-Claw（mode 7） |
|---|---|---|
| 語音對話 | ✅ ASR→LLM→TTS | ✅ 同一條語音管線 |
| LED 控制 | ❌ | ✅ 語音調色 / 開關 |
| AI 模式切換 | ❌ | ✅ 語音切換 |
| 拍照 + AI 視覺分析 | ❌ | ✅ 拍照並呼叫多模態 AI 理解畫面 |

## 原理

ESP-Claw 模式在語音管線之上，透過 `RegisterMcpTools()` 註冊了 5 個 NanoCam 專用工具：

```
使用者語音 "把燈調成藍色"
  → ASR 語音辨識（雲端）
  → LLM 理解意圖 → 呼叫 self.led.set_color({"r":0, "g":0, "b":255})
  → NanoCam WS2812 LED 變藍
  → TTS: "好的，燈已經調成藍色"
```

## 步驟

### 12.1 燒錄固件

ESP-Claw 使用 `nanocam_espclaw/` 獨立固件專案：

```bash
cd nanocam_espclaw
idf.py set-target esp32s3
idf.py build
idf.py -p COMx flash
```

### 12.2 切換模式

啟動後設定為 ESP-Claw 模式：

```
ai_mode:7
```

裝置會自動重啟並進入。使用 `ai_mode:6` 可切回 XiaoZhi AI 模式。

### 12.3 語音控制示例

喚醒後直接說出需求：

```
💬 "把燈打開"              → WS2812 亮白色
💬 "把燈調成藍色"          → LED 變藍
💬 "關燈"                  → LED 關閉
💬 "切換到人臉偵測模式"    → NVS 儲存 ai_mode:2 + 重啟
💬 "看看這裡有什麼"        → 拍照 + 上傳多模態 AI 分析
💬 "我面前有杯子嗎"        → 多模態 AI 辨識畫面
```

### 12.4 拍照 + AI 視覺分析

當使用者說「看看……」時，固件會擷取一張 VGA RGB565 影像，壓縮為 JPEG，傳送到伺服器端設定的多模態 API 進行分析，結果透過 TTS 語音播報。
> 多模態 API 的 URL 和 token 由伺服器端在連線握手階段自動下發，不需要在串列埠手動輸入設定指令。

## 5 個 NanoCam 專用工具

| 工具名稱 | 功能 | 參數 |
|---|---|---|
| `self.led.set_color` | 設定 WS2812 RGB LED（GPIO18） | `r,g,b`：0-255 |
| `self.led.turn_off` | 關閉 LED | 無 |
| `self.camera.set_ai_mode` | 切換 AI 模式（NVS 儲存 + 重啟） | `mode`：0-7 |
| `self.camera.inspect_image` | 拍照 + 多模態 LLM 視覺分析 | `prompt`：問題描述 |
| `self.get_device_info` | 裝置資訊 JSON | 無 |

## 設定檔

| 內容 | 路徑 |
|---|---|
| MCP 工具註冊 | `nanocam_espclaw/main/boards/nanocam/nanocam_board.cc` |
| Vision 發送邏輯 | `nanocam_espclaw/main/boards/common/esp32_camera.cc` |
| SDK 預設設定 | `nanocam_espclaw/sdkconfig.defaults` |

> ESP-Claw 固件是獨立專案，不與 `nanocam_vision` 共用程式碼。兩個固件需分別編譯燒錄。

## 如何選擇

| 你的需求 | 推薦模式 |
|---|---|
| 只想語音聊天、問答 | mode 6（XiaoZhi） |
| 想語音控制 LED | mode 7（ESP-Claw） |
| 想拍照 + AI「看」畫面 | mode 7（ESP-Claw） |
| 想語音切換 AI 偵測模式 | mode 7（ESP-Claw） |

> ESP-Claw 的完整使用方式（伺服器端設定、自訂 MCP 工具開發等）仍在探索中，文件會隨研究進展持續更新。
