# Chapter 7: QR Code Scanning

## Principle

Uses the precompiled esp-code-scanner library to decode QR codes (QR Code / Barcode) in the image in real time. The RGB565 frames output by the camera are passed directly to the scanner — no grayscale conversion is needed. A brand-new scanner object is created for every frame and destroyed as soon as the scan finishes, preventing internal state from accumulating.
The decoded result is delivered through:
1. **Serial log** output
2. The **shared buffer** `g_last_code` stores the latest result for overlay display on the HTTP/MJPEG stream
3. A green text label overlaid at the **bottom of the web image**

## Steps

### 7.1 Switching the Mode

```
ai_mode:5
```

### 7.2 Scanning a Code

Place a QR code in front of the camera; the serial port outputs:

```
I (xxxxx) qrcode: Decoded [QR-Code]: https://example.com
```

At the same time, green text showing the decoded content appears at the bottom of the web image at `http://<IP>/`.

### 7.3 Continuous Scanning

Aim at the next code and it is decoded and output automatically; the scanner is rebuilt on every frame, so it can keep working continuously without crashing.

## Code

### Core Scanning Logic

`main/ai/nano_qrcode.cpp`:

```cpp
// Create a brand-new scanner object for every frame
esp_image_scanner_t *scn = esp_code_scanner_create();
esp_code_scanner_config_t cfg = {
    ESP_CODE_SCANNER_MODE_FAST,
    ESP_CODE_SCANNER_IMAGE_RGB565,
    fb->width, fb->height
};
esp_code_scanner_set_config(scn, cfg);
int count = esp_code_scanner_scan_image(scn, fb->buf);
// Decoding succeeded
const esp_code_scanner_symbol_t result = esp_code_scanner_result(scn);
ESP_LOGI(TAG, "Decoded [%s]: %s", result.type_name, result.data);
// Save to the shared buffer for web overlay
snprintf(g_last_code, sizeof(g_last_code), "%s: %s",
         result.type_name, result.data);
esp_code_scanner_destroy(scn);
```

## Result

Aim at a QR code → the decoded content is output over serial + overlaid on the web image.
