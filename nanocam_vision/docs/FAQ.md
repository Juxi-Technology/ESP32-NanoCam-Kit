# ESP32-NanoCam 常见问题 (FAQ)

---

## 烧录问题

### Q: 烧录失败 / 连接超时
A: 
1. Type-C 线是否支持数据传输（非仅充电线）
2. 按住 BOOT 键再上电，进入下载模式
3. 检查 CH340K 驱动是否安装
4. 尝试降低波特率

### Q: 烧录后串口无输出
A: 波特率是否设为 115200？换一个串口工具试试。PuTTY / MobaXterm / Arduino 串口监视器均可。

---

## WiFi 问题

### Q: 连不上 WiFi
A: 
1. 检查 WiFi 名称和密码是否正确（区分大小写）
2. 不支持 5GHz WiFi，仅支持 2.4GHz
3. `wifi_reset` 恢复出厂后重试
4. 确保 WiFi 名称和密码无中文、不超过 30 字符

### Q: 怎么看 IP 地址？
A: 串口发送 `sta_ip`，返回 `sta_ip:192.168.x.x`

### Q: STA 连不上，AP 模式能用吗？
A: 可以。NanoCam 自带 AP 热点 `NanoCam-AP`（密码 `12345678`），手机连上后打开 `http://192.168.4.1`

---

## 摄像头问题

### Q: 摄像头不出画面
A: 
1. FPC 排线是否插紧（金属触点朝下）
2. 串口日志是否有 `Camera init OK`
3. 确认摄像头型号（出厂默认 GC2145）
4. 画面反了？串口发 `@vflip:1@` 或 `@mirror:1@`

### Q: 画面卡顿 / 帧率低
A: 
1. WiFi 信号弱 → 靠近路由器
2. 多设备同时看 → 减少客户端
3. AI 模式帧率低于纯图传模式

---

## AI 模式问题

### Q: 切换模式后没反应
A: 切换模式后 NanoCam 会自动重启，等待 2-3 秒。串口会打印 `OK (rebooting to mode X)`

### Q: 人脸识别模式串口失灵
A: 模式 4（人脸识别）占用内存较大，Type-C 串口可能失效。使用 PH2.0 串口或 I2C 切换模式。

### Q: 猫脸 / 人脸检测不准确
A: 确保光线充足，人脸正对摄像头。检测距离建议 30cm-1m。

---

## 硬件问题

### Q: NanoCam 发热
A: ESP32-S3 + 摄像头 + WiFi 同时工作会有一定发热，正常。如过热请检查供电。

### Q: 能用其他摄像头吗？
A: 支持 DVP 接口的 GC2145(默认) / OV2640 / OV5640 等。换摄像头后在 `menuconfig` → `NanoCam Configuration` 中选择对应型号。

### Q: LED 不亮
A: 核心板上的 LED 是电源指示灯，底板供电正常就会亮。

---

## 开发问题

### Q: 怎么编译固件？
A: 
1. 安装 ESP-IDF 5.4+
2. `idf.py set-target esp32s3`
3. `idf.py build`

### Q: 怎么添加新的 AI 模式？
A: 在 `main/ai/` 目录下新增模块，实现 `register_xxx()` 函数，然后在 `app_main.cpp` 的 switch 中添加分支。

### Q: 串口指令格式是什么？
A: 见 [串口协议手册](串口协议手册.md)

---

## 其他

### Q: 支持 OTA 升级吗？
A: 支持。通过串口发送 `nano_ota:http://your-server/firmware.bin` 即可无线升级。支持签名校验和自动回滚。

### Q: 有手机 App 吗？
A: 可通过浏览器访问 Web 管理后台，或使用第三方 MJPEG 查看器。

### Q: 商业授权怎么获取？
A: 联系 **pm@juxitech.com**
