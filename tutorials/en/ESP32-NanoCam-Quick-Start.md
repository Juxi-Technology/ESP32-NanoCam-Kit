# ESP32-NanoCam Quick Start Guide

---

## Prerequisites

- NanoCam core board + base board (ESP32-S3 N16R8 + CH340K)
- USB Type-C data cable (data transfer supported)
- Computer (Windows / Mac / Linux)
- GC2145 camera module (connected at the factory)

---

## Step 1: Flash the Firmware (3 minutes)

### Method A: No Development Environment (Recommended)

1. Open your browser and visit [esptool-js](https://espressif.github.io/esptool-js/)
2. Connect the NanoCam to your computer with a Type-C cable
3. Select the serial port, baud rate 115200
4. Find the firmware file `nanocam_xxx.bin` inside the extracted archive
5. Select the firmware file `nanocam_xxx.bin`, address `0x0`
6. Click "START" and wait for completion

### Method B: Command Line (Advanced)

```bash
pip install esptool
esptool.py --chip esp32s3 --port COM3 write_flash 0x0 nanocam.bin
```

---

## Step 2: Connect to WiFi (2 minutes)

By default the NanoCam runs **AP+STA dual mode simultaneously** — no switching required:
- The **AP hotspot** is always on: connect your phone directly to `NanoCam-AP` (password `12345678`), then open `http://192.168.4.1` in a browser
- **STA to a router**: configure the WiFi once
Connect to the NanoCam's Type-C port with a serial tool (baud rate **115200 8N1**):

```
sta_ssid:your WiFi name
sta_pd:your WiFi password
```

> Receiving `OK` means the configuration succeeded. The device reboots automatically after the password is changed.
To switch WiFi modes (usually not necessary):

| Command | Mode | Description |
|---|---|---|
| `wifi_mode:0` | AP only | Disables STA, keeps only the hotspot |
| `wifi_mode:1` | STA only | Disables the hotspot, connects only to the router |
| `wifi_mode:2` | AP+STA | Default; both work simultaneously |

---

## Step 3: View the Video (1 minute)

1. Send `sta_ip` over serial to get the STA IP
2. Enter `http://<IP address>` in your browser (or `http://192.168.4.1` in AP mode)
3. The web page shows the live video

---

## Step 4: Explore the AI (2 minutes)

Send the following commands over serial to switch modes:

| Command | Mode | Effect |
|---|---|---|
| `ai_mode:0` | Normal streaming | Real-time MJPEG video |
| `ai_mode:1` | Cat face detection | Cat face detection boxes appear on screen |
| `ai_mode:2` | Face detection | Face detection boxes appear on screen |
| `ai_mode:3` | Color recognition | Select a color → real-time tracking |
| `ai_mode:4` | Face recognition | Enroll → identify → delete |
| `ai_mode:5` | QR code scanning | Aim at a QR code → content output to serial |
| `ai_mode:6` | LLM agent | Voice wake-up "你好小智" (XiaoZhi AI) |
| `ai_mode:7` | ESP-Claw | ESP-Claw AI Agent (official Espressif framework) |

> Each mode switch requires a manual reboot — press the module's RST button to reboot; the new mode takes effect after the restart.

---

## Step 5: Integrate Into Your Project

### Arduino Control

```cpp
Serial.begin(115200);
Serial.print("ai_mode:2");  // Switch to face detection
```

### Python Control

```python
import serial
ser = serial.Serial("COM3", 115200)
ser.write(b"ai_mode:1\r\n")  # Switch to cat face detection
```

### View the Complete Command List

→ Serial Protocol Manual

---

## FAQ

| Problem | Solution |
|---|---|
| Flashing fails | Check that the Type-C cable supports data; hold S2(BOOT) on the base board while powering on |
| No video | Send `sta_ip` over serial to confirm the IP; check that both devices are on the same subnet |
| Camera stays dark | Check that the FPC cable is fully seated with the metal contacts facing down; check PWDN(IO12)/RESET(IO14) |
| Cannot connect to WiFi | Send `wifi_reset` to restore factory settings, then configure again |

More questions → [FAQ](https://FAQ.md)

---

## Next Steps

- 📖 Serial Protocol Manual — complete AT command reference
- 🎓 Tutorial Outline — 28-chapter progressive tutorial
- 🔧 Hardware Spec — complete GPIO pin mapping
- 🤖 ROS2 Integration Guide — micro-ROS tutorial
