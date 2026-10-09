# 第25章: MQTT 物联网集成

> 难度: ★★★ | 产出: 云端 AI 检测 Dashboard

## 原理

NanoCam 内置 MQTT 客户端,检测到 AI 事件时自动发布 JSON 消息到指定 Topic。

## 配置

```
nano_mqtt:192.168.1.100,1883,nanocam/events
```

## 消息格式

```json
{"type":"cat","x":120,"y":80,"w":100,"h":120}
{"type":"face","x":60,"y":40,"w":80,"h":100}
{"type":"motion","timestamp":1234567890}
```

## 云平台接入

### EMQX (开源,本地部署)
```bash
docker run -d --name emqx -p 1883:1883 -p 8083:8083 emqx/emqx
```

### 阿里云 IoT / ThingsBoard
配置相应的 MQTT broker 地址和认证信息。

## 下一步
→ [第26章: OTA远程升级](Ch26_OTA远程升级.md)
