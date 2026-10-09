# 第27章: 自定义 MCP 工具开发

> 难度: ★★★★ | 产出: 自定义设备控制工具

## 原理

MCP(JSON-RPC 2.0)允许开发者在 NanoCam 固件中注册自定义工具(函数),LLM 自动发现并调用。

## 注册工具

```cpp
// 添加一个控制 LED 的工具
McpServer::GetInstance().AddTool(
    "self.led.set_color",     // 工具名
    "设置LED颜色",              // 描述(LLM用于理解用途)
    PropertyList({             // 参数列表
        Property("r", kPropertyTypeInteger, 0, 255),
        Property("g", kPropertyTypeInteger, 0, 255),
        Property("b", kPropertyTypeInteger, 0, 255)
    }),
    [](const PropertyList& props) -> ReturnValue {
        int r = props["r"].value<int>();
        int g = props["g"].value<int>();
        int b = props["b"].value<int>();
        led_set_color(r, g, b);
        return "OK";
    }
);
```

## 命名规范

- `self.<module>.<action>` 格式
- 描述要清晰,方便 LLM 理解
- 参数类型支持: bool/int/string

## 下一步
→ [第28章: I2C从机协议](Ch28_I2C从机协议.md)
