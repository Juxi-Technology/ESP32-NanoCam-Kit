# ESP32-NanoCam 用户指南

> 版本：v0.2.0 | 最后更新：2026-06-27
>
> 多 Agent AI 相机完整使用手册：从烧录到运行全部功能。

---

## 目录

1. [项目概述](#1-项目概述)
2. [环境搭建](#2-环境搭建)
3. [编译与烧录](#3-编译与烧录)
4. [WiFi 配网](#4-wifi-配网)
5. [7 种 AI 视觉模式](#5-7-种-ai-视觉模式)
6. [ESP-Claw MCP 工具](#6-esp-claw-mcp-工具)
7. [XiaoZhi AI 语音对话](#7-xiaozhi-ai-语音对话)
8. [ROS2 集成](#8-ros2-集成)
9. [CLI 和桌面工具](#9-cli-和桌面工具)
10. [P3 音频工具](#10-p3-音频工具)
11. [Superpowers 插件系统](#11-superpowers-插件系统)
12. [附录](#12-附录)

---

## 1. 项目概述

ESP32-NanoCam 是一款基于 ESP32-S3 的多 Agent AI 相机平台，集成了 7 种边缘 AI 视觉检测、XiaoZhi AI 语音对话和 ESP-Claw 智能体框架。

### 1.1 硬件规格

| 组件 | 型号 / 规格 |
|------|-------------|
| 主芯片 | ESP32-S3 N16R8 |
| Flash | 16 MB（GD25Q128ESIG） |
| PSRAM | 8 MB |
| 摄像头 | GC2145（默认）/ OV2640 / OV5640（DVP 接口） |
| 音频编解码 | ES8311（I2C 地址 0x30） |
| 音频功放 | NS4150B |
| 麦克风 | AP2718AT |
| RGB LED | WS2812 × 1（GPIO18） |
| USB 转串口 | CH340K（UART0，GPIO43 TX / GPIO44 RX） |
| 固件版本 | v0.2.0 |
| 开发框架 | ESP-IDF v5.4.1 |
| 编程语言 | C++17 |

### 1.2 功能特性

| 类别 | 功能 | 说明 |
|------|------|------|
| AI 视觉 | 模式 0：纯图传 | MJPEG 推流，无 AI 处理 |
| AI 视觉 | 模式 1：猫脸检测 | 基于 CatFaceDetectMN03 模型 |
| AI 视觉 | 模式 2：人脸检测 | 两级流水线 MSR01 + MNP01，10 个关键点 |
| AI 视觉 | 模式 3：颜色检测 | HSV 色彩空间，9 种预设颜色 |
| AI 视觉 | 模式 4：人脸识别 | FaceRecognition112V1S8/S16，注册/辨认/删除 |
| AI 视觉 | 模式 5：二维码识别 | 外部解码器实时解码 |
| 语音 AI | 模式 6：XiaoZhi AI | ASR → LLM → TTS 完整语音流水线 |
| 智能体 | 模式 7：ESP-Claw | 5 个 MCP 工具 + Vision LLM 拍照分析 |
| 通信 | WiFi STA/AP/STA+AP | 2.4GHz b/g/n |
| 通信 | MJPEG HTTP 推流 | `http://<IP>/stream` |
| 通信 | MQTT | 可选，事件发布 |
| 通信 | I2C Slave | 可选，外部 MCU 通信 |
| 通信 | micro-ROS 2 | 可选，发布 `/esp32_img` 和 `/esp32_detect` |

### 1.3 目录结构

```
ESP32-NanoCam/
├── main/
│   ├── app_main.cpp                  # 主入口：多 Agent 调度
│   ├── nano_config.h                 # 硬件引脚定义
│   ├── nano_camera.cpp               # 摄像头驱动
│   ├── nano_wifi.cpp                 # WiFi 管理
│   ├── nano_httpd.cpp                # HTTP 服务器 + MJPEG
│   ├── nano_serial.cpp               # 串口 AT 命令处理
│   ├── nano_espclaw.cpp              # ESP-Claw 桥接 + 5 个 MCP 工具
│   ├── nano_llm.cpp                  # XiaoZhi AI 启动入口
│   ├── nano_ros.cpp                  # micro-ROS 发布
│   ├── nano_mqtt.cpp                 # MQTT 客户端
│   ├── nano_i2c_slave.cpp            # I2C 从机
│   ├── nano_rgb.cpp                  # WS2812 RGB LED 控制
│   ├── ai/
│   │   ├── nano_cat_face_detection.cpp    # 模式 1：猫脸检测
│   │   ├── nano_human_face_detection.cpp  # 模式 2：人脸检测
│   │   ├── nano_color_detection.cpp       # 模式 3：颜色检测
│   │   ├── nano_human_face_recognition.cpp # 模式 4：人脸识别
│   │   ├── nano_motion_detection.cpp      # 运动检测（无独立 ai_mode）
│   │   └── nano_ai_utils.cpp              # AI 工具函数
│   ├── application/                  # XiaoZhi AI 框架
│   ├── audio_codecs/                 # ES8311 驱动
│   ├── protocols/                    # 通信 / 显示 / LED
│   └── boards/nano_cam/              # 板级配置
├── components/                       # ESP-DL / ESP-WHO / CodeScanner
├── tools/
│   ├── nano_cli/nano.py              # CLI 命令行工具
│   ├── nano_desktop/main.py          # PyQt6 桌面应用
│   ├── p3_tools/                     # P3 音频编解码工具集
│   ├── ros_nanocam/                  # ROS2 MediaPipe 节点
│   ├── ros_ai_car/                   # AI 小车控制节点
│   ├── ros_mediapipe/                # 11 个 MediaPipe Demo
│   └── ros_opencv/                   # 21 个 OpenCV Demo
├── libraries/                        # Arduino / Python / STM32 / Micro:bit
├── docs/                             # 28 篇文档 + 28 章教程
└── partitions/                       # 分区表模板（4M / 8M / 16M / 32M）
```

---

## 2. 环境搭建

### 2.1 安装 ESP-IDF v5.4.1

#### Windows（推荐离线安装器）

1. 从 [Espressif 官网](https://docs.espressif.com/projects/esp-idf/en/v5.4.1/esp32s3/getting-started/windows-setup.html) 下载 **ESP-IDF v5.4.1 离线安装器**。
2. 运行安装器，选择目标目录（推荐 `C:\Espressif`）。
3. 安装器会自动配置 Python、Git 和交叉编译工具链。
4. 安装完成后，桌面会出现 `ESP-IDF 5.4 CMD` 和 `ESP-IDF 5.4 PowerShell` 快捷方式。
5. 打开 `ESP-IDF 5.4 PowerShell`，验证安装：

```powershell
idf.py --version
```

应输出类似于 `ESP-IDF v5.4.1` 的信息。

#### Linux

```bash
# 安装依赖
sudo apt-get install git wget flex bison gperf python3 python3-pip \
  python3-venv cmake ninja-build ccache libffi-dev libssl-dev \
  dfu-util libusb-1.0-0

# 克隆 ESP-IDF
mkdir -p ~/esp && cd ~/esp
git clone -b v5.4.1 --recursive https://github.com/espressif/esp-idf.git

# 安装工具链
cd esp-idf
./install.sh esp32s3

# 激活环境（每次打开终端都需要执行）
. ./export.sh
```

建议将 `export.sh` 添加到 `~/.bashrc` 中：

```bash
echo "alias get_idf='. ~/esp/esp-idf/export.sh'" >> ~/.bashrc
source ~/.bashrc
```

#### macOS

```bash
# 安装依赖
brew install cmake ninja dfu-util python3

# 克隆并安装（同 Linux 步骤）
mkdir -p ~/esp && cd ~/esp
git clone -b v5.4.1 --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
./install.sh esp32s3
. ./export.sh
```

### 2.2 安装 Python 依赖

确保 Python 3.8+ 已安装：

```bash
python3 --version
```

### 2.3 克隆项目

```bash
git clone git@github.com:Juxi-Technology/ESP32-NanoCam.git
cd ESP32-NanoCam
```

### 2.4 验证环境

```bash
idf.py --version
# 期望输出: ESP-IDF v5.4.1
```

---

## 3. 编译与烧录

### 3.1 设置目标芯片

```bash
idf.py set-target esp32s3
```

此命令只需执行一次，会写入 `sdkconfig` 文件。

### 3.2 配置项目（可选）

```bash
idf.py menuconfig
```

在弹出的菜单中，以下配置项较为关键：

| 菜单路径 | 设置项 | 说明 |
|----------|--------|------|
| `Serial flasher config` → `Flash size` | 16 MB | 确保 Flash 大小正确 |
| `NanoCam Configuration` → `Camera Model` | GC2145 / OV2640 / OV5640 | 选择摄像头型号 |
| `Component config` → `ESP PSRAM` | 启用 | 必须启用 8MB PSRAM |
| `NanoCam Configuration` → `Enable micro-ROS` | 可选 | 启用 ROS2 桥接 |
| `NanoCam Configuration` → `Enable ESP-Claw` | 可选 | 启用 ESP-Claw MCP 工具 |
| `NanoCam Configuration` → `Enable LLM Agent` | 可选 | 启用 XiaoZhi AI 语音 |

### 3.3 分区表

使用 16MB 分区表 `partitions/16m.csv`：

```bash
# 在 sdkconfig 中或 menuconfig 中设置:
# Partition Table → Custom partition table CSV → partitions/16m.csv
```

分区布局如下：

| 分区名 | 类型 | 子类型 | 偏移 | 大小 | 说明 |
|--------|------|--------|------|------|------|
| nvs | data | nvs | 0x9000 | 16 KB | 非易失存储（WiFi/AI 配置） |
| otadata | data | ota | 0xd000 | 8 KB | OTA 数据 |
| phy_init | data | phy | 0xf000 | 4 KB | PHY 初始化数据 |
| model | data | spiffs | 0x10000 | 960 KB | AI 模型存储（SPIFFS） |
| ota_0 | app | ota_0 | 0x100000 | 6 MB | 固件分区 0 |
| ota_1 | app | ota_1 | 0x700000 | 6 MB | 固件分区 1（OTA 目标） |

### 3.4 编译

```bash
idf.py build
```

首次编译会下载依赖组件（ESP-DL、ESP-WHO 等），耗时较长（约 5-10 分钟）。后续增量编译会快很多。

### 3.5 烧录

将 NanoCam 通过 Type-C 数据线连接到电脑（注意：必须使用数据线，而非仅充电线）。

```bash
idf.py -p COMx flash monitor
```

将 `COMx` 替换为实际端口号：
- Windows：`COM3`、`COM4` 等（在设备管理器中查看"端口 (COM 和 LPT)"）
- Linux：`/dev/ttyUSB0` 或 `/dev/ttyACM0`
- macOS：`/dev/cu.usbserial-*`

**烧录过程：**
1. 如果烧录失败，按住 BOOT 键（GPIO0）不放，再按一下 EN 键，松开 EN，再松开 BOOT，进入手动下载模式。
2. 重新执行 `idf.py -p COMx flash`。
3. 烧录完成后，NanoCam 自动重启，串口输出启动日志。

### 3.6 USB 驱动说明

| USB 芯片 | 适用系统 | 驱动下载 |
|----------|---------|---------|
| CH340K | Windows / Linux / macOS | [WCH 官网](http://www.wch.cn/downloads/CH341SER_EXE.html) |
| CP2102 | Windows / Linux / macOS | [Silicon Labs 官网](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers) |

NanoCam 出厂使用 CH340K，Windows 10/11 通常会自动安装驱动。如果设备管理器中出现感叹号设备，请手动安装上述驱动。

### 3.7 串口参数

| 参数 | 值 |
|------|-----|
| 波特率 | 115200 |
| 数据位 | 8 |
| 校验位 | 无 |
| 停止位 | 1 |
| 流控 | 无 |

---

## 4. WiFi 配网

NanoCam 支持三种 WiFi 模式：**STA**（连接路由器）、**AP**（自发热点）、**AP+STA**（同时工作）。默认出厂模式为 AP+STA。

### 4.1 STA 模式（连接路由器）

#### 方式一：串口 AT 命令

在串口工具（PuTTY、MobaXterm、Arduino 串口监视器等）中发送：

```
nano_wifi:MyWiFi,12345678
```

或者分步设置：

```
sta_ssid:MyWiFi
sta_pd:12345678
```

> **注意**：修改 WiFi 密码后 NanoCam 会自动重启（约 2 秒），重启后新配置生效。

#### 方式二：CLI 工具

```bash
python tools/nano_cli/nano.py COM3 wifi MyWiFi 12345678
```

#### 验证连接

```
sta_ip
```

返回：`sta_ip:192.168.1.100`（示例 IP）

### 4.2 AP 模式（自发热点）

#### 方式一：串口 AT 命令

```
nano_ap:NanoCam-AP,12345678
```

或者分步设置：

```
ap_ssid:NanoCam-AP
ap_pd:12345678
```

#### 方式二：CLI 工具

```bash
python tools/nano_cli/nano.py COM3 ap NanoCam-AP 12345678
```

连接 AP 后，手机/电脑获取 IP 为 `192.168.4.x` 网段。NanoCam AP 地址固定为 `192.168.4.1`。

### 4.3 WiFi 模式切换

```
wifi_mode:0   # AP 模式
wifi_mode:1   # STA 模式
wifi_mode:2   # AP+STA 模式
```

### 4.4 查看 MJPEG 视频流

NanoCam 启动 HTTP 服务器后，浏览器打开：

```
http://<IP地址>/
```

- STA 模式：`http://192.168.1.100/`（替换为实际 IP）
- AP 模式：`http://192.168.4.1/`

或者用 CLI 工具一键打开：

```bash
python tools/nano_cli/nano.py COM3 stream 192.168.1.100
```

### 4.5 WiFi 注意事项

- 仅支持 2.4GHz WiFi，不支持 5GHz。
- WiFi 名称和密码最多 30 个字符，不支持中文。
- 出厂重置：发送 `wifi_reset` 恢复默认设置。

---

## 5. 7 种 AI 视觉模式

NanoCam 启动后默认运行 AI 模式 0（纯图传）。切换模式通过串口 AT 命令或 CLI 工具。

### 5.1 模式切换

#### 串口命令

```
ai_mode:N    # N = 0~7，对应 8 种模式
```

发送后 NanoCam 自动保存到 NVS 并重启，新模式下启动。

#### CLI 工具

```bash
python tools/nano_cli/nano.py COM3 mode N
```

#### AI 模式速查表

| 模式 | 名称 | 摄像头格式 | 说明 |
|:----:|------|-----------|------|
| 0 | Stream（纯图传） | JPEG | MJPEG 推流，无 AI 处理，帧率最高 |
| 1 | Cat Face（猫脸检测） | RGB565 | 实时猫脸检测 + 画框 |
| 2 | Human Face（人脸检测） | RGB565 | 两级流水线人脸检测 + 10 关键点 |
| 3 | Color（颜色检测） | RGB565 | HSV 色彩空间 9 色识别 |
| 4 | Face ID（人脸识别） | RGB565 | 人脸注册/辨认/删除 |
| 5 | QR Code（二维码） | RGB565 | 实时二维码解码 |
| 6 | XiaoZhi AI | JPEG | LLM 语音智能体 |
| 7 | ESP-Claw | JPEG | MCP 工具 + Vision LLM |

> 模式 6 = XiaoZhi AI，模式 7 = ESP-Claw，两者互不混用。

---

### 5.2 模式 0：Stream（纯图传）

| 属性 | 值 |
|------|-----|
| 源文件 | [app_main.cpp](file:///d:/esp32-cam/ESP32-NanoCam/main/app_main.cpp) |
| 摄像头格式 | JPEG |
| 分辨率 | QVGA (320×240) |
| 帧缓冲 | 2 |
| AI 处理 | 无 |
| 输出 | MJPEG HTTP 流 `http://<IP>/` |

最简单的模式，摄像头以 JPEG 格式连续采集，通过 HTTP 服务器 MJPEG 推流到浏览器。延迟最低，帧率最高。

---

### 5.3 模式 1：Cat Face Detection（猫脸检测）

| 属性 | 值 |
|------|-----|
| 源文件 | [nano_cat_face_detection.cpp](file:///d:/esp32-cam/ESP32-NanoCam/main/ai/nano_cat_face_detection.cpp) |
| 模型 | CatFaceDetectMN03 |
| 置信度阈值 | 0.4 |
| IoU 阈值 | 0.3 |
| Top-K | 10 |
| 摄像头格式 | RGB565 |

**工作原理**：摄像头输出 RGB565 帧 → CatFaceDetectMN03 推理 → 检测到猫脸则画矩形框 → 推流到 HTTP/MJPEG。

**串口输出**：检测结果通过 `print_detection_result()` 打印到串口日志，包含猫脸的边界框坐标 `(x1, y1, x2, y2)` 和置信度。

---

### 5.4 模式 2：Human Face Detection（人脸检测）

| 属性 | 值 |
|------|-----|
| 源文件 | [nano_human_face_detection.cpp](file:///d:/esp32-cam/ESP32-NanoCam/main/ai/nano_human_face_detection.cpp) |
| 第一阶段 | HumanFaceDetectMSR01（置信度 0.3, IoU 0.3） |
| 第二阶段 | HumanFaceDetectMNP01（置信度 0.4, IoU 0.3） |
| 关键点 | 10 个（左眼、右眼、鼻子、左嘴角、右嘴角 × 各 2 坐标） |
| 摄像头格式 | RGB565 |

**两阶段流水线**：
1. **第一阶段 (MSR01)**：快速粗检测，生成候选框列表。
2. **第二阶段 (MNP01)**：在候选框基础上精确定位人脸，输出 10 个关键点。

**串口输出格式**：

```
$lx,ly,rx,ry,#
```

- `lx, ly`：人脸左上角坐标
- `rx, ry`：人脸右下角坐标
- 未检测到人脸时输出 `$000,000,320,240,#`

**关键点坐标**（在 `draw_detection_result()` 中绘制）：
- keypoint[0-1]：左眼
- keypoint[2-3]：左嘴角
- keypoint[4-5]：鼻子
- keypoint[6-7]：右眼
- keypoint[8-9]：右嘴角

**I2C 输出**：通过 `set_IIC_data(lx, ly, rx, ry)` 同步输出检测框坐标到 I2C 从机。

---

### 5.5 模式 3：Color Detection（颜色检测）

| 属性 | 值 |
|------|-----|
| 源文件 | [nano_color_detection.cpp](file:///d:/esp32-cam/ESP32-NanoCam/main/ai/nano_color_detection.cpp) |
| 色彩空间 | HSV |
| 预设颜色 | 9 种 |
| 检测器 | ColorDetector |
| 摄像头格式 | RGB565 |

**9 种预设颜色**：

| 编号 | 颜色 | HSV 范围 (H: 0-180, S: 0-255, V: 0-255) | LCD 色值 |
|:----:|------|----------------------------------------|----------|
| 1 | 红色 (red) | H:156-10, S:70-255, V:90-255 | 0x00F8 |
| 2 | 橙色 (orange) | H:11-22, S:70-255, V:90-255 | 0x20FD |
| 3 | 黄色 (yellow) | H:23-33, S:70-255, V:90-255 | 0xE0FF |
| 4 | 绿色 (green) | H:34-75, S:70-255, V:90-255 | 0xE007 |
| 5 | 青色 (cyan) | H:76-96, S:70-255, V:90-255 | 0xFF07 |
| 6 | 蓝色 (blue) | H:97-124, S:70-255, V:90-255 | 0x1F00 |
| 7 | 紫色 (purple) | H:125-155, S:70-255, V:90-255 | 0x1EA1 |
| 8 | 白色 (white) | H:0-180, S:0-40, V:220-255 | 0xFFFF |
| 9 | 灰色 (gray) | H:0-180, S:0-50, V:50-219 | 0x1084 |

**串口输出格式**：与人脸检测相同 `$lx,ly,rx,ry,#`，未检测到输出 `$000,000,320,240,#`。

**I2C 输出**：同步输出检测框坐标。

**虚拟按键操作**：

| 按键 | 功能 |
|------|------|
| `KEY_MENU` | 录入模式 / 检测模式切换 |
| `KEY_PLAY` | 确定录入颜色 / 删除颜色 |
| `KEY_UPUP` | 增加检测框范围 |
| `KEY_DOWN` | 减小检测框范围 |

**使用流程**：
1. 发送 `KEY_MENU` 进入录入模式 → 画面中出现录入框。
2. 将目标颜色对准框内 → 发送 `KEY_PLAY` 确认录入。
3. 发送 `KEY_MENU` 回到检测模式 → 系统开始实时检测。
4. 可发送 `KEY_UPUP` / `KEY_DOWN` 调整检测灵敏度。

---

### 5.6 模式 4：Face Recognition（人脸识别）

| 属性 | 值 |
|------|-----|
| 源文件 | [nano_human_face_recognition.cpp](file:///d:/esp32-cam/ESP32-NanoCam/main/ai/nano_human_face_recognition.cpp) |
| 模型 | FaceRecognition112V1S8 或 FaceRecognition112V1S16 |
| 人脸检测 | MSR01 + MNP01 双阶段 |
| 特征维度 | 112 维 |
| 存储分区 | "fr"（Flash 分区） |
| 摄像头格式 | RGB565 |

**状态机**（`recognizer_state_t`）：

| 状态 | 值 | 说明 |
|------|:--:|------|
| IDLE | 0 | 空闲 |
| DETECT | 1 | 检测模式 |
| ENROLL | 2 | 注册新人脸 |
| RECOGNIZE | 3 | 辨认当前人脸 |
| DELETE | 4 | 删除最后一个人脸 |

**串口输出格式**：

```
@ID:N!
```

- `ID` 为正数表示匹配到已注册人脸
- `ID` 为负数表示未能匹配（陌生人）
- 例如 `@ID:1!` 表示匹配到 ID=1 的用户，`@ID:-1!` 表示未识别

**I2C 输出**：通过 `set_IIC_data_id(id)` 同步输出识别结果 ID。

**虚拟按键操作**：

| 按键 | 功能 |
|------|------|
| `KEY_MENU` | 保存当前检测到的人脸（ENROLL） |
| `KEY_PLAY` | 辨认当前人脸（RECOGNIZE） |
| `KEY_UPUP` | 回到检测模式（DETECT） |
| `KEY_DOWN` | 删除最后一个人脸（DELETE） |

**使用流程**：
1. **注册人脸**：将人脸正对摄像头 → 发送 `KEY_MENU` → 系统注册人脸并分配 ID（ID 从 1 开始递增）。
2. **辨认人脸**：发送 `KEY_PLAY` → 系统识别并返回 `@ID:N!`。
3. **删除人脸**：发送 `KEY_DOWN` → 删除最后注册的人脸。
4. 画面中会显示识别结果（绿色 "ID N" 或红色 "who ?"）。

> **注意**：模式 4 占用内存较大，Type-C 串口可能在此模式下失效。推荐使用扩展串口（PH2.0，GPIO19/20）或 I2C 通信。

---

### 5.7 模式 5：QR Code（二维码识别）

| 属性 | 值 |
|------|-----|
| 源文件 | [app_main.cpp](file:///d:/esp32-cam/ESP32-NanoCam/main/app_main.cpp)（调度） |
| 解码器 | esp-code-scanner（外部组件） |
| 摄像头格式 | RGB565 |

**工作原理**：摄像头帧送入二维码解码器，成功解码后结果通过串口输出。解码结果同时通过 I2C 和串口发出。

**输出内容**：二维码中的原始文本内容。

---

### 5.8 Motion Detection（运动检测，无独立 ai_mode 编号）

| 属性 | 值 |
|------|-----|
| 源文件 | [nano_motion_detection.cpp](file:///d:/esp32-cam/ESP32-NanoCam/main/ai/nano_motion_detection.cpp) |
| 方法 | 帧差法（frame differencing） |
| 帧差阈值 | 8 |
| 触发阈值 | 移动点数 > 50 |
| 摄像头格式 | RGB565 |

**工作原理**：
1. 连续取两帧 RGB565 图像。
2. 调用 `dl::image::get_moving_point_number()` 计算两帧差异。
3. 如果移动点数超过 50，判定为"有运动"。
4. 触发后在画面左上角绘制 20×20 的填充矩形作为指示。

**串口输出**：检测到运动时打印 `"Something moved!"`。

**帧差计算参数**：

```cpp
get_moving_point_number(frame1->buf, frame2->buf, frame1->height, frame1->width, 8, 15);
//                                                                           ^  ^
//                                                             帧差阈值=8   背景过滤阈值=15
```

---

## 6. ESP-Claw MCP 工具

ESP-Claw 模式（`ai_mode:7` 或 `CONFIG_NANOCAM_ENABLE_ESPCLAW=y` 时的模式 7）在 XiaoZhi AI 语音流水线基础上注册了 5 个 MCP（Model Context Protocol）工具。

### 6.1 编译开关

在 `menuconfig` 中启用：

```
NanoCam Configuration → Enable ESP-Claw → 勾选
```

或手动在 `sdkconfig` 中设置：

```
CONFIG_NANOCAM_ENABLE_ESPCLAW=y
```

### 6.2 LLM 服务器配置

ESP-Claw 的 `inspect_image` 工具需要配置 LLM API 服务器。

#### 串口配置

```
nano_server:https://api.openai.com
nano_api_key:sk-your-api-key-here
```

| 指令 | 说明 | 示例 |
|------|------|------|
| `nano_server:URL` | 设置 LLM 服务器地址 | `nano_server:https://api.your-llm.com` |
| `nano_api_key:KEY` | 设置 API 密钥 | `nano_api_key:sk-xxxxxxxx` |

配置保存在 NVS 的 `nanocam` 分区中，键名分别为 `server` 和 `apikey`。

> 支持所有 OpenAI 兼容 API（vLLM / Ollama / 本地模型 / 代理），只要提供符合 `/v1/chat/completions` 协议的端点即可。

### 6.3 5 个 MCP 工具详解

所有工具注册在 [nano_espclaw.cpp](file:///d:/esp32-cam/ESP32-NanoCam/main/nano_espclaw.cpp) 的 `nano_espclaw_register_tools()` 函数中。

#### 工具 1：`self.led.set_color`

| 属性 | 值 |
|------|-----|
| 功能 | 设置 WS2812 RGB LED 颜色 |
| 参数 | r, g, b，整数 0-255 |
| 返回值 | `"ok"` |

控制 GPIO18 连接的 WS2812 RGB LED，每通道 0-255 可调。

#### 工具 2：`self.led.turn_off`

| 属性 | 值 |
|------|-----|
| 功能 | 关闭 RGB LED |
| 参数 | 无 |
| 返回值 | `"ok"` |

#### 工具 3：`self.camera.set_ai_mode`

| 属性 | 值 |
|------|-----|
| 功能 | 切换 AI 视觉模式 |
| 参数 | mode，整数 0-5 |
| 返回值 | `"ok"`（切换后重启） |

将 AI 模式写入 NVS 的 `nanocam` 分区（key: `aimode`），然后调用 `esp_restart()` 重启设备。

#### 工具 4：`self.get_device_info`

| 属性 | 值 |
|------|-----|
| 功能 | 获取设备信息 JSON |
| 参数 | 无 |
| 返回值 | JSON 字符串 |

返回示例：

```json
{
  "device": "NanoCam",
  "chip": "ESP32-S3 N16R8",
  "camera": "GC2145",
  "audio": "ES8311",
  "flash": "16MB",
  "psram": "8MB",
  "framework": "XiaoZhi+ESP-Claw"
}
```

#### 工具 5：`self.camera.inspect_image`

| 属性 | 值 |
|------|-----|
| 功能 | 拍摄照片并发送到 Vision LLM 进行分析 |
| 参数 | prompt，字符串，分析提示词 |
| 返回值 | LLM 的文本分析结果 |

**完整工作流程**（见源码 [nano_espclaw.cpp](file:///d:/esp32-cam/ESP32-NanoCam/main/nano_espclaw.cpp#L276-L337)）：

1. **拍照**：调用 `esp_camera_fb_get()` 从摄像头抓取一帧。
2. **JPEG 转换**：如果原始格式不是 JPEG（如 RGB565），通过 `fmt2jpg()` 转码为 JPEG（质量 80%）。
3. **Base64 编码**：使用 mbedTLS 将 JPEG 数据编码为 Base64。
4. **构建请求**：构建 OpenAI 兼容的 JSON 请求体：

```json
{
  "model": "gpt-4o",
  "max_tokens": 512,
  "messages": [{
    "role": "user",
    "content": [
      {"type": "text", "text": "<prompt>"},
      {"type": "image_url", "image_url": {"url": "data:image/jpeg;base64,<base64>"}}
    ]
  }]
}
```

5. **HTTP POST**：发送到 `{server_url}/v1/chat/completions`（超时 20 秒），附带 `Authorization: Bearer {api_key}` 头。
6. **解析响应**：提取 `choices[0].message.content` 作为分析结果返回。

**使用示例**（LLM 自然语言触发）：

```
用户："摄像头前面是什么？"
→ LLM 调用 self.camera.inspect_image(prompt="摄像头前面是什么？")
→ NanoCam 拍照 → 发送到 Vision LLM → 返回："桌子上有一杯咖啡和一个键盘"
```

---

## 7. XiaoZhi AI 语音对话

XiaoZhi AI 模式（`ai_mode:6` 或 `CONFIG_NANOCAM_ENABLE_LLM=y` 时的模式 6）提供完整的语音对话流水线。

### 7.1 编译开关

```
NanoCam Configuration → Enable LLM Agent → 勾选
```

即：

```
CONFIG_NANOCAM_ENABLE_LLM=y
```

### 7.2 语音流水线

```
麦克风 (AP2718AT) → ES8311 ADC → I2S DIN(GPIO40)
    → Opus 编码 → WebSocket → 云端 ASR
    → 云端 LLM
    → 云端 TTS → WebSocket → Opus 解码
    → I2S DOUT(GPIO48) → ES8311 DAC → NS4150B 功放 → 喇叭
```

入口代码在 [nano_llm.cpp](file:///d:/esp32-cam/ESP32-NanoCam/main/nano_llm.cpp)：

```cpp
void nano_llm_start(void) {
    Board &board = Board::GetInstance();
    board.Initialize();
    Application::GetInstance().Start();  // 阻塞调用
}
```

### 7.3 MCP 服务器

XiaoZhi AI 内置 MCP 服务器（[mcp_server.h](file:///d:/esp32-cam/ESP32-NanoCam/main/mcp_server.h)），兼容 **MCP 2024-11-05 规范**，通过 JSON-RPC 2.0 通信。

**消息结构**：

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

**JSON-RPC 2.0 方法**：

| 方法 | 说明 |
|------|------|
| `initialize` | 初始化会话，交换能力 |
| `tools/list` | 获取所有可用工具列表 |
| `tools/call` | 调用指定工具 |

**通用内置工具**：

| 工具名 | 功能 | 参数 |
|--------|------|------|
| `self.get_device_status` | 查询设备状态 | 无 |
| `self.audio_speaker.set_volume` | 设置音量 | volume: 0-100 |
| `self.screen.set_brightness` | 设置屏幕亮度 | brightness: 0-100 |
| `self.camera.take_photo` | 拍照 + AI 分析 | question: string |

### 7.4 P3 音频格式

XiaoZhi AI 语音传输使用自定义的 **P3 音频格式**：

```
[1B type] [1B reserved] [2B length] [Opus payload]
```

| 参数 | 值 |
|------|-----|
| 头部长度 | 4 字节 |
| 类型字段 | 1 字节（通常为 0） |
| 保留字段 | 1 字节（通常为 0） |
| 数据长度 | 2 字节（大端序） |
| 编码格式 | Opus |
| 采样率 | 16000 Hz |
| 声道 | 单声道 |
| 帧时长 | 60 ms |
| 帧大小 | 960 samples |

### 7.5 音量控制

音量范围 0-100，可通过 MCP 工具或 API 实时调节。

### 7.6 多语言支持

使用 `tools/scripts/gen_lang.py` 生成多语言字符串资源。

---

## 8. ROS2 集成

NanoCam 通过 micro-ROS 将摄像头画面和 AI 检测结果发布到 ROS2 话题。

### 8.1 架构

```
NanoCam (ESP32-S3)                PC (Linux + ROS2)
┌──────────────────┐              ┌──────────────────────┐
│  nano_ros.cpp    │  UDP:9999    │  micro-ros-agent     │
│  /esp32_img      │ ──────────→  │  (Docker)            │
│  /esp32_detect   │              │         ↓            │
└──────────────────┘              │    ROS2 Topics       │
                                  │         ↓            │
                                  │  RViz2 / 自定义节点   │
                                  └──────────────────────┘
```

### 8.2 NanoCam 端配置

**启用 micro-ROS**：`menuconfig` → `NanoCam Configuration` → 勾选 `Enable micro-ROS`，设置 PC IP 地址：

```
CONFIG_NANOCAM_ENABLE_ROS=y
CONFIG_NANOCAM_ROS_AGENT_IP="192.168.1.100"
```

重新编译烧录后，NanoCam 启动时自动连接 micro-ros-agent。

**发布的话题**：

| 话题 | 消息类型 | 说明 |
|------|---------|------|
| `/esp32_img` | sensor_msgs/Image | 摄像头实时画面 |
| `/esp32_detect` | std_msgs/String | AI 检测结果文本 |

### 8.3 PC 端启动

```bash
# 启动 micro-ros-agent (Humble)
docker run -it --rm --net=host \
  microros/micro-ros-agent:humble udp4 --port 9999 -v4

# 验证连接
ros2 topic list
# 应看到: /esp32_img /esp32_detect /parameter_events

# 查看数据
ros2 topic echo /esp32_detect
ros2 topic hz /esp32_img
```

### 8.4 RViz2 可视化

1. 启动 `rviz2`。
2. 添加 `Image` 显示，topic 选 `/esp32_img`。
3. `Fixed Frame` 设为 `esp32_cam_frame`。

### 8.5 NanoCam ROS2 节点

`tools/ros_nanocam/` 目录下的 ROS2 节点（基于 MediaPipe）：

| 节点 | 文件 | 输入话题 | 输出话题 | 功能 |
|------|------|---------|---------|------|
| 图像查看器 | [nanocam_img_viewer.py](file:///d:/esp32-cam/ESP32-NanoCam/tools/ros_nanocam/nanocam_img_viewer.py) | `/esp32_img` | — | OpenCV 实时显示 |
| 手部检测 | [nanocam_hand_detector.py](file:///d:/esp32-cam/ESP32-NanoCam/tools/ros_nanocam/nanocam_hand_detector.py) | `/esp32_img` | `/nanocam_gesture` | MediaPipe 21 点手部追踪 |
| 面部网格 | [nanocam_face_mesh.py](file:///d:/esp32-cam/ESP32-NanoCam/tools/ros_nanocam/nanocam_face_mesh.py) | `/esp32_img` | — | MediaPipe 468 点面部网格 |
| 姿态检测 | [nanocam_pose_detector.py](file:///d:/esp32-cam/ESP32-NanoCam/tools/ros_nanocam/nanocam_pose_detector.py) | `/esp32_img` | `/nanocam_pose` | 人体 33 关键点 |
| 3D 物体 | [nanocam_objectron.py](file:///d:/esp32-cam/ESP32-NanoCam/tools/ros_nanocam/nanocam_objectron.py) | `/esp32_img` | — | 3D 物体检测 |
| 手势控制 | [nanocam_gesture_ctrl.py](file:///d:/esp32-cam/ESP32-NanoCam/tools/ros_nanocam/nanocam_gesture_ctrl.py) | `/esp32_img` | — | 手势 → 舵机控制 |

### 8.6 ROS2 AI 小车

`tools/ros_ai_car/` 目录提供 AI 小车控制节点：

| 节点 | 文件 | 功能 |
|------|------|------|
| 人脸跟踪 | [face_fllow.py](file:///d:/esp32-cam/ESP32-NanoCam/tools/ros_ai_car/face_fllow.py) | 人脸跟踪云台，PID 控制器 |
| 颜色跟踪 | [colorTracker.py](file:///d:/esp32-cam/ESP32-NanoCam/tools/ros_ai_car/colorTracker.py) | HSV 颜色追踪 |
| 手势控制 | [HandCtrl.py](file:///d:/esp32-cam/ESP32-NanoCam/tools/ros_ai_car/HandCtrl.py) | 手势 → 小车运动 |
| 巡线 | [follow_line.py](file:///d:/esp32-cam/ESP32-NanoCam/tools/ros_ai_car/follow_line.py) | 视觉巡线 |

### 8.7 MediaPipe & OpenCV Demo

- `tools/ros_mediapipe/` — 11 个 MediaPipe 教程脚本（手部、姿态、面部、3D 物体、虚拟画笔、手势识别等）。
- `tools/ros_opencv/` — 21 个 OpenCV 教程脚本。

### 8.8 故障排除

| 问题 | 解决 |
|------|------|
| topic 列表为空 | 检查 NanoCam 和 PC 是否同一网段 |
| 图像不显示 | RViz2 Fixed Frame 设为 `esp32_cam_frame` |
| Agent 连接失败 | Docker 是否用了 `--net=host` |
| 端口冲突 | 9999 端口不能有其他程序占用 |

---

## 9. CLI 和桌面工具

### 9.1 nano.py CLI

`tools/nano_cli/nano.py` 是命令行控制工具，需安装 `pyserial`：

```bash
pip install pyserial
```

#### 命令速查表

| 命令 | 用法 | 说明 |
|------|------|------|
| `info` | `nano.py COM3 info` | 设备状态 JSON |
| `wifi` | `nano.py COM3 wifi SSID PASS` | 配置 STA WiFi（触发重启） |
| `ap` | `nano.py COM3 ap SSID PASS` | 配置 AP 热点（触发重启） |
| `mode` | `nano.py COM3 mode N` | 切换 AI 模式 0-7（触发重启） |
| `stream` | `nano.py COM3 stream [IP]` | 浏览器打开 MJPEG 图传 |
| `monitor` | `nano.py COM3 monitor` | 交互式串口监视器 |
| `ip` | `nano.py COM3 ip` | 查询 STA IP 地址 |
| `ver` | `nano.py COM3 ver` | 查固件版本 |
| `reboot` | `nano.py COM3 reboot` | 软复位 |
| `reset` | `nano.py COM3 reset` | 恢复出厂设置（触发重启） |
| `send` | `nano.py COM3 send CMD` | 发送任意 AT 命令 |

#### `info` 输出示例

```
Device:    NanoCam
Version:   0.2.0
AI Mode:   2 (HumanFace)
WiFi Mode: 2
STA IP:    192.168.1.100
Free Heap: 245 KB
```

底层的 `nano_info` 串口命令返回完整 JSON：

```json
{"device":"NanoCam","ver":"0.2.0","chip":"ESP32-S3","flash":"16MB","psram":"8MB","ai_mode":2,"wifi_mode":2,"sta_ip":"192.168.1.100","free_heap":245760}
```

#### `monitor` 模式

进入交互式串口监视器后可直接输入 AT 命令：

```
ai_mode:1
sta_ip
nano_info
```

按 `Ctrl+C` 退出。

### 9.2 nano_desktop 桌面应用

基于 PyQt6 + OpenCV 的跨平台桌面客户端（[main.py](file:///d:/esp32-cam/ESP32-NanoCam/tools/nano_desktop/main.py)）。

**安装依赖**：

```bash
pip install pyserial PyQt6 opencv-python numpy
```

**启动**：

```bash
python tools/nano_desktop/main.py
```

**功能特性**：

- **实时 MJPEG 视频流**：通过 OpenCV `VideoCapture` 拉取 HTTP 流，PyQt6 QPixmap 渲染显示。
- **AI 模式切换**：下拉菜单选择 7 种 AI 模式（Stream / Cat Face / Human Face / Color / Face ID / QR Code / LLM Agent），点击即切换。
- **串口命令输入**：底部命令输入框 + 日志输出面板。
- **串口连接管理**：端口输入 → Connect 按钮。

UI 分为左面板（控制区：端口连接、AI 模式、命令输入）和右侧大画布（MJPEG 视频流实时显示）。

---

## 10. P3 音频工具

`tools/p3_tools/` 是 XiaoZhi AI 语音流水线使用的 P3 音频格式编解码工具集。

### 10.1 P3 格式规范

```
[1B type] [1B reserved] [2B length (大端)] [Opus payload]
```

| 参数 | 值 |
|------|-----|
| 编码 | Opus |
| 采样率 | 16000 Hz |
| 声道 | 单声道 |
| 帧时长 | 60 ms |
| 帧大小 | 960 samples (int16) |

### 10.2 工具概览

| 工具 | 文件 | 功能 |
|------|------|------|
| 音频 → P3 | [convert_audio_to_p3.py](file:///d:/esp32-cam/ESP32-NanoCam/tools/p3_tools/convert_audio_to_p3.py) | 将 WAV/MP3 等音频转为 P3 |
| P3 → 音频 | [convert_p3_to_audio.py](file:///d:/esp32-cam/ESP32-NanoCam/tools/p3_tools/convert_p3_to_audio.py) | 将 P3 文件解码为 WAV |
| P3 播放器 | [play_p3.py](file:///d:/esp32-cam/ESP32-NanoCam/tools/p3_tools/play_p3.py) | 命令行 P3 播放器 |
| 批量转换 | [batch_convert_gui.py](file:///d:/esp32-cam/ESP32-NanoCam/tools/p3_tools/batch_convert_gui.py) | 批量转换 GUI |
| GUI 播放器 | [p3_gui_player.py](file:///d:/esp32-cam/ESP32-NanoCam/tools/p3_tools/p3_gui_player.py) | 图形界面 P3 播放器 |

### 10.3 音频转 P3

```bash
python tools/p3_tools/convert_audio_to_p3.py input.wav output.p3
```

**处理流程**（见 [convert_audio_to_p3.py](file:///d:/esp32-cam/ESP32-NanoCam/tools/p3_tools/convert_audio_to_p3.py#L11-L49)）：

1. 使用 `librosa` 加载音频文件。
2. 如果立体声，转换到单声道。
3. 如果采样率非 16000Hz，重采样到 16000Hz。
4. 可选的响度归一化（LUFS=-16）。
5. 以 60ms 为一帧，用 `opuslib` 编码为 Opus 数据包。
6. 每帧写入 4 字节头部（type=0, reserved=0, length）+ Opus 数据。

**参数选项**：

| 参数 | 说明 |
|------|------|
| `-d` | 禁用响度自动调节（TTS 音频建议使用） |
| `--target-lufs` | 目标响度值（默认 -16 LUFS） |

### 10.4 P3 转音频

```bash
python tools/p3_tools/convert_p3_to_audio.py input.p3 output.wav
```

**处理流程**：

1. 流式读取 P3 文件，逐帧解析头部。
2. 提取 Opus 数据，Opus 解码为 PCM int16。
3. 拼接所有 PCM 帧，使用 `soundfile` 写入 WAV（16000Hz, int16, 单声道）。

### 10.5 P3 播放

```bash
python tools/p3_tools/play_p3.py input.p3
```

使用 `sounddevice` 实时播放，支持淡入淡出效果。

**依赖安装**：

```bash
pip install opuslib numpy sounddevice soundfile librosa pyloudnorm tqdm
```

---

## 11. Superpowers 插件系统

本项目使用 **Claude Code Superpowers** 插件系统来扩展 AI 编码能力。

### 11.1 安装与启用

```bash
# 添加插件市场
/plugin marketplace add forrestchang/andrej-karpathy-skills

# 安装 Andrej Karpathy 编码指南 Superpower
/plugin install andrej-karpathy-skills@karpathy-guidelines
```

| Superpower | 用途 | 状态 |
|-----------|------|:--:|
| `karpathy-guidelines` | LLM 编码行为规范，减少常见错误 | ✅ 已集成 |

### 11.2 Andrej Karpathy 编码四原则

#### 原则 1：编码前思考

> **不要假设。不要隐藏困惑。呈现权衡。**

- 明确说明假设。如果不确定，提问。
- 如果存在多种理解，全部呈现 —— 不要默默选择一个。
- 如果存在更简单的方法，指出来。在必要时提出异议。
- 如果有不清楚的地方，停下来，说出困惑之处，提出问题。

#### 原则 2：简洁优先

> **用最少的代码解决问题。不做任何推测性开发。**

- 不添加需求之外的功能。
- 不为一次性代码创建抽象。
- 不添加未被要求的"灵活性"或"可配置性"。
- 不为不可能发生的场景做错误处理。
- 自我检查："资深工程师会觉得这太复杂吗？"如果是，简化它。

#### 原则 3：精准修改

> **只碰必须碰的。只清理自己造成的混乱。**

- 不要"改进"相邻的代码、注释或格式化。
- 不要重构没有坏的东西。
- 匹配现有代码风格，即使你更倾向于不同写法。
- 注意到的无关死代码可以提出来，但不要擅自删除。
- 每一行改动都应该能直接追溯到用户的请求。

#### 原则 4：目标驱动执行

> **定义成功标准。循环直到验证通过。**

| 弱描述 | 强描述 |
|--------|--------|
| "添加验证" | "为无效输入编写测试，然后让它们通过" |
| "修复 bug" | "编写重现 bug 的测试，然后让它通过" |
| "重构 X" | "确保重构前后测试都能通过" |

多步骤任务需先出简短计划：

```
1. [步骤] → 验证: [检查]
2. [步骤] → 验证: [检查]
3. [步骤] → 验证: [检查]
```

---

## 12. 附录

### 12.1 完整引脚定义表

#### DVP 摄像头接口

| 信号 | GPIO | 说明 |
|------|:----:|------|
| XCLK | 9 | 摄像头主时钟 24MHz |
| PCLK | 6 | 像素时钟 |
| VSYNC | 13 | 垂直同步 |
| HREF | 11 | 水平参考 |
| D0 (Y2) | 4 | 数据位 0 |
| D1 (Y3) | 2 | 数据位 1 |
| D2 (Y4) | 1 | 数据位 2 |
| D3 (Y5) | 3 | 数据位 3 |
| D4 (Y6) | 5 | 数据位 4 |
| D5 (Y7) | 7 | 数据位 5 |
| D6 (Y8) | 8 | 数据位 6 |
| D7 (Y9) | 10 | 数据位 7 |
| PWDN | 12 | 摄像头断电控制 |
| RESET | 14 | 摄像头复位 |

#### I2C 总线（摄像头 SCCB + ES8311 共用）

| 信号 | GPIO | 说明 |
|------|:----:|------|
| SDA | 41 | I2C 数据线 |
| SCL | 42 | I2C 时钟线 |

#### Audio I2S（ES8311 全双工）

| 信号 | GPIO | 说明 |
|------|:----:|------|
| MCLK | 39 | I2S 主时钟 |
| BCLK (SCLK) | 38 | I2S 位时钟 |
| WS (LRCK) | 47 | I2S 字选择 |
| DOUT (DAC) | 48 | I2S 数据输出（播放） |
| DIN (ADC) | 40 | I2S 数据输入（录音） |
| ES8311 I2C 地址 | 0x30 | 编解码器 I2C 地址 |

#### Flash SPI（模块内部，仅供参考）

| 信号 | GPIO | 说明 |
|------|:----:|------|
| CS | 29 | 片选 |
| SCLK | 30 | SPI 时钟 |
| SO (MISO) | 31 | 主入从出 |
| SI (MOSI) | 32 | 主出从入 |
| HOLD | 33 | 保持 |
| WP | 34 | 写保护 |

#### 其他外设

| 信号 | GPIO | 说明 |
|------|:----:|------|
| RGB LED (WS2812) | 18 | 可寻址 RGB LED |
| UART0 TX | 43 | USB 串口发送 → CH340K RXD |
| UART0 RX | 44 | USB 串口接收 ← CH340K TXD |
| 扩展串口 TXD | 20 | PH2.0 扩展串口 |
| 扩展串口 RXD | 19 | PH2.0 扩展串口 |
| BOOT 按键 | 0 | 上电时按下进入下载模式 |
| EN (CHIP_PU) | NC | 由底板 CH340K 一键下载电路控制 |

### 12.2 串口命令参考表

#### WiFi 配置

| 指令 | 说明 | 示例 | 返回值 |
|------|------|------|--------|
| `sta_ssid:名称` | 设置 WiFi 名称 | `sta_ssid:MyWiFi` | `OK` |
| `sta_pd:密码` | 设置 WiFi 密码 | `sta_pd:12345678` | `OK`（重启） |
| `ap_ssid:名称` | 设置热点名称 | `ap_ssid:NanoCam-AP` | `OK` |
| `ap_pd:密码` | 设置热点密码 | `ap_pd:12345678` | `OK`（重启） |
| `wifi_mode:X` | 切换模式 (0=AP, 1=STA, 2=AP+STA) | `wifi_mode:1` | `OK`（变化时重启） |

#### AI 模式

| 指令 | 模式 | 说明 | 重启 |
|------|:--:|------|:--:|
| `ai_mode:0` | 图传 | MJPEG 推流，无 AI | ✅ |
| `ai_mode:1` | 猫脸 | 实时猫脸画框 + 置信度 | ✅ |
| `ai_mode:2` | 人脸 | 实时人脸画框 + 10 关键点坐标 | ✅ |
| `ai_mode:3` | 颜色 | 框选录入 → 实时 9 色检测 | ✅ |
| `ai_mode:4` | 面容 ID | 注册 → 辨认 → 删除 | ✅ |
| `ai_mode:5` | 二维码 | 实时解码 → 串口输出 | ✅ |
| `ai_mode:6` | 运动 | 帧差法运动侦测 | ✅ |
| `ai_mode:7` | 智能体 | LLM 语音 + MCP 工具 | ✅ |

#### 信息查询

| 指令 | 说明 | 返回值示例 |
|------|------|-----------|
| `sta_ip` | 查询 STA IP | `sta_ip:192.168.1.100` |
| `ap_ip` | 查询 AP IP | `ap_ip:192.168.4.1` |
| `wifi_ver` | 查询固件版本 | `NanoCam Board Ver:0.2.0` |
| `nano_info` | 完整设备信息 (JSON) | 见 9.1 节 |

#### 系统控制

| 指令 | 说明 | 返回值 |
|------|------|--------|
| `wifi_reset` | 恢复出厂设置 | `Reset_OK`（重启） |
| `nano_reboot` | 软复位 | `Rebooting...` |
| `nano_snap` | 拍照存储到 SPIFFS | `OK /spiffs/photo_0001.jpg` |

#### LLM 服务器配置

| 指令 | 说明 | 示例 |
|------|------|------|
| `nano_server:URL` | 设置 LLM 服务器地址 | `nano_server:https://api.openai.com` |
| `nano_api_key:KEY` | 设置 API 密钥 | `nano_api_key:sk-xxxxxxxx` |

#### 虚拟按键（颜色识别 / 人脸识别模式下使用）

| 指令 | 颜色识别 | 人脸识别 |
|------|---------|---------|
| `KEY_MENU` | 录入/检测模式切换 | 保存当前检测到的人脸（ENROLL） |
| `KEY_PLAY` | 确定录入 / 删除颜色 | 辨认当前人脸（RECOGNIZE） |
| `KEY_UPUP` | 增加检测框范围 | 回到检测模式（DETECT） |
| `KEY_DOWN` | 减小检测框范围 | 删除最后一个人脸（DELETE） |

#### 扩展指令（部分规划中）

| 指令 | 说明 | 状态 |
|------|------|:--:|
| `nano_mqtt:broker,port,topic` | 配置 MQTT 服务器 | 🔨 |
| `nano_ota:url` | OTA 远程升级 | 📋 |
| `nano_led:R,G,B` | 设置 RGB LED | 📋 |
| `nano_stream:on/off` | 启停图传 | 📋 |

### 12.3 常见问题 FAQ

#### 烧录问题

**Q: 烧录失败 / 连接超时**

A: 请按以下步骤排查：
1. Type-C 线是否支持数据传输（非仅充电线）。
2. 按住 BOOT 键（GPIO0）再上电，进入手动下载模式。
3. 检查 CH340K 驱动是否安装（设备管理器无感叹号）。
4. 尝试降低波特率。

**Q: 烧录后串口无输出**

A: 确认波特率为 115200，数据位 8，无校验，停止位 1。换用 PuTTY 或 MobaXterm 等工具测试。

#### WiFi 问题

**Q: 连不上 WiFi**

A:
1. 检查 WiFi 名称和密码是否正确（区分大小写）。
2. **不支持 5GHz WiFi，仅支持 2.4GHz**。
3. 发送 `wifi_reset` 恢复出厂后重试。
4. 确保 WiFi 名称和密码无中文、不超过 30 字符。

**Q: STA 连不上，AP 模式能用吗？**

A: 可以。NanoCam 自带 AP 热点 `NanoCam-AP`（默认密码 `12345678`），手机/电脑连上后浏览器打开 `http://192.168.4.1`。

**Q: 怎么看 IP 地址？**

A: 串口发送 `sta_ip`，返回 `sta_ip:192.168.x.x`。

#### 摄像头问题

**Q: 摄像头不出画面**

A:
1. 检查 FPC 排线是否插紧（金属触点朝下）。
2. 串口日志是否有 `[OK] Camera`。
3. 确认摄像头型号与 menuconfig 设置一致（出厂默认 GC2145）。
4. 画面翻转：串口发 `@vflip:1@` 或 `@mirror:1@`。

**Q: 画面卡顿 / 帧率低**

A: WiFi 信号弱会降低帧率。AI 处理模式下帧率比纯图传模式低是正常现象。可以靠近路由器，或减少同时观看的客户端数量。

#### AI 模式问题

**Q: 切换模式后没反应**

A: 切换模式后 NanoCam 会自动重启，等待 2~3 秒。串口会打印 `OK (rebooting to mode X)`。

**Q: 人脸识别模式（模式 4）串口失灵**

A: 模式 4 占用内存较大，Type-C 串口配置功能可能失效。请使用 PH2.0 扩展串口或 I2C 通信。

**Q: 猫脸 / 人脸检测不准确**

A: 确保光线充足，人脸正对摄像头。建议检测距离 30cm ~ 1m。

#### 硬件问题

**Q: NanoCam 发热**

A: ESP32-S3 + 摄像头 + WiFi 同时工作会有一定发热，属正常现象。如果过热请检查供电是否稳定（5V/1A 以上）。

**Q: 能用其他摄像头吗？**

A: 支持 DVP 接口的 GC2145（默认）、OV2640、OV5640 等。更换后在 `menuconfig` → `NanoCam Configuration` 中选择对应型号并重新编译。

**Q: LED 不亮**

A: 核心板上的 LED 是电源指示灯，底板供电正常就会亮。WS2812 RGB LED 需在固件中主动控制。

#### 开发问题

**Q: Flash 大小不匹配**

A: 确保 `menuconfig` → `Serial flasher config` → `Flash size` 设置为 16 MB，且分区表使用 `partitions/16m.csv`。

**Q: 怎么添加新的 AI 模式？**

A: 在 `main/ai/` 目录下新增模块，实现 `register_xxx()` 函数，然后在 [app_main.cpp](file:///d:/esp32-cam/ESP32-NanoCam/main/app_main.cpp) 的 `switch(ai_mode)` 中添加分支。

**Q: 支持 OTA 升级吗？**

A: 支持。通过串口发送 `nano_ota:http://your-server/firmware.bin` 即可无线升级。分区表已包含 `ota_0` 和 `ota_1` 双分区，支持签名校验和自动回滚。

### 12.4 相关资源

| 资源 | 链接 |
|------|------|
| ESP-DL | [github.com/espressif/esp-dl](https://github.com/espressif/esp-dl) |
| ESP-WHO | [github.com/espressif/esp-who](https://github.com/espressif/esp-who) |
| XiaoZhi AI | [github.com/78/xiaozhi-esp32](https://github.com/78/xiaozhi-esp32) |
| ESP-Claw | [github.com/espressif/esp-claw](https://github.com/espressif/esp-claw) |
| ESP32-S3 TRM | [技术参考手册](https://www.espressif.com/sites/default/files/documentation/esp32-s3_technical_reference_manual_en.pdf) |
| 项目文档索引 | [docs/README.md](file:///d:/esp32-cam/ESP32-NanoCam/docs/README.md) |
| 串口协议手册 | [docs/串口协议手册.md](file:///d:/esp32-cam/ESP32-NanoCam/docs/串口协议手册.md) |
| MCP 协议手册 | [docs/MCP协议手册.md](file:///d:/esp32-cam/ESP32-NanoCam/docs/MCP协议手册.md) |
| ROS2 集成指南 | [docs/ROS2集成指南.md](file:///d:/esp32-cam/ESP32-NanoCam/docs/ROS2集成指南.md) |
| FAQ | [docs/FAQ.md](file:///d:/esp32-cam/ESP32-NanoCam/docs/FAQ.md) |
| 硬件规格书 | [docs/硬件规格书.md](file:///d:/esp32-cam/ESP32-NanoCam/docs/硬件规格书.md) |
| Git 仓库 | [git@github.com:Juxi-Technology/ESP32-NanoCam.git](git@github.com:Juxi-Technology/ESP32-NanoCam.git) |

---

> **文档版本**：v0.2.0
>
> **维护者**：Juxi-Technology
>
> 如有问题，请查阅 [FAQ](file:///d:/esp32-cam/ESP32-NanoCam/docs/FAQ.md) 或提交 [GitHub Issue](https://github.com/Juxi-Technology/ESP32-NanoCam/issues)。
