# Chapter 4: Face Detection

## Principle

Face detection uses the ESP-DL deep learning library, based on a lightweight MobileNet detection model. Input: a 320x240 RGB565 image; output: a list of face bounding boxes (position + size + confidence). Inference runs on the ESP32-S3 itself, with no network connection required.

### Detection Result Format

- Coordinates: top-left corner (x,y) + width and height (w,h)
- Confidence: a floating-point number between 0 and 1
- Multiple boxes are returned when there are multiple faces

## Steps

### 4.1 Switching the Mode

```
ai_mode:2
```

### 4.2 Observing the Effect

Open `http://<IP>` in a browser to see face detection boxes.

### 4.3 Getting the Coordinates

Serial output format:

```
I (xxxxx) detection_result: [ 0]: ( 45,  30, 180, 210)
I (xxxxx) detection_result:       left eye: ( 90,  80), right eye: (150,  80), nose: (120, 120), mouth left: ( 95, 150), mouth right: (145, 150)
```

- First line: `[index] (x, y, w, h)` — face box coordinates
- Second line: 5 keypoints — left eye, right eye, nose, mouth left, mouth right

## Code

### Arduino: Reading Coordinates to Control a Servo

```cpp
// Parse the $face:x,y,w,h# format
if (nanoSerial.available()) {
    String line = nanoSerial.readStringUntil('\n');
    if (line.startsWith("$face:")) {
        int x = line.substring(6).toInt();
        int y = line.substring(line.indexOf(',')+1).toInt();
        servoX.write(map(x, 0, 320, 0, 180));
    }
}
```

### Python Reading

```python
ser = serial.Serial("COM3", 115200)
line = ser.readline().decode()
if line.startswith("$face:"):
    parts = line[6:-1].split(",")
    x, y, w, h = map(int, parts)
```

## Result

A face appears in front of the camera → a green box is drawn on the image → coordinates are output over serial.
