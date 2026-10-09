# 第10章: AI 视觉理解

## 关于本章

AI 视觉理解是 **ESP-Claw（模式 7）**的独有功能，不在 XiaoZhi AI（模式 6）中使用。
> 本章使用的 `self.camera.take_photo` 和 `self.camera.inspect_image` 工具，视觉分析 API 地址由服务端在 MCP 握手阶段通过 `capabilities.vision` 字段自动下发。固件端不需要手动配置 API URL —— 这意味着 API 配置在 xiaozhi.me 控制台或自建服务端完成，详情参考 第11章: ESP-Claw 语音控制。

## 原理

视觉分析的完整流程：

```
用户语音 "看看桌上有什么"
  → ASR 语音识别
  → LLM 决策：需要拍照分析 → 调用 self.camera.take_photo 或 self.camera.inspect_image
  → 固件：esp_camera_fb_get() 抓帧 (VGA RGB565)
  → JPEG 压缩
  → 通过 Explain() 发送到服务端下发的 Vision API
  → 多模态 LLM 返回文字描述
  → TTS 语音播报
```

### 两个拍照工具的区别

| 工具 | 用途 | 谁发 Vision API |
|---|---|---|
| `self.camera.take_photo` | 拍照后用 LLM 内置 vision 能力描述 | 服务端 |
| `self.camera.inspect_image` (NanoCam 专用) | 拍照后调用 `camera->Explain()` → HTTP POST 到独立多模态 API | 固件端 |

两者的区别：`take_photo` 走 XiaoZhi 服务端的 LLM 视觉（通用实现），`inspect_image` 是本项目的专用实现，固件直接调用独立的多模态 API（地址由服务端下发）。

## 步骤

### 11.1 确保 ESP-Claw 模式

```
ai_mode:7
```

设备重启后进入 ESP-Claw 模式。

### 11.2 拍照+AI 分析

唤醒后直接说出问题：

```
💬 "看看这里有什么"
💬 "我面前有杯子吗"
💬 "这本书是什么颜色的"
💬 "桌上放了几个苹果"
💬 "帮我看看这张纸上面写了什么字"
```

NanoCam 会拍照、上传、分析，然后语音回答结果。

### 11.3 场景识别示例

| 语音输入 | AI 返回示例 |
|---|---|
| "这是什么" | "这是一台黑色的笔记本电脑，旁边有一个白色的咖啡杯" |
| "有苹果吗" | "没有看到苹果。桌上有两本书和一支笔" |
| "什么颜色" | "你指着的是一个红色的马克杯" |
| "几个杯子" | "画面上有 2 个杯子" |

## 代码

### 核心拍照+分析回调

`nanocam_espclaw/main/boards/nanocam/nanocam_board.cc` — MCP 工具注册:

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

`nanocam_espclaw/main/boards/common/esp32_camera.cc` — Explain() 实现:

```cpp
std::string Esp32Camera::Explain(const std::string &question) {
    // explain_url_ 由服务端在 MCP 握手时通过 capabilities.vision.url 下发
    // 抓帧 → JPEG 压缩 → HTTP POST 到多模态 API
    // 返回 LLM 分析结果
}
```

### 服务端 MCP 握手（Vision API 下发）

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

固件收到后调用 `camera->SetExplainUrl(url, token)` 保存 API 地址，后续 `inspect_image` 调用时直接使用。

## 支持的多模态模型

通过服务端下发不同的 `vision.url`，可以使用任何 OpenAI 兼容 API：

| 模型 | API 地址示例 | 适用场景 |
|---|---|---|
| `gpt-4o` | `https://api.openai.com/v1/chat/completions` | 最强综合能力 |
| `gpt-4o-mini` | `https://api.openai.com/v1/chat/completions` | 性价比高 |
| `qwen-vl-max` | `https://dashscope.aliyuncs.com/compatible-mode/v1/chat/completions` | 中文理解更优 |
| `llava:13b` (Ollama) | `http://localhost:11434/v1/chat/completions` | 完全离线 |
| `claude-fable-5` | 需配置代理 | 详细场景描述 |

## 效果

"看看这里有什么" → 拍照上传 → AI 分析 → 语音播报"I see a red cup on a wooden table" —— 真正的 AI 眼睛。
