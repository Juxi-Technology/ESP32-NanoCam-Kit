# 第11章: ESP-Claw 语音控制

## 关于本章

当设备切换为 `ai_mode:7` 时，NanoCam 进入 ESP-Claw 模式。它与 XiaoZhi AI（`ai_mode:6`）**共用同一份固件** (`nanocam_espclaw/`)，差别仅在于：ESP-Claw 模式在语音对话的基础上，额外注册了 5 个硬件控制工具。

| 对比维度 | XiaoZhi AI (mode 6) | ESP-Claw (mode 7) |
|---|---|---|
| 语音对话 | ✅ ASR→LLM→TTS | ✅ 同一条语音管线 |
| LED 控制 | ❌ | ✅ 语音调色 / 开关 |
| AI 模式切换 | ❌ | ✅ 语音切换 |
| 拍照 + AI 视觉分析 | ❌ | ✅ 拍照并调用多模态 AI 理解画面 |

## 原理

ESP-Claw 模式在语音管线之上，通过 `RegisterMcpTools()` 注册了 5 个 NanoCam 专用工具：

```
用户语音 "把灯调成蓝色"
  → ASR 语音识别（云端）
  → LLM 理解意图 → 调用 self.led.set_color({"r":0, "g":0, "b":255})
  → NanoCam WS2812 LED 变蓝
  → TTS: "好的，灯已经调成蓝色"
```

## 步骤

### 12.1 烧录固件

ESP-Claw 使用 `nanocam_espclaw/` 独立固件项目：

```bash
cd nanocam_espclaw
idf.py set-target esp32s3
idf.py build
idf.py -p COMx flash
```

### 12.2 切换模式

启动后设置为 ESP-Claw 模式：

```
ai_mode:7
```

设备自动重启后进入。用 `ai_mode:6` 可切回 XiaoZhi AI 模式。

### 12.3 语音控制示例

唤醒后直接说出需求：

```
💬 "把灯打开"              → WS2812 亮白色
💬 "把灯调成蓝色"          → LED 变蓝
💬 "关灯"                  → LED 关闭
💬 "切换到人脸检测模式"    → NVS 保存 ai_mode:2 + 重启
💬 "看看这里有什么"        → 拍照 + 上传多模态 AI 分析
💬 "我面前有杯子吗"        → 多模态 AI 识别画面
```

### 12.4 拍照 + AI 视觉分析

当用户说"看看..."时，固件抓取一帧 VGA RGB565 图像，压缩为 JPEG，发送到服务端配置的多模态 API 进行分析，结果通过 TTS 语音播报。
> 多模态 API 的 URL 和 token 由服务端在连接握手阶段自动下发，不需要在串口手动输入配置命令。

## 5 个 NanoCam 专用工具

| 工具名 | 功能 | 参数 |
|---|---|---|
| `self.led.set_color` | 设置 WS2812 RGB LED (GPIO18) | `r,g,b`: 0-255 |
| `self.led.turn_off` | 关闭 LED | 无 |
| `self.camera.set_ai_mode` | 切换 AI 模式 (NVS 保存 + 重启) | `mode`: 0-7 |
| `self.camera.inspect_image` | 拍照 + 多模态 LLM 视觉分析 | `prompt`: 问题描述 |
| `self.get_device_info` | 设备信息 JSON | 无 |

## 配置文件

| 内容 | 路径 |
|---|---|
| MCP 工具注册 | `nanocam_espclaw/main/boards/nanocam/nanocam_board.cc` |
| Vision 发送逻辑 | `nanocam_espclaw/main/boards/common/esp32_camera.cc` |
| SDK 默认配置 | `nanocam_espclaw/sdkconfig.defaults` |

> ESP-Claw 固件是独立项目，不与 `nanocam_vision` 共用代码。两个固件需分别编译烧录。

## 如何选择

| 你的需求 | 推荐模式 |
|---|---|
| 只想语音聊天、问答 | mode 6 (XiaoZhi) |
| 想语音控制 LED | mode 7 (ESP-Claw) |
| 想拍照 + AI "看"画面 | mode 7 (ESP-Claw) |
| 想语音切换 AI 检测模式 | mode 7 (ESP-Claw) |

> ESP-Claw 的完整使用方式（服务端配置、自定义 MCP 工具开发等）仍在探索中，文档会随研究进展持续更新。
