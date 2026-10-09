# 第19章: STM32 小车 + PID 电机控制

> 难度: ★★★★ | 产出: STM32 主控 + NanoCam 视觉 + PID 巡线

## 原理

STM32 通过串口 AT 指令控制 NanoCam AI 模式，通过 I2C 读取检测坐标。PID 算法根据坐标偏差控制电机差速，实现精准巡线和跟踪。

## BSP 文件结构

```
STM32_BSP/
├── APP/esp32_wifi.c/h     # NanoCam WiFi 通信
├── APP/app_motor.c/h       # 电机调度
├── APP/PID_Motor.c/h       # PID 速度闭环
├── BSP/motor/bsp_motor.c/h # PWM 驱动
├── BSP/motor/bsp_encoder.c/h # 编码器读取
├── BSP/servo/bsp_servo.c/h # 舵机云台
└── USER/main.c            # 主控逻辑
```

## 实战: PID 巡线小车

1. NanoCam 巡线模式检测黑线位置
2. STM32 读取 X 坐标偏差
3. PID 输出 = Kp*偏差 + Ki*积分 + Kd*微分
4. 左右轮差速 = 目标速度 ± PID 输出

## 下一步
→ [第20章: micro-ROS 图传](Ch20_microROS图传.md)
