# ESP32-NanoCam MCP 协议手册

> Model Context Protocol (JSON-RPC 2.0) — LLM 与设备的标准通信协议

---

## 一、协议概述

MCP 让 LLM 服务器发现并调用 NanoCam 上的功能(工具)。NanoCam 作为 MCP 服务器,通过 WebSocket 或 MQTT 传输 JSON-RPC 2.0 消息。

### 消息结构

```json
{
  "session_id": "...",
  "type": "mcp",
  "payload": {
    "jsonrpc": "2.0",
    "method": "...",
    "params": {...},
    "id": 1
  }
}
```

---

## 二、交互流程

### 1. 初始化会话

```json
// Server → NanoCam
{
  "jsonrpc": "2.0",
  "method": "initialize",
  "params": {
    "capabilities": {
      "vision": {
        "url": "http://server:8000/vision",
        "token": "xxx"
      }
    }
  },
  "id": 1
}
```

### 2. 获取工具列表

```json
// Server → NanoCam
{"jsonrpc":"2.0","method":"tools/list","params":{"cursor":""},"id":2}

// NanoCam → Server (response)
{
  "jsonrpc":"2.0","id":2,
  "result":{
    "tools":[
      {"name":"self.get_device_status","description":"..."},
      {"name":"self.camera.take_photo","description":"..."}
    ]
  }
}
```

### 3. 调用工具

```json
// Server → NanoCam
{
  "jsonrpc":"2.0","method":"tools/call",
  "params":{"name":"self.camera.take_photo","arguments":{"question":"What is this?"}},
  "id":3
}
```

---

## 三、内置工具

### XiaoZhi AI (ai_mode:6) 通用工具

| 工具名 | 功能 | 参数 |
|--------|------|------|
| `self.get_device_status` | 查询设备状态 | 无 |
| `self.audio_speaker.set_volume` | 设置音量 | volume: 0-100 |
| `self.screen.set_brightness` | 屏幕亮度 | brightness: 0-100 |
| `self.camera.take_photo` | 拍照+AI分析 | question: string |

### ESP-Claw (ai_mode:7) 专用工具

| 工具名 | 功能 | 参数 |
|--------|------|------|
| `self.camera.inspect_image` | **拍照 + 视觉 LLM 分析** (OpenAI Vision API) | prompt: string |
| `self.led.set_color` | 设置 WS2812 RGB LED | r,g,b: 0-255 |
| `self.led.turn_off` | 关闭 LED | 无 |
| `self.camera.set_ai_mode` | 切换 AI 视觉模式 (NVS 保存+重启) | mode: 0-5 |
| `self.get_device_info` | 获取 NanoCam 设备 JSON | 无 |

> ESP-Claw 模式共用 XiaoZhi 语音管线，但额外注册 5 个专用 MCP 工具。
> 工具注册在 `nano_espclaw_register_tools()` 中，先于 `AddCommonTools()` 执行。

---

## 四、自定义工具

```cpp
McpServer::GetInstance().AddTool(
    "self.led.set_color",
    "Set LED color",
    PropertyList({
        Property("r", kPropertyTypeInteger, 0, 255),
        Property("g", kPropertyTypeInteger, 0, 255),
        Property("b", kPropertyTypeInteger, 0, 255)
    }),
    [](const PropertyList& p) -> ReturnValue {
        led_set_rgb(p["r"].value<int>(), p["g"].value<int>(), p["b"].value<int>());
        return true;
    }
);
```

---

## 五、错误码

| 代码 | 含义 |
|:--:|------|
| -32601 | 工具不存在 |
| -32602 | 参数无效 |
| -32603 | 内部错误 |
