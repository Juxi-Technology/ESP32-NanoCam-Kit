# 第12章: MCP设备控制

> 难度: ★★★★ | 模式: ai_mode:6 | 产出: 语音智能家居

## 原理

MCP(Model Context Protocol) 使用 JSON-RPC 2.0 协议,让 LLM 服务器发现和调用设备上的功能(工具)。NanoCam 作为 MCP 服务器,注册可被调用的工具列表,LLM 根据用户意图自动选择合适的工具。

## 步骤

### 12.1 获取工具列表
LLM 服务器发送 `tools/list` 请求,NanoCam 返回所有可用工具。

### 12.2 调用内置工具
`tools/call` → 控制音量、屏幕亮度等。

### 12.3 自定义 MCP 工具
注册新的 GPIO 控制工具:
```cpp
mcp_server.AddTool("self.led.set_color", "设置LED颜色",
    PropertyList({
        Property("r", kPropertyTypeInteger, 0, 255),
        Property("g", kPropertyTypeInteger, 0, 255),
        Property("b", kPropertyTypeInteger, 0, 255)
    }), [](const PropertyList& props) -> ReturnValue {
        int r = props["r"].value<int>();
        int g = props["g"].value<int>();
        int b = props["b"].value<int>();
        // 控制 LED
        return true;
    });
```

### 12.4 语音控制硬件
"把灯调成蓝色"→LLM→MCP→`self.led.set_color`→GPIO。

## 效果
语音命令→LLM理解意图→MCP调用→硬件执行。

## 下一步
→ [第13章: ESP-Claw AI Agent](Ch13_ESP-Claw.md)
