# ESP32-NanoCam I2C 从机协议手册

> 地址: 0x33 | 速率: 100kHz (标准) / 400kHz (快速)
>
> 兼容 NanoCam I2C 寄存器映射

---

## 一、硬件连接

| NanoCam 引脚 | 外部 MCU |
|-------------|---------|
| SDA (IO41) | SDA |
| SCL (IO42) | SCL |
| GND | GND |

> NanoCam 核心板 P1-5=SDA, P1-6=SCL

## 二、寄存器表

### 控制寄存器 (Write)

| 地址 | 名称 | 说明 | 值 |
|------|------|------|----|
| 0x01 | HFLIP | 水平翻转 | 0=关 1=开 |
| 0x02 | VFLIP | 垂直翻转 | 0=关 1=开 |
| 0x03 | MODE_SELECT | 切换 AI 模式 | 0=Stream 1=Cat 2=Face 3=Color 4=FaceID 5=QR 6=LLM 7=ESP-Claw |
| 0x04 | MODE_RESET | 模块复位 | 写入任意值 |
| 0x08 | VIRTUAL_KEY | 虚拟按键 | 1=MENU 2=PLAY 3=UP 4=DOWN |

### 检测结果寄存器 (Read)

| 地址 | 名称 | 格式 | 说明 |
|------|------|------|------|
| 0x20-0x21 | LX | uint16 BE | 左上角 X |
| 0x22-0x23 | LY | uint16 BE | 左上角 Y |
| 0x24-0x25 | RX | uint16 BE | 右下角 X |
| 0x26-0x27 | RY | uint16 BE | 右下角 Y |
| 0x28-0x29 | MX | uint16 BE | 中心点 X |
| 0x2A-0x2B | MY | uint16 BE | 中心点 Y |
| 0x2C-0x2D | ID | int16 BE | 识别 ID (人脸/颜色) |
| 0x2E-0x2F | AREA | uint16 BE | 框面积 |

## 三、Arduino 示例

```cpp
#include <Wire.h>

void setup() {
    Wire.begin();        // NanoCam is slave at 0x33
    Serial.begin(115200);
}

void loop() {
    // Read detection center X (registers 0x28-0x29)
    Wire.beginTransmission(0x33);
    Wire.write(0x28);    // register address
    Wire.endTransmission(false);

    Wire.requestFrom(0x33, 2);
    if (Wire.available() == 2) {
        int mx = (Wire.read() << 8) | Wire.read();
        Serial.print("Center X: ");
        Serial.println(mx);
    }

    // Switch to cat face mode
    // Wire.beginTransmission(0x33);
    // Wire.write(0x03);  // mode select register
    // Wire.write(1);      // mode 1 = cat face
    // Wire.endTransmission();

    delay(100);
}
```

## 四、Python (树莓派) 示例

```python
from smbus2 import SMBus

bus = SMBus(1)  # /dev/i2c-1 on Raspberry Pi

# Read detection center X
mx = (bus.read_byte_data(0x33, 0x28) << 8) | bus.read_byte_data(0x33, 0x29)
print(f"Center X: {mx}")

# Switch to cat face mode (mode 1)
bus.write_byte_data(0x33, 0x03, 1)
```

## 五、STM32 HAL 示例

```c
uint8_t reg = 0x28;       // center X register
uint8_t data[2];
HAL_I2C_Master_Transmit(&hi2c1, 0x33 << 1, &reg, 1, 100);
HAL_I2C_Master_Receive(&hi2c1, 0x33 << 1, data, 2, 100);
int16_t mx = (data[0] << 8) | data[1];
```