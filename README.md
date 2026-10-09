# ESP32-NanoCam

**ESP32-S3 dual-mode AI camera module** — on-device edge vision AI (offline) + cloud LLM voice agents (online).

Designed and developed by Juxi Technology (深圳市钜犀科技有限责任公司).

This repository contains three firmware projects and their supporting resources:

## Directory Overview

### nanocam_vision — Main firmware (recommended)

The full-featured ESP-IDF firmware:

- **Edge vision AI (offline)**: human face detection / face recognition / cat face detection / color detection / QR code scanning / motion detection, with MJPEG live streaming
- **Cloud agents (online)**: xiaozhi AI voice chat (ASR → LLM → TTS) and the ESP-Claw visual agent (photo capture + multimodal LLM understanding)
- **Interfaces**: HTTP streaming / web admin console, UART AT commands, I2C slave, MQTT, micro-ROS 2, WS2812 RGB LED
- **Supporting resources**: `docs/` (28 documents + a 28-chapter tutorial series) and `libraries/` (cross-platform SDKs for Arduino / Python / STM32 / micro:bit / Raspberry Pi / Pico)

Build instructions: see [`nanocam_vision/README.md`](nanocam_vision/README.md) (requires ESP-IDF v5.4+).

### nanocam_xiaozhi — xiaozhi AI voice firmware

Adapted from the open-source project [xiaozhi-esp32](https://github.com/78/xiaozhi-esp32) (MIT): a xiaozhi AI voice assistant firmware for NanoCam hardware.

### nanocam_espclaw — ESP-Claw firmware

Also based on xiaozhi-esp32 (MIT), with added support for the NanoCam S3CAM board (`bread-compact-wifi-s3cam`), for ESP-Claw hardware.

## License

Fully open source:

- **Software** (firmware / source code): **MIT License** — free to use, modify, and distribute, including commercially, as long as the copyright notice is retained
- **Hardware & documentation** (schematics / docs / images): **CC BY 4.0**

See [`nanocam_vision/LICENSE`](nanocam_vision/LICENSE). `nanocam_xiaozhi/` and `nanocam_espclaw/` retain the upstream MIT license. Third-party open-source components are listed in [`nanocam_vision/THIRD_PARTY_NOTICES.md`](nanocam_vision/THIRD_PARTY_NOTICES.md).

---

## 中文说明

ESP32-S3 双模态 AI 摄像头模组 —— 边缘视觉 AI（离线）+ 云端大模型语音智能体（联网）。由深圳市钜犀科技有限责任公司（Juxi Technology）设计开发。

本仓库包含三个固件项目及配套资料：

- **`nanocam_vision`（主固件，推荐）**：基于 ESP-IDF 的完整功能固件。边缘 AI 视觉（人脸检测 / 人脸识别 / 猫脸检测 / 颜色识别 / 二维码扫描 / 运动检测）+ MJPEG 实时图传；云端智能体（小智 AI 语音对话、ESP-Claw 视觉 Agent）；接口含 HTTP 图传 / Web 管理后台、串口 AT 指令、I2C 从机、MQTT、micro-ROS 2、WS2812；配套 `docs/`（28 篇文档 + 28 章教程）与 `libraries/`（Arduino / Python / STM32 / micro:bit / 树莓派 / Pico 跨平台 SDK）。编译方法见 [`nanocam_vision/README.md`](nanocam_vision/README.md)（需要 ESP-IDF v5.4+）。
- **`nanocam_xiaozhi`**：基于开源项目 [xiaozhi-esp32](https://github.com/78/xiaozhi-esp32)（MIT）适配的小智 AI 语音助手固件。
- **`nanocam_espclaw`**：同样基于 xiaozhi-esp32（MIT），新增 NanoCam S3CAM 板卡支持（`bread-compact-wifi-s3cam`），用于 ESP-Claw 硬件。

许可：本项目完全开源 —— 软件 MIT（可自由使用、修改、商用，只需保留版权声明），硬件与文档 CC BY 4.0，详见 [`nanocam_vision/LICENSE`](nanocam_vision/LICENSE)；第三方组件归属见 [`nanocam_vision/THIRD_PARTY_NOTICES.md`](nanocam_vision/THIRD_PARTY_NOTICES.md)。
