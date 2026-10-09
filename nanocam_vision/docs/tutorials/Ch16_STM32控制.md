# 第16章: STM32 控制 NanoCam

> 难度: ★★★ | 产出: STM32 主控的AI视觉系统

## 原理

STM32 通过 UART 发送 AT 指令,通过 I2C 读取检测结果。

## 代码

### HAL 串口发送 AT 指令
```c
#include "nanocam_stm32.h"
NanoCam_STM32 cam(&huart1);

void setup() {
    cam.init();
    cam.setWiFi("MyWiFi", "password");
    cam.setAIMode(NANOCAM_HUMAN_FACE);
}
```

### 解析返回数据
```c
char ip[32];
cam.getSTAIP(ip, sizeof(ip));
```

### 中断接收
```c
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    // 非阻塞接收 AI 检测结果
    if (rx_buf[len-1] == '#') {
        parseDetection(rx_buf);
    }
}
```

## 下一步
→ [第17章: 树莓派控制](Ch17_树莓派控制.md)
