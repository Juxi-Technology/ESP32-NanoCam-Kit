# nanocam_vision — ESP32-NanoCam Main Firmware

ESP-IDF firmware for the ESP32-S3 dual-mode AI camera: **edge vision AI (offline)** + **cloud LLM voice agents (online)**.

- Edge AI: human face detection / face recognition / cat face detection / color detection / QR code scanning / motion detection, with MJPEG live streaming
- Agents: xiaozhi AI voice chat (ASR → LLM → TTS) and the ESP-Claw visual agent (photo capture + multimodal LLM understanding)
- Interfaces: HTTP (streaming + web admin console), UART AT commands, I2C slave, MQTT, micro-ROS 2, WS2812 RGB LED

## Quick Start

Requires [ESP-IDF](https://docs.espressif.com/projects/esp-idf/) v5.4+ (v5.4.1 recommended):

```bash
cd nanocam_vision
idf.py set-target esp32s3
idf.py build
idf.py -p <PORT> flash monitor
```

> The first build fetches `esp32-camera` via the IDF Component Manager (network required). All other components (esp-dl / micro-ROS / esp-code-scanner) are bundled in this directory.

## Documentation

- [Documentation index](docs/README.md) (28 documents + a 28-chapter tutorial series, in Chinese)
- [Quick start](docs/快速开始.md) · [User guide](docs/USER_GUIDE.md) · [UART protocol](docs/串口协议手册.md) · [FAQ](docs/FAQ.md)

## License

MIT License (software) + CC BY 4.0 (hardware & documentation). See [LICENSE](LICENSE). Third-party components: see [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).

---

## 中文说明

基于 ESP-IDF 的 ESP32-S3 双模态 AI 相机固件：**边缘 AI 视觉（离线）** + **云端大模型语音智能体（联网）**。

- 边缘 AI：人脸检测 / 人脸识别 / 猫脸检测 / 颜色识别 / 二维码扫描 / 运动检测，MJPEG 实时图传
- 智能体：小智 AI 语音对话（ASR → LLM → TTS）、ESP-Claw 视觉 Agent（拍照 + 多模态 LLM 理解）
- 接口：HTTP（图传 + Web 管理后台）、串口 AT 指令、I2C 从机、MQTT、micro-ROS 2、WS2812 RGB

编译需要 ESP-IDF v5.4+（推荐 v5.4.1），进入本目录执行 `idf.py set-target esp32s3 && idf.py build`。首次构建会通过 IDF Component Manager 自动拉取 `esp32-camera`（需要网络）；其余组件（esp-dl / micro-ROS / esp-code-scanner）已随本目录分发。

文档入口：[文档索引](docs/README.md)（28 篇文档 + 28 章递进教程）。许可：MIT（软件）+ CC BY 4.0（硬件/文档），详见 [LICENSE](LICENSE)。
