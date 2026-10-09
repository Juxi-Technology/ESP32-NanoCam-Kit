# Chapter 6: Color Recognition

## Principle

Based on the HSV (Hue-Saturation-Value) color space. The RGB565 image output by the camera is processed by esp-dl's ColorDetector engine: the image is scaled down to 80×80 resolution to reduce noise, then converted pixel by pixel into HSV values, which are matched against 7 preset color thresholds.

### Preset Color Thresholds (OpenCV standard H range, 0-180 scale)

| Color | Hue (H) | Saturation (S) | Value (V) | Area threshold |
|---|---|---|---|---|
| Red | 0-15 | 70-255 | 90-255 | 64 |
| Yellow | 23-33 | 70-255 | 90-255 | 64 |
| Green | 34-75 | 70-255 | 90-255 | 64 |
| Blue | 97-124 | 70-255 | 90-255 | 64 |
| Purple | 125-155 | 70-255 | 90-255 | 64 |
| White | 0-180 | 0-40 | 200-255 | 80 |
| Black | 0-180 | 0-255 | 0-50 | 80 |

> Hue uses the OpenCV 0-180 scale (corresponding to 0-360°). `set_bgr(false)` ensures the library reads the RGB565 data as-is without swapping channels.

## Steps

### 6.1 Entering Color Mode

```
ai_mode:3
```

The device reboots automatically into color detection mode, and the WS2812 RGB LED (GPIO18) displays the currently recognized color.

### 6.2 Observing the Recognition Result

Put a solid-colored object in front of the camera; open `http://<IP>` in a browser and you will see:
- A **colored rectangle** marking the detected color region
- A **color label text** (red/yellow/green/blue/purple/white/black)
- The box and label colors match the actually detected color
> Color mode only does image overlay (OSD) and does not output serial logs. To get the coordinates, read them through the I2C registers.

### 6.3 Reading Detection Data over I2C

The NanoCam acts as an I2C slave (address `0x33`, GPIO SDA=41 SCL=42) and updates the detection box center coordinates in real time.

| Register | Content | Data type |
|---|---|---|
| 0x28-0x29 | Center X | int16 BE |
| 0x2A-0x2B | Center Y | int16 BE |
| 0x2C-0x2D | Recognition ID | int16 BE |

## Code

### Core Detection Engine

`components/modules/ai/who_color_detection.cpp` — based on esp-dl ColorDetector:

```cpp
// Build the detector; set_bgr(false) keeps the color channels correct
ColorDetector detector;
detector.set_bgr(false);
detector.set_detection_shape({80, 80, 1});
// Register the 7 color thresholds
detector.register_color({h_lo, h_hi, s_lo, s_hi, v_lo, v_hi}, area_min, "red");
// Detect
auto &results = detector.detect((uint16_t *)frame->buf,
    {(int)frame->height, (int)frame->width, 3});

// Iterate over the results, drawing boxes + labels
for (int ci = 0; ci < (int)results.size(); ci++) {
    for (int ri = 0; ri < (int)results[ci].size(); ri++) {
        color_detect_result_t &res = results[ci][ri];
        draw_rect(frame, res.box[0], res.box[1], res.box[2], res.box[3], color_lcd);
        fb_gfx_print(frame, lx, ly, color_lcd, color_name);
    }
}
```

## Result

Red/green/blue objects → color recognized → box + label drawn → coordinates output over I2C → can be connected to a servo for sorting.
