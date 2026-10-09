# 第10章：AI 視覺理解

## 關於本章

AI 視覺理解是 **ESP-Claw（模式 7）**的獨有功能，不在 XiaoZhi AI（模式 6）中使用。
> 本章使用的 `self.camera.take_photo` 和 `self.camera.inspect_image` 工具，視覺分析 API 地址由伺服器端在 MCP 握手階段透過 `capabilities.vision` 欄位自動下發。固件端不需要手動設定 API URL —— 這意味著 API 設定在 xiaozhi.me 控制台或自行架設的伺服器端完成，詳情請參考第 11 章：ESP-Claw 語音控制。

## 原理

視覺分析的完整流程：

```
使用者語音 "看看桌上有什麼"
  → ASR 語音辨識
  → LLM 決策：需要拍照分析 → 呼叫 self.camera.take_photo 或 self.camera.inspect_image
  → 固件：esp_camera_fb_get() 抓幀 (VGA RGB565)
  → JPEG 壓縮
  → 透過 Explain() 傳送到伺服器端下發的 Vision API
  → 多模態 LLM 返回文字描述
  → TTS 語音播報
```

### 兩個拍照工具的差異

| 工具 | 用途 | 誰發送 Vision API |
|---|---|---|
| `self.camera.take_photo` | 拍照後用 LLM 內建 vision 能力描述 | 伺服器端 |
| `self.camera.inspect_image`（NanoCam 專用） | 拍照後呼叫 `camera->Explain()` → HTTP POST 到獨立多模態 API | 固件端 |

兩者的差異：`take_photo` 走 XiaoZhi 伺服器端的 LLM 視覺（通用實作），`inspect_image` 是本專案的專用實作，固件直接呼叫獨立的多模態 API（地址由伺服器端下發）。

## 步驟

### 11.1 確認 ESP-Claw 模式

```
ai_mode:7
```

裝置重啟後進入 ESP-Claw 模式。

### 11.2 拍照 + AI 分析

喚醒後直接說出問題：

```
💬 "看看這裡有什麼"
💬 "我面前有杯子嗎"
💬 "這本書是什麼顏色的"
💬 "桌上放了幾個蘋果"
💬 "幫我看看這張紙上面寫了什麼字"
```

NanoCam 會拍照、上傳、分析，然後以語音回答結果。

### 11.3 場景辨識示例

| 語音輸入 | AI 返回示例 |
|---|---|
| 「這是什麼」 | 「這是一台黑色的筆記型電腦，旁邊有一個白色的咖啡杯」 |
| 「有蘋果嗎」 | 「沒有看到蘋果。桌上有兩本書和一支筆」 |
| 「什麼顏色」 | 「你指著的是一個紅色的馬克杯」 |
| 「幾個杯子」 | 「畫面上有 2 個杯子」 |

## 程式碼

### 核心拍照 + 分析回呼

`nanocam_espclaw/main/boards/nanocam/nanocam_board.cc` — MCP 工具註冊：

```cpp
mcp.AddTool("self.camera.inspect_image",
    "Take a photo with the camera and send it to the vision AI for analysis.",
    PropertyList({ Property("prompt", kPropertyTypeString) }),
    [this](const PropertyList &props) -> ReturnValue {
        auto camera = GetCamera();
        if (!camera->Capture()) {
            return std::string("{\"error\":\"Camera capture failed\"}");
        }
        std::string prompt = props["prompt"].value<std::string>();
        return camera->Explain(prompt);
    });
```

`nanocam_espclaw/main/boards/common/esp32_camera.cc` — Explain() 實作：

```cpp
std::string Esp32Camera::Explain(const std::string &question) {
    // explain_url_ 由伺服器端在 MCP 握手時透過 capabilities.vision.url 下發
    // 抓幀 → JPEG 壓縮 → HTTP POST 到多模態 API
    // 返回 LLM 分析結果
}
```

### 伺服器端 MCP 握手（Vision API 下發）

```json
{
  "capabilities": {
    "vision": {
      "url": "https://api.openai.com/v1/chat/completions",
      "token": "sk-..."
    }
  }
}
```

固件收到後會呼叫 `camera->SetExplainUrl(url, token)` 儲存 API 地址，後續呼叫 `inspect_image` 時直接使用。

## 支援的多模態模型

透過伺服器端下發不同的 `vision.url`，可以使用任何 OpenAI 兼容 API：

| 模型 | API 地址示例 | 適用場景 |
|---|---|---|
| `gpt-4o` | `https://api.openai.com/v1/chat/completions` | 綜合能力最強 |
| `gpt-4o-mini` | `https://api.openai.com/v1/chat/completions` | 性價比高 |
| `qwen-vl-max` | `https://dashscope.aliyuncs.com/compatible-mode/v1/chat/completions` | 中文理解更佳 |
| `llava:13b`（Ollama） | `http://localhost:11434/v1/chat/completions` | 完全離線 |
| `claude-fable-5` | 需設定代理 | 詳細場景描述 |

## 效果

「看看這裡有什麼」→ 拍照上傳 → AI 分析 → 語音播報「I see a red cup on a wooden table」 —— 真正的 AI 眼睛。
