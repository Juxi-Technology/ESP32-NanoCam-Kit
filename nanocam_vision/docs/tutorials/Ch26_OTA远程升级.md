# 第26章: OTA 远程升级

> 难度: ★★★ | 产出: 无线更新固件

## 原理

ESP-IDF 原生支持 OTA(Over-The-Air)升级。NanoCam 从 HTTP 服务器下载新固件,校验后写入 ota_0/ota_1 分区,重启后切换。

## 使用

```
nano_ota:http://192.168.1.100:8080/nanocam_v0.3.0.bin
```

## 安全

- HTTPS 支持(TLS 证书)
- 固件签名校验
- 回滚机制: 新固件启动失败自动回退到上一个版本

## 分区表支持

NanoCam 的 16MB Flash 分区已预留 ota_0(3MB) 和 ota_1(3MB) 两个 OTA 槽位。

## 下一步
→ [第27章: 自定义MCP工具](Ch27_自定义MCP工具.md)
