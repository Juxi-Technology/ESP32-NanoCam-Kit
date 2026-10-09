# Chapter 3: Camera Basics

## Principle

The NanoCam connects to the camera through the DVP (Digital Video Port) interface. The GC2145 sensor outputs 8-bit parallel pixel data; the ESP32-S3's LCD_CAM peripheral stores it directly into PSRAM via DMA, and the HTTP server then streams it to the browser in MJPEG format.

### Key Concepts

- **DVP**: 8 parallel data lines + 3 synchronization signals (VSYNC/HREF/PCLK)
- **MJPEG**: each frame is an independent JPEG image; the browser loads them continuously to create a video effect
- **PSRAM**: 8MB PSRAM used as the frame buffer, holding 2-4 frames

## Steps

### 3.1 Viewing the Default Image

After flashing, the firmware is in streaming mode by default; open `http://<IP>` in a browser to see the image.

| Command | Function |
|---|---|
| ai_mode:0\r | Video transmission module |
| ai_mode:1\r | Cat face detection |
| ai_mode:2\r | Face detection |
| ai_mode:3\r | Color recognition |
| ai_mode:4\r | Face recognition |
| ai_mode:5\r | QR code scanning |
