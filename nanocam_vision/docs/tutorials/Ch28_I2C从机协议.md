# 第28章: I2C 从机协议开发

> 难度: ★★★★ | 产出: 外部MCU直接读取AI结果

## 原理

NanoCam 作为 I2C 从机(地址 0x33),外部 MCU 通过 I2C 总线读取检测结果寄存器或写入控制命令。

## 寄存器表

| 地址 | 内容 | 格式 |
|------|------|------|
| 0x20-0x21 | 左上角 X | uint16 BE |
| 0x22-0x23 | 左上角 Y | uint16 BE |
| 0x24-0x25 | 右下角 X | uint16 BE |
| 0x26-0x27 | 右下角 Y | uint16 BE |
| 0x28-0x29 | 中心点 X | uint16 BE |
| 0x2A-0x2B | 中心点 Y | uint16 BE |
| 0x2C-0x2D | 识别 ID | int16 BE |
| 0x03 | 模式选择(写) | 0-6 |

## 代码

### Arduino 读取
```cpp
Wire.beginTransmission(0x33);
Wire.write(0x28);  // 中心点 X
Wire.endTransmission(false);
Wire.requestFrom(0x33, 2);
int16_t mx = (Wire.read() << 8) | Wire.read();
```

### STM32 读取
```c
uint8_t reg = 0x28;
uint8_t data[2];
HAL_I2C_Master_Transmit(&hi2c1, 0x33<<1, &reg, 1, 100);
HAL_I2C_Master_Receive(&hi2c1, 0x33<<1, data, 2, 100);
int16_t mx = (data[0] << 8) | data[1];
```

## 完整协议

→ [I2C 协议手册](../I2C协议手册.md)
