# 第14章: Arduino 控制 NanoCam

> 难度: ★★☆ | 产出: Arduino 主控的AI视觉系统

## 原理

Arduino 通过串口发送 AT 指令控制 NanoCam,通过 I2C 读取 AI 检测结果。

## 步骤

### 14.1 串口 AT 指令驱动
```cpp
#include <SoftwareSerial.h>
SoftwareSerial nano(2, 3);  // RX, TX

void setup() {
    nano.begin(115200);
    nano.print("ai_mode:2
");  // 人脸检测
}
```

### 14.2 NanoCam Arduino 库
```cpp
#include <NanoCam.h>
NanoCam cam(Serial1);

void setup() {
    cam.begin();
    cam.setWifi("MyWiFi", "password");
    cam.setAIMode(NANOCAM_HUMAN_FACE);
}
```

### 14.3 读取 AI 结果
I2C 寄存器读取或串口解析 `$face:` 格式。

## 代码

代码位置 `libraries/Arduino/examples/` (3 个例程):
- `BasicStream` — WiFi 配网+图传
- `AIModeSwitch` — 按钮切换 7 种 AI 模式
- `FaceTracking` — 人脸追踪舵机云台

## 下一步
→ [第15章: Python控制](Ch15_Python控制.md)
