# Kapitel 5: Katzengesichtsdetektion

## Funktionsprinzip

Die Katzengesichtsdetektion verwendet das Modell CatFaceDetectMN03, das speziell auf Katzen-Gesichtsmerkmale (dreieckige Ohren / großer Augenabstand / Nase) optimiert trainiert wurde. Eingabe ist ein 320x240-RGB565-Bild; ausgegeben wird eine Liste von Katzengesichtsrahmen. Mit der Gesichtsdetektion aus Kapitel 4 teilt es sich dasselbe Ausgabeformat von `print_detection_result`.

### Modellunterschiede: Katzen- vs. Gesichtsdetektion

| Merkmal | Gesichtsdetektion (ai_mode:2) | Katzengesichtsdetektion (ai_mode:1) |
|---|---|---|
| Modell | MSR01 + MNP01 (zweistufige Kaskade) | CatFaceDetectMN03 (einstufig) |
| Landmarken | 10 (Augen/Nasenspitze/Mundwinkel) | Keine (Modell gibt keine aus) |
| Konfidenzschwelle | MSR01=0.3, MNP01=0.4 | 0.4 |
| Rahmendarstellung | Grünes hohles Rechteck + 5 Landmarken | Grünes hohles Rechteck (ohne Landmarken) |

## Schritte

### 5.1 Modus wechseln

```
ai_mode:1
```

Das Gerät startet automatisch neu und wechselt in den Katzengesichtsdetektionsmodus

### 5.2 Ergebnis beobachten

Legen Sie eine Katze oder ein Katzenbild vor die Kamera; im Browser unter `http://<IP>` erscheint ein grüner Rahmen um das Katzengesicht.

### 5.3 Serielle Ausgabe

Bei erkannter Katze gibt die serielle Schnittstelle aus:

```
I (xxxxx) detection_result: [ 0]: ( 45,  30, 180, 210)
```

- Format: `[Nummer] (x, y, w, h)` — Koordinaten der oberen linken Ecke des Katzengesichtsrahmens + Breite/Höhe
- Das Katzenmodell gibt keine Landmarken aus (anders als die Gesichtsdetektion)

## Code

### Zentrale Detektionslogik

`components/modules/ai/who_cat_face_detection.cpp`:

```cpp
CatFaceDetectMN03 detector(0.4F, 0.3F, 10, 0.3F);
std::list<dl::detect::result_t> &detect_results = detector.infer(
    (uint16_t *)frame->buf, {(int)frame->height, (int)frame->width, 3});

if (detect_results.size() > 0) {
    draw_detection_result((uint16_t *)frame->buf, frame->height, frame->width, detect_results);
    print_detection_result(detect_results);  // Koordinaten über die serielle Schnittstelle ausgeben
}
```

### Arduino: Koordinaten auslesen und Servo steuern

```cpp
// Format $face:x,y,w,h# parsen
if (nanoSerial.available()) {
    String line = nanoSerial.readStringUntil('\n');
    if (line.startsWith("$face:")) {
        int x = line.substring(6).toInt();
        int y = line.substring(line.indexOf(',')+1).toInt();
        servoX.write(map(x, 0, 320, 0, 180));
    }
}
```

### Serielles Auslesen mit Python

```python
import serial
ser = serial.Serial("COM3", 115200)
while True:
    line = ser.readline().decode().strip()
    if "detection_result" in line:
        print(line)
```

## Ergebnis

Die Katze erscheint → grüner Rahmen im Bild → Koordinaten werden seriell ausgegeben. Mit Arduino/Python können die Koordinaten ausgelesen und ein Servo zur Verfolgung angesteuert werden.
