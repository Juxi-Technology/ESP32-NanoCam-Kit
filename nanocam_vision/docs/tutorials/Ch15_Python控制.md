# 第15章: Python 控制 NanoCam

> 难度: ★★☆ | 产出: Python 上位机/Jupyter 交互

## 原理

Python 通过 `pyserial` 或 `nanocam` 库发送 AT 指令控制 NanoCam。

## 步骤

### 15.1 pyserial 串口通信
```python
import serial
ser = serial.Serial("COM3", 115200, timeout=2)
ser.write(b"ai_mode:1

")
print(ser.readline().decode())
```

### 15.2 NanoCam Python 库
```python
from nanocam import NanoCam
cam = NanoCam("COM3")
cam.set_ai_mode(1)           # 猫脸检测
info = cam.get_info()         # 设备信息 JSON
print(info["ai_mode"])
```

### 15.3 Jupyter Notebook 交互
逐行运行代码,实时查看效果:
```python
cam.set_ai_mode(2)           # 人脸检测
# 浏览器打开 http://<IP> 看效果
```

### 15.4 MJPEG 流解码
```python
import cv2
cap = cv2.VideoCapture("http://192.168.1.100:80/stream")
while True:
    ret, frame = cap.read()
    cv2.imshow("NanoCam", frame)
    if cv2.waitKey(1) & 0xFF == ord('q'): break
```

## 下一步
→ [第16章: STM32控制](Ch16_STM32控制.md)
