# nanocam_vision — ESP32-NanoCam 主固件

基于 ESP-IDF 的 ESP32-S3 双模态 AI 相机固件：**边缘 AI 视觉（离线）** + **云端大模型语音智能体（联网）**。

- 边缘 AI：人脸检测 / 人脸识别 / 猫脸检测 / 颜色识别 / 二维码扫描 / 运动检测，MJPEG 实时图传
- 智能体：小智 AI 语音对话（ASR → LLM → TTS）、ESP-Claw 视觉 Agent（拍照 + 多模态 LLM 理解）
- 接口：HTTP（图传 + Web 管理后台）、串口 AT 指令、I2C 从机、MQTT、micro-ROS 2、WS2812 RGB

## 快速开始

需要 [ESP-IDF](https://docs.espressif.com/projects/esp-idf/) v5.4+（推荐 v5.4.1）：

```bash
cd nanocam_vision
idf.py set-target esp32s3
idf.py build
idf.py -p <串口> flash monitor
```

> 首次构建会通过 IDF Component Manager 自动拉取 `esp32-camera`（需要网络）。其余组件（esp-dl / micro-ROS / esp-code-scanner）已随本目录分发。

## 文档

- [文档索引](docs/README.md)（28 篇文档 + 28 章递进教程）
- [快速开始](docs/快速开始.md) · [用户手册](docs/USER_GUIDE.md) · [串口协议](docs/串口协议手册.md) · [FAQ](docs/FAQ.md)

## 许可

NanoCam Non-Commercial License v1.0 + 硬件/文档 CC BY-NC 4.0，详见 [LICENSE](LICENSE)。第三方组件见 [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md)。
