# ESP32-NanoCam Serial Protocol Manual

> Baud rate: 115200 | Data bits: 8 | Parity: None | Stop bits: 1 | Flow control: None
> Compatible with the mainstream camera module AT command set, with additional NanoCam extended commands.

---

## 1. General Rules

- Commands are **case-insensitive** (`STA_SSID` = `sta_ssid`)
- Commands must be followed by **any English punctuation mark** (`,` `.` `:` `;` etc.) as a terminator
- Some commands trigger an **automatic reboot** after being changed
- Each command ends with `\r\n` (serial assistants usually add this automatically)

## 2. WiFi Configuration

### STA Mode (connect to a router)

| Command | Description | Example | Return |
|---|---|---|---|
| `sta_ssid:name` | Set WiFi SSID | `sta_ssid:MyWiFi` | `OK` |
| `sta_pd:password` | Set WiFi password (reboots after change) | `sta_pd:12345678` | `OK` (reboots) |

> The WiFi SSID and password are at most 30 characters; Chinese characters are not supported.

### AP Mode (self-hosted hotspot)

| Command | Description | Example | Return |
|---|---|---|---|
| `ap_ssid:name` | Set hotspot SSID | `ap_ssid:NanoCam-AP` | `OK` |
| `ap_pd:password` | Set hotspot password (reboots after change) | `ap_pd:12345678` | `OK` (reboots) |

### WiFi Mode

| Command | Description | Parameter | Return |
|---|---|---|---|
| `wifi_mode:X` | Switch mode | 0=AP 1=STA 2=AP+STA | `OK` (reboots when changed) |

---

## 3. AI Mode Switching

| Command | Mode | Description | Reboot |
|---|---|---|---|
| `ai_mode:0` | Normal | MJPEG streaming, no AI | ✅ |
| `ai_mode:1` | Cat face detection | Real-time cat face boxes + confidence | ✅ |
| `ai_mode:2` | Face detection | Real-time face boxes + coordinates | ✅ |
| `ai_mode:3` | Color recognition | Box-select to enroll → real-time detection | ✅ |
| `ai_mode:4` | Face recognition | Enroll → identify → delete | ✅ |
| `ai_mode:5` | QR code | Real-time decoding → serial output | ✅ |
| `ai_mode:6` | LLM agent | XiaoZhi AI voice chat + AI vision | ✅ |
| `ai_mode:7` | ESP-Claw | Voice control + photo vision analysis + OpenAI Vision | ✅ |

> `ai_mode` valid values: 0-7. Out-of-range values fall back to 0 by default. The device reboots automatically after a change, and the new mode takes effect after the reboot.

---

## 4. Information Queries

| Command | Description | Example Return |
|---|---|---|
| `sta_ip` | Query STA IP | `sta_ip:192.168.1.100` |
| `ap_ip` | Query AP IP | `ap_ip:192.168.4.1` |
| `wifi_ver` | Query firmware version | `NanoCam Board Ver:0.2.0` |

---

## 5. System Control

| Command | Description | Return |
|---|---|---|
| `wifi_reset` | Restore factory settings (reboots) | `Reset_OK` |
| `nano_reboot` | Soft reset | `Rebooting...` |
| `nano_info` | Full device info (JSON) | See below |

### nano_info Response Example

```json
{
  "device": "NanoCam",
  "ver": "0.2.0",
  "chip": "ESP32-S3",
  "flash": "16MB",
  "psram": "8MB",
  "ai_mode": 1,
  "wifi_mode": 2,
  "sta_ip": "192.168.1.100",
  "free_heap": 245760
}
```

---

## 6. Face Recognition Commands

> Only valid in ai_mode:4 (face recognition mode).

| Command | Description | Label Behavior | Example Return |
|---|---|---|---|
| `face_eril` | Enroll the face detected in the current frame | Blue "Enroll: ID N", flashes for 0.5s | `>>> face enroll triggered` |
| `face_rz` | Enter continuous face recognition mode | Green "ID: N" / red "who?", **stays visible without disappearing** | `>>> face recognize triggered` |
| `face_del` | Delete the last enrolled face ID | Red "N IDs left", flashes for 0.5s | `>>> face delete triggered` |
| `face_detect` | Exit recognition mode, back to plain face detection | Clears all labels | `>>> face detect mode` |

### Face Recognition Workflow

```
ai_mode:4          # Enter face recognition mode (device reboots automatically)
face_eril          # Enroll the face (make sure only one face is in the frame)
face_rz            # Start continuous recognition — the label stays visible without disappearing
face_detect        # Exit recognition mode — labels are cleared
face_del           # Delete the last enrolled face
```

### Face Recognition Notes

1. When enrolling, make sure there is **only one face** in the frame, at a distance of 30-50cm
2. In recognition mode (`face_rz`) the label **stays visible** and does not disappear after 0.5s — this is new behavior in 0.3.0
3. To exit recognition mode, send `face_detect`; otherwise the label keeps showing
4. Face features are stored in the Flash `fr` partition, persist across power cycles, up to 47 IDs
5. Recognition uses frame skipping (MFN inference runs once every 10 frames)

---

## 7. Extended Commands (NanoCam-specific)

| Command | Description | Status |
|---|---|---|
| `nano_server:url` | Set LLM server address (saved in NVS) | ✅ |
| `nano_api_key:key` | Set LLM API key (saved in NVS) | ✅ |
| `nano_mqtt:broker,port,topic` | Configure MQTT server | 🔨 |
| `nano_led:R,G,B` | Set RGB LED (WS2812, GPIO18 DIN) | 📋 |
| `nano_snap` | Take a photo and store it (SPIFFS) | ✅ |
| `nano_stream:on/off` | Start/stop streaming | 📋 |

### nano_server / nano_api_key

| Command | Description | Example | Return |
|---|---|---|---|
| `nano_server:URL` | Set LLM server address | `nano_server:https://api.openai.com` | `OK server=https://api.openai.com` |
| `nano_api_key:KEY` | Set API key | `nano_api_key:sk-xxxx` | `OK` |

> Supports any OpenAI-compatible API (vLLM / Ollama / local models all work).
ESP-Claw mode (ai_mode:7) supports `nano_server`; XiaoZhi AI (ai_mode:6) uses a separate server configuration.

---

## 8. Notes

1. `sta_pd` / `ap_pd` reboot automatically after being changed; the new password takes effect after the reboot
2. `ai_mode` reboots automatically after being changed (only when the mode actually changes)
3. In face recognition mode (mode 4), Type-C serial configuration may stop working (insufficient memory)
4. WiFi SSID/password cannot exceed 30 characters and cannot contain Chinese characters
5. Commands must be followed by a punctuation mark as a terminator
