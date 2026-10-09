# 第24章: Web管理后台

> 难度: ★★☆ | 产出: 手机浏览器控制 NanoCam

## 原理

NanoCam 将 Web 前端文件存入 SPIFFS,HTTP 服务器直接提供页面。页面通过 `/api?cmd=xxx` 发送 AT 指令,通过 `/stream` 显示实时画面。

## 功能

- **实时预览**: 嵌入 MJPEG 流
- **AI 模式切换**: 7个模式按钮一键切换
- **WiFi 配网**: 网页输入 SSID 和密码
- **设备信息**: 版本/内存/状态面板

## 部署

1. 将 `data/web_admin/` 内容写入 SPIFFS:
```bash
idf.py build flash spiffs
```
2. 浏览器打开 `http://<IP>/admin`

## 下一步
→ [第25章: MQTT物联网集成](Ch25_MQTT物联网集成.md)
