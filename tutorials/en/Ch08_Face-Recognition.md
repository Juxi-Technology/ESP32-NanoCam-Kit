# Chapter 8: Face Recognition

## Principle

Face recognition = **face detection** (MSR01+MNP01 two-stage pipeline) + **feature extraction** (FaceRecognition112V1S8 MFN neural network) + **cosine similarity matching**.

```
Camera RGB565 frame
  → MSR01 coarse detection (320×240, 0.3F threshold)
  → MNP01 fine detection (based on coarse detection candidate boxes, 0.4F threshold)
  → 10 facial keypoint extraction (both eyes / nose tip / mouth corners)
  → Keypoint alignment → crop a 112×112 face
  → MFN convolutional network → 512-dimensional feature vector
  → L2 normalization
  → Compute the cosine distance against every registered ID vector in Flash one by one
  → Maximum cosine similarity > threshold (0.55) → match successful → output ID
  → All similarities < threshold → stranger → output "who?"
```

### Performance Optimization

MFN feature extraction and full-database matching are computationally heavy; running them on every frame would make the image stutter. The current implementation uses a **frame-skipping strategy**: face detection runs on every frame (cheap), while MFN recognition runs once every 10 frames (expensive), and the label keeps displaying the previous recognition result as an overlay. This keeps the image smooth and prevents the ID label from flickering.

### Face Feature Storage

Enrolled face features (id + 512-dimensional embedding) are persistently stored in the `fr` partition of Flash (96 KB, up to 47 face IDs). They are not lost when power is removed.

## Hardware Preparation

- NanoCam core board + base board
- USB-C data cable (connects to the computer for power + serial)
- Serial terminal (baud rate 115200)

## Steps

### 8.1 Entering Face Recognition Mode

```
ai_mode:4
```

The device reboots automatically into FaceID mode, and the WS2812 RGB LED (GPIO18 DIN, VDD50 powered) shows purple. After the reboot, the serial port should show:

```
I (5526) MFN: fr partition size: 98304 bytes, maxminum 47 IDs can be stored
I (5526) MFN: No face ID in flash
```

`No face ID in flash` means no face has been enrolled yet — this is normal.

### 8.2 Enrolling a Face

Have the face directly in front of the camera (distance 30-50cm, even lighting), and make sure there is **only one face** in the image. Send over serial:

```
face_eril
```

After detecting a face, the device automatically extracts its features and enrolls them to Flash:

```
I (xxxx) ENROLL: ID 1 is enrolled
```

The image overlays the blue text `Enroll: ID 1`, which disappears after about 0.5 seconds.
> **Note**: the command is `face_eril` (an abbreviation of enroll), not `face_enroll`. If you see `fail: unknown command`, check the spelling.

### 8.3 Recognizing Faces

After enrollment is complete, send the recognition command:

```
face_rz
```

The system enters continuous recognition mode. The current face is compared against all registered IDs in Flash:
- **Match successful**: the serial port outputs `Similarity: 0.85, Match ID: 1`, and a green `ID: 1` is continuously overlaid on the image
- **Stranger**: the serial port outputs `Similarity: 0.32, Match ID: 0`, and a red `who?` is continuously overlaid on the image
> The label **stays displayed** and does not disappear. To exit recognition mode, send `face_detect` to return to pure detection mode.

### 8.4 Deleting a Face

```
face_del
```

Deletes the most recently enrolled face ID; the serial port returns `N IDs left`, and the image briefly shows the number of remaining IDs. The feature in Flash is deleted at the same time.

### 8.5 Exiting Recognition Mode

```
face_detect
```

Returns to pure face detection mode (draws only boxes + keypoints, no recognition), and the ID label is cleared.
> **About DETECT mode**: on the ESP32-S3, serial coordinate printing in pure face detection mode is disabled (`#if !CONFIG_IDF_TARGET_ESP32S3`); this avoids flooding the serial port with detection logs. The `detection_result` coordinate logs are only output after entering recognition mode (`face_rz`).

## Complete Command Reference

| Command | Function | Label behavior | Persistent |
|---|---|---|---|
| `face_eril` | Enroll the currently detected face | Blue "Enroll: ID N" | Flashes for 0.5s |
| `face_rz` | Enter continuous recognition mode | Green "ID: N" / red "who?" | ✅ Persistent |
| `face_del` | Delete the most recently enrolled ID | Red "N IDs left" | Flashes for 0.5s |
| `face_detect` | Exit recognition, return to pure detection | Clears all labels | — |

## Operation Flow Example

```
ai_mode:4                          # Enter face recognition mode
[Device reboots, LED purple]

face_eril                          # Enroll the first face (Zhang San)
→ ID 1 is enrolled

face_eril                          # Enroll the second face (Li Si)
→ ID 2 is enrolled

face_rz                            # Start continuous recognition
→ Zhang San stands in front of the camera: the image continuously shows "ID: 1"
→ Li Si stands in front of the camera: the image continuously shows "ID: 2"
→ A stranger stands in front of the camera: the image continuously shows "who?"

face_detect                        # Exit recognition mode
→ The label disappears and only detection boxes are drawn

face_del                           # Delete Li Si (ID 2)
→ 1 IDs left

face_rz                            # Recognize again
→ Zhang San stands in front of the camera: "ID: 1"
→ Li Si stands in front of the camera: "who?" (already deleted)
```

> Face recognition mode uses a large amount of memory (MFN model + face detection dual models); the Type-C serial port (UART0) works normally. If the serial port does not respond, first check that the baud rate is 115200.

## Code

### Core Recognition Logic

`components/modules/ai/who_human_face_recognition.cpp` — frame-skipping recognition strategy:

```cpp
case RECOGNIZE:
{
    // Frame skipping: run MFN recognition once every 10 detections
    static int recog_skip = 0;
    if (recog_skip <= 0) {
        recognize_result = recognizer->recognize(
            (uint16_t *)frame->buf,
            {(int)frame->height, (int)frame->width, 3},
            detect_results.front().keypoint);
        recog_skip = 10;
    }
    recog_skip--;
    frame_show_state = SHOW_STATE_RECOGNIZE;
    break;
}
```

## Troubleshooting

| Symptom | Possible cause | Solution |
|---|---|---|
| `No face ID in flash` | Normal; nothing has been enrolled yet | Send `face_eril` to enroll |
| Recognition result is always `who?` | Insufficient lighting / off angle / similarity below threshold | Re-enroll, face the camera directly, ensure even lighting |
| No response when enrolling | The number of faces in the image ≠ 1 | Make sure there is only one face, at a distance of 30-50cm |
| Image stutters during recognition | Normal; MFN inference takes time | Already optimized with frame skipping — it runs once every 10 frames |
| Label flickers | — | Fixed; labels now display continuously without disappearing |
| `fail: unknown command` | Command spelling error | Check the command: `face_eril`, not `face_enroll` |

## Result

Enroll a face → continuous recognition shows the ID → results are output over I2C/serial → control a relay/servo — a complete access control solution.
