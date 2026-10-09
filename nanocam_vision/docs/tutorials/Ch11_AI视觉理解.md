# 第11章: AI视觉理解

> 难度: ★★★ | 模式: ai_mode:6 | 产出: AI导盲助手

## 原理

NanoCam 拍照→JPEG编码→HTTP上传→LLM服务器分析→返回文字描述。大模型具备多模态视觉理解能力,可以识别物体、颜色、文字、场景等。

## 步骤

### 11.1 拍照+AI解释
在 LLM 模式下,NanoCam 通过 MCP 工具 `self.camera.take_photo` 拍照并发送给服务器:
- 服务器收到 JPEG 图片
- LLM 分析图片内容
- 返回文字描述

### 11.2 提问画面内容
"这是什么?" → AI 描述画面中的物体。
"什么颜色?" → AI 回答颜色信息。

### 11.3 场景理解
"桌子上有几个杯子?" → AI 计数并回答。

## 代码

### MCP 调用示例
```json
{
  "jsonrpc": "2.0",
  "method": "tools/call",
  "params": {
    "name": "self.camera.take_photo",
    "arguments": {"question": "What is in front of me?"}
  },
  "id": 1
}
```

## 效果
对着物体→AI描述→"这是一个红色的杯子,放在木质桌面上"。

## 下一步
→ [第12章: MCP设备控制](Ch12_MCP设备控制.md)
