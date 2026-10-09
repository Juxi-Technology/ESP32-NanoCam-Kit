# NanoCam 上位机 / 外设端 SDK

本目录包含 ESP32-NanoCam 的跨平台客户端库与示例，通过**串口 AT 指令**或 **I2C 从机**协议与 NanoCam 模组通信，适用于任意主控平台。

> 协议细节见 [串口协议手册](../docs/串口协议手册.md) 与 [I2C 协议手册](../docs/I2C协议手册.md)。

## 目录说明

| 目录 | 平台 | 内容 |
|------|------|------|
| `Arduino/` | Arduino | `NanoCam` 库（`NanoCam.h`）+ 5 个例程（AI 模式切换 / 串口流 / 人脸追踪 / I2C 测试 / UART 测试） |
| `Python/` | PC（Python ≥3.7） | `nanocam` 包（`pip install ./Python`）+ QuickStart Notebook |
| `STM32/` | STM32 | `nanocam_stm32.h` 串口驱动头文件 |
| `Microbit/` | micro:bit | MakeCode 扩展（`nanocam.ts`）+ 舵机云台固件（`microbit-wifi_supkit.hex`） |
| `Examples/` | 综合 | `Arduino_WiFi`（透传配网）、`Car_Control`（4WD 小车 / Pico 小车 / STM32 BSP）、`Pico_Car`、`RaspberryPi`（树莓派 / Jetson 配网脚本） |

## 快速使用

**Arduino**：复制 `Arduino/` 到 Arduino `libraries/` 目录，打开 `Arduino/examples/` 下的例程：

```cpp
#include <NanoCam.h>
NanoCam cam(Serial1);

void setup() {
    cam.begin();
    cam.setAIMode(NANOCAM_HUMAN_FACE);  // 人脸检测模式
}
```

**Python**：

```bash
pip install ./Python
```

```python
from nanocam import NanoCam
cam = NanoCam("/dev/ttyUSB0")
cam.set_ai_mode(2)  # 人脸检测
```

**小车 / 云台控制**：见 `Examples/Car_Control/README.md` 与教程 [第18章 小车与云台控制](../docs/tutorials/Ch18_小车与云台控制.md)。

## 许可

与项目整体一致：MIT License 开源，可自由使用、修改与商用，详见 [LICENSE](../LICENSE)。
