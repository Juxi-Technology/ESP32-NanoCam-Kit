# NanoCam 小车与云台控制示例

本目录包含多个平台使用 NanoCam 控制小车和舵机云台的完整代码。

## 目录结构

| 目录 | 平台 | 功能 |
|------|------|------|
| `Arduino_4WD/` | Arduino UNO | 4WD 小车控制 (无云台/带云台) |
| `Pico_Car/` | Raspberry Pi Pico | 小车 + 舵机云台 |
| `STM32_BSP/` | STM32F103 | WiFi 通信 + 电机驱动 + 舵机云台 |

## 工作原理

```
NanoCam (AI检测) ──串口──→ MCU (Arduino/STM32/Pico) ──→ 电机/舵机
        │                        │
   人脸检测/颜色识别          解析坐标→PID控制→PWM输出
```

## Arduino 使用

```cpp
#include <NanoCam.h>
NanoCam cam(Serial1);

void setup() {
    cam.begin();
    cam.setAIMode(NANOCAM_HUMAN_FACE);  // 人脸追踪模式
}

void loop() {
    // 读取 AI 检测坐标，控制舵机云台跟踪
}
```

## STM32 使用

参考 `STM32_BSP/APP/esp32_wifi.c` — 通过串口 AT 指令控制 NanoCam，通过 PID 控制电机。

## Pico 使用

```python
from nanocam import NanoCam
cam = NanoCam("/dev/ttyS0")
cam.set_ai_mode(2)  # 人脸检测
```
