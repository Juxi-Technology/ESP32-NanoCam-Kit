# 第13章: ESP-Claw AI Agent

> 难度: ★★★ | 模式: ai_mode:7 | 产出: 拍照+AI视觉理解 + 语音控制

## 原理

ESP-Claw 模式在 NanoCam 上复用了 XiaoZhi AI 成熟的语音管线（ASR→LLM→TTS），同时额外注册了 ESP-Claw 风格的 MCP 工具，实现 **拍照+视觉 LLM 分析** 和智能硬件控制。

### 架构

```
用户语音 "看看有没有苹果"
  → ES8311 麦克风采集
  → ASR 语音识别（云端）
  → LLM 决策调 MCP 工具
  → self.camera.inspect_image({prompt:"有没有苹果?"})
  → 摄像头拍 JPEG → base64 编码 → HTTP POST 多模态 LLM
  → LLM 返回: "没有苹果,但桌上有一个蓝色水杯"
  → TTS 语音播报
```

### 核心能力

| 特性 | 说明 |
|------|------|
| 📸 视觉理解 | 拍照→base64→OpenAI Vision API→语音描述 |
| 💡 语音控灯 | "打开灯光""调成红色"→WS2812 变色 |
| 🔄 模式切换 | "切换到人脸检测"→NVS 保存+自动重启 |
| 🗣️ 语音交互 | 全双工 ASR→LLM→TTS 语音管线 |
| ⚙️ MCP 协议 | JSON-RPC 2.0, LLM 自动发现设备功能 |

### ESP-Claw vs XiaoZhi AI

| 特性 | XiaoZhi AI (mode 6) | ESP-Claw (mode 7) |
|------|-------------------|-------------------|
| 语音交互 | ✅ ASR→LLM→TTS | ✅ 共用同一语音管线 |
| MCP 工具 | 通用工具（音量/亮度/拍照） | **+5 个专用工具**（LED/AI模式/视觉） |
| 视觉理解 | `self.camera.take_photo` (server 端分析) | **`self.camera.inspect_image`** (OpenAI Vision) |
| LED 控制 | ❌ | ✅ `self.led.set_color` |
| AI 模式切换 | ❌ | ✅ `self.camera.set_ai_mode` |
| 适合场景 | 通用 AI 对话、教育 | **硬件控制、视觉巡检、智能家居** |

## 步骤

### 13.1 烧录固件 + 串口配置

```bash
idf.py build flash monitor
```

启动后通过串口配置 LLM 服务器（必须）：

```
sta_ssid:MyWiFi
sta_pd:12345678
nano_server:https://api.openai.com
nano_api_key:sk-xxxxxxxxxxxxxxxx
ai_mode:7
```

> `nano_server` 指向任意 OpenAI 兼容 API（也支持本地 vLLM/Ollama）。
> `nano_api_key` 为 API 密钥。两者都保存在 NVS flash 中，断电不丢失。

### 13.2 首次对话

设备重启，LED 显示紫色（Violet），听到"你好小智"后即可对话：

```
你: "你好小智,看看我面前有没有一个苹果"
NanoCam: (拍照中...) "没有看到苹果。你面前有一台黑色的笔记本电脑和一个白色咖啡杯。"
```

### 13.3 语音控制功能

```
💬 "打开灯光"              → WS2812 亮白光
💬 "把灯调成蓝色"           → LED 变蓝色
💬 "切换到人脸检测模式"      → NVS 保存 ai_mode:2, 自动重启
💬 "看看这里有什么"          → 拍照 + AI 视觉分析
💬 "关灯"                   → LED 关闭
```

### 13.4 视觉理解流程

```
LLM 调 self.camera.inspect_image({prompt})
  → esp_camera_fb_get() 抓帧（VGA RGB565）
  → fmt2jpg() 转 JPEG (quality 80)
  → mbedtls_base64_encode() 编码
  → POST {nano_server}/v1/chat/completions
     {"model":"gpt-4o","messages":[{"role":"user","content":[
       {"type":"text","text":"prompt"},
       {"type":"image_url","image_url":{"url":"data:image/jpeg;base64,..."}}
     ]}]}
  → 解析 choices[0].message.content
  → 返回文字 → TTS 播报
```

### 13.5 MCP 工具清单

| 工具 | 功能 | 参数 |
|------|------|------|
| `self.camera.inspect_image` | **拍照 + 视觉 LLM 分析** | prompt: 问题描述 |
| `self.led.set_color` | 设置 WS2812 RGB LED | r,g,b: 0-255 |
| `self.led.turn_off` | 关闭 LED | 无 |
| `self.camera.set_ai_mode` | 切换 AI 模式(NVS保存+重启) | mode: 0-5 |
| `self.get_device_info` | 获取设备 JSON | 无 |

> 工具在 `nano_espclaw_register_tools()` 中注册，先于 `AddCommonTools()` 执行。

## 代码

### 核心桥接代码

[`main/nano_espclaw.cpp`](../main/nano_espclaw.cpp) — 5 个 MCP 工具的注册：

```cpp
void nano_espclaw_register_tools(void) {
    auto &mcp = McpServer::GetInstance();
    // LED, AI mode switch, device info, inspect_image...
}
```

### 拍照+分析回调（核心逻辑）

```cpp
[](const PropertyList &props) -> ReturnValue {
    // 1. 抓帧
    camera_fb_t *fb = esp_camera_fb_get();
    // 2. 转 JPEG (RGB565 → JPEG)
    fmt2jpg(fb->buf, fb->len, fb->width, fb->height,
            PIXFORMAT_RGB565, 80, &jpeg_buf, &jpeg_len);
    // 3. base64 编码
    std::string b64 = base64_encode(jpeg_buf, jpeg_len);
    // 4. POST OpenAI Vision API
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "model", "gpt-4o");
    // ... build messages[].content[] with text + image_url ...
    esp_http_client_perform(client);
    // 5. 解析 choices[0].message.content → 返回
}
```

### 调用示例

```json
{
  "jsonrpc": "2.0",
  "method": "tools/call",
  "params": {
    "name": "self.camera.inspect_image",
    "arguments": {"prompt": "What color is the object in front of you?"}
  },
  "id": 3
}
```

## 配置文件位置

| 配置项 | 路径 |
|--------|------|
| MCP 工具注册 | [main/nano_espclaw.cpp](../main/nano_espclaw.cpp) |
| API 声明 | [main/nano_espclaw.h](../main/nano_espclaw.h) |
| 模式调度 | [main/app_main.cpp](../main/app_main.cpp) (mode 7) |
| 串口指令 | [main/nano_serial.cpp](../main/nano_serial.cpp) (`nano_server:`/`nano_api_key:`) |
| Kconfig | `menuconfig` → `[*] Enable ESP-Claw Agent` |

## 下一步
→ [第14章: Arduino 控制](Ch14_Arduino控制.md)
