# Chapter 2: Quick Start

## 2.1 Firmware Flashing

### Steps

1. Unzip the folder → `nanocam_xxx.bin`
2. Open [esptool-js](https://espressif.github.io/esptool-js/)
3. Connect the NanoCam via Type-C
4. Click Connect → select the serial port
5. Select the firmware file, enter `0x0` as the address
6. Click START → wait for completion

### Verification

Connect the NanoCam with a serial terminal (115200 8N1); you should see:

```
NanoCam Board Ver:0.3.0
```

---

## 2.2 WiFi Provisioning

> Output: **NanoCam connects to WiFi and gets an IP**

### Method A: Serial Provisioning (Most Common)

```
sta_ssid:your WiFi name
sta_pd:your WiFi password
```

Receiving `OK` → setup successful. The device reboots automatically after the password is changed.

### Method B: Direct AP Hotspot Connection

The NanoCam has a built-in hotspot: `NanoCam-AP`, password `12345678`
After connecting your phone, open `http://192.168.4.1` in a browser

### Verification

```
sta_ip
```

Returns: `sta_ip:192.168.x.x` ✅

---

## 2.3 First Frame

> Output: **NanoCam's live image visible in the browser**
1. Enter `http://<IP address>` in your browser
2. See the live MJPEG image
3. Send `ai_mode:1` over serial → switch to cat face detection → detection boxes appear on the image

### Endpoint Description

| URL | Purpose |
|---|---|
| `http://<IP>/` | Live image (HTML) |
| `http://<IP>/stream` | Raw MJPEG stream (readable by OpenCV/VLC) |
| `http://<IP>/status` | Device status JSON |
| `http://<IP>/admin` | Web admin console |
