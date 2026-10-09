# Chapter 5: Cat Face Detection

## Principle

Cat face detection uses the CatFaceDetectMN03 model, trained specifically for cat facial features (triangular ears / wide eye spacing / nose). Input: a 320x240 RGB565 image; output: a list of cat face bounding boxes. It shares the same `print_detection_result` output format with Chapter 4's face detection.

### Cat Face vs. Face Detection Model Differences

| Comparison | Face detection (ai_mode:2) | Cat face detection (ai_mode:1) |
|---|---|---|
| Model | MSR01 + MNP01 two-stage cascade | CatFaceDetectMN03 single-stage |
| Keypoints | 10 (both eyes / nose tip / mouth corners) | None (the model does not output them) |
| Confidence threshold | MSR01=0.3, MNP01=0.4 | 0.4 |
| Detection box drawing | Green hollow rectangle + 5 keypoints | Green hollow rectangle (no keypoints) |

## Steps

### 5.1 Switching the Mode

```
ai_mode:1
```

The device reboots automatically into cat face detection mode

### 5.2 Observing the Effect

Put a cat or a picture of a cat in front of the camera; open `http://<IP>` in a browser to see a green detection box marking the cat's face.

### 5.3 Serial Output

When a cat face is detected, the serial port outputs:

```
I (xxxxx) detection_result: [ 0]: ( 45,  30, 180, 210)
```

- Format: `[index] (x, y, w, h)` — top-left corner coordinates of the cat face box + width and height
- The cat face model does not output keypoints (unlike face detection)

## Code

### Core Detection Logic

`components/modules/ai/who_cat_face_detection.cpp`:

```cpp
CatFaceDetectMN03 detector(0.4F, 0.3F, 10, 0.3F);
std::list<dl::detect::result_t> &detect_results = detector.infer(
    (uint16_t *)frame->buf, {(int)frame->height, (int)frame->width, 3});

if (detect_results.size() > 0) {
    draw_detection_result((uint16_t *)frame->buf, frame->height, frame->width, detect_results);
    print_detection_result(detect_results);  // Output coordinates over serial
}
```

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

### Python Serial Reading

```python
import serial
ser = serial.Serial("COM3", 115200)
while True:
    line = ser.readline().decode().strip()
    if "detection_result" in line:
        print(line)
```

## Result

A cat appears → a green box is drawn on the image → coordinates are output over serial. You can read the coordinates with Arduino/Python to drive a servo and track it.
