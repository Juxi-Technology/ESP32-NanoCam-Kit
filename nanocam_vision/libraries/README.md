# NanoCam Cross-Platform SDK

Client libraries and examples for ESP32-NanoCam. They talk to the module over **UART AT commands** or the **I2C slave** interface, so they work with any host MCU / platform.

> Protocol details: [UART protocol manual](../docs/串口协议手册.md) and [I2C protocol manual](../docs/I2C协议手册.md) (in Chinese).

## Contents

| Directory | Platform | Content |
|-----------|----------|---------|
| `Arduino/` | Arduino | `NanoCam` library (`NanoCam.h`) + 5 examples (AI mode switch / serial stream / face tracking / I2C test / UART test) |
| `Python/` | PC (Python ≥3.7) | `nanocam` package (`pip install ./Python`) + QuickStart notebook |
| `STM32/` | STM32 | `nanocam_stm32.h` UART driver header |
| `Microbit/` | micro:bit | MakeCode extension (`nanocam.ts`) + servo gimbal firmware (`microbit-wifi_supkit.hex`) |
| `Examples/` | All | `Arduino_WiFi` (WiFi provisioning), `Car_Control` (4WD car / Pico car / STM32 BSP), `Pico_Car`, `RaspberryPi` (RPi / Jetson WiFi setup scripts) |

## Quick Usage

**Arduino**: copy `Arduino/` into your Arduino `libraries/` folder, then open an example from `Arduino/examples/`:

```cpp
#include <NanoCam.h>
NanoCam cam(Serial1);

void setup() {
    cam.begin();
    cam.setAIMode(NANOCAM_HUMAN_FACE);  // human face detection mode
}
```

**Python**:

```bash
pip install ./Python
```

```python
from nanocam import NanoCam
cam = NanoCam("/dev/ttyUSB0")
cam.set_ai_mode(2)  # human face detection
```

**Car / gimbal control**: see `Examples/Car_Control/README.md` and tutorial [Chapter 18](../docs/tutorials/Ch18_小车与云台控制.md) (in Chinese).

## License

MIT, same as the project. See [LICENSE](../LICENSE).

---

## 中文说明

本目录包含 ESP32-NanoCam 的跨平台客户端库与示例，通过**串口 AT 指令**或 **I2C 从机**协议与 NanoCam 模组通信，适用于任意主控平台。

协议细节见 [串口协议手册](../docs/串口协议手册.md) 与 [I2C 协议手册](../docs/I2C协议手册.md)。

| 目录 | 平台 | 内容 |
|------|------|------|
| `Arduino/` | Arduino | `NanoCam` 库（`NanoCam.h`）+ 5 个例程（AI 模式切换 / 串口流 / 人脸追踪 / I2C 测试 / UART 测试） |
| `Python/` | PC（Python ≥3.7） | `nanocam` 包（`pip install ./Python`）+ QuickStart Notebook |
| `STM32/` | STM32 | `nanocam_stm32.h` 串口驱动头文件 |
| `Microbit/` | micro:bit | MakeCode 扩展（`nanocam.ts`）+ 舵机云台固件（`microbit-wifi_supkit.hex`） |
| `Examples/` | 综合 | `Arduino_WiFi`（透传配网）、`Car_Control`（4WD 小车 / Pico 小车 / STM32 BSP）、`Pico_Car`、`RaspberryPi`（树莓派 / Jetson 配网脚本） |

小车 / 云台控制示例见 `Examples/Car_Control/README.md` 与教程 [第18章 小车与云台控制](../docs/tutorials/Ch18_小车与云台控制.md)。许可：MIT，详见 [LICENSE](../LICENSE)。
