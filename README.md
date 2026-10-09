# ESP32-NanoCam

ESP32-S3 双模态 AI 摄像头模组 —— 边缘视觉 AI（离线）+ 云端大模型语音智能体（联网）。由深圳市钜犀科技有限责任公司（Juxi Technology）设计开发。

本仓库包含三个固件项目及配套资料：

## 目录说明

### nanocam_vision —— 主固件（推荐）

基于 ESP-IDF 的完整功能固件：

- **边缘 AI 视觉（离线）**：人脸检测 / 人脸识别 / 猫脸检测 / 颜色识别 / 二维码扫描 / 运动检测，MJPEG 实时图传
- **云端智能体（联网）**：小智 AI 语音对话（ASR → LLM → TTS）、ESP-Claw 视觉 Agent（拍照 + 多模态 LLM 理解）
- **对外接口**：HTTP 图传 / Web 管理后台、串口 AT 指令、I2C 从机、MQTT、micro-ROS 2、WS2812 RGB
- **配套资料**：`docs/`（28 篇文档 + 28 章递进教程）、`libraries/`（Arduino / Python / STM32 / micro:bit / 树莓派 / Pico 跨平台 SDK）

编译方法见 [`nanocam_vision/README.md`](nanocam_vision/README.md)（需要 ESP-IDF v5.4+）。

### nanocam_xiaozhi —— 小智 AI 固件

基于虾哥的开源项目 [xiaozhi-esp32](https://github.com/78/xiaozhi-esp32)（MIT 许可）适配的小智 AI 语音助手固件。

### nanocam_espclaw —— ESP-Claw 固件

同样基于 xiaozhi-esp32（MIT 许可），新增 NanoCam S3CAM 板卡支持（`bread-compact-wifi-s3cam`），用于 ESP-Claw 硬件。

## 许可

- 软件：NanoCam Non-Commercial License v1.0（非商业用途免费，商用请联系 pm@juxitech.com）
- 硬件与文档：CC BY-NC 4.0

详见 [`nanocam_vision/LICENSE`](nanocam_vision/LICENSE)；`nanocam_xiaozhi/` 与 `nanocam_espclaw/` 内保留上游 MIT 许可证；第三方开源组件归属见 [`nanocam_vision/THIRD_PARTY_NOTICES.md`](nanocam_vision/THIRD_PARTY_NOTICES.md)。
