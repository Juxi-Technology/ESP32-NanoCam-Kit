# Kapitel 6: Farberkennung

## Funktionsprinzip

Grundlage ist der HSV-Farbraum (Farbton-Sättigung-Helligkeit). Das von der Kamera ausgegebene RGB565-Bild wird von der ColorDetector-Engine aus esp-dl verarbeitet: Das Bild wird zur Rauschreduzierung auf 80×80 skaliert, Pixel für Pixel in HSV-Werte umgerechnet und mit den 7 voreingestellten Farbschwellen abgeglichen.

### Voreingestellte Farbschwellen (OpenCV-Standard-H-Bereich, Skala 0-180)

| Farbe | Farbton (H) | Sättigung (S) | Helligkeit (V) | Mindestfläche |
|---|---|---|---|---|
| Rot | 0-15 | 70-255 | 90-255 | 64 |
| Gelb | 23-33 | 70-255 | 90-255 | 64 |
| Grün | 34-75 | 70-255 | 90-255 | 64 |
| Blau | 97-124 | 70-255 | 90-255 | 64 |
| Lila | 125-155 | 70-255 | 90-255 | 64 |
| Weiß | 0-180 | 0-40 | 200-255 | 80 |
| Schwarz | 0-180 | 0-255 | 0-50 | 80 |

> Für den Farbton gilt die OpenCV-Skala 0-180 (entspricht 0-360°). `set_bgr(false)` stellt sicher, dass die Bibliothek die RGB565-Daten unverändert liest und die Kanäle nicht vertauscht.

## Schritte

### 6.1 Farbmodus aktivieren

```
ai_mode:3
```

Das Gerät startet automatisch neu in den Farberkennungsmodus; die WS2812-RGB-LED (GPIO18) zeigt die aktuell erkannte Farbe an.

### 6.2 Erkennungsergebnisse beobachten

Legen Sie ein einfarbiges Objekt vor die Kamera; im Browser unter `http://<IP>` erscheint:
- **Farbiger rechteckiger Rahmen** markiert den erkannten Farbbereich
- **Farbetikett-Text** (red/yellow/green/blue/purple/white/black)
- Rahmen- und Etikettfarbe entsprechen der tatsächlich erkannten Farbe
> Der Farbmodus führt nur eine Bildüberlagerung (OSD) durch und gibt keine seriellen Logs aus. Koordinaten müssen über die I2C-Register gelesen werden.

### 6.3 Detektionsdaten über I2C lesen

NanoCam fungiert als I2C-Slave (Adresse `0x33`, GPIO SDA=41 SCL=42) und aktualisiert die Mittelpunktkoordinaten des Detektionsrahmens in Echtzeit.

| Register | Inhalt | Datentyp |
|---|---|---|
| 0x28-0x29 | Mittelpunkt X | int16 BE |
| 0x2A-0x2B | Mittelpunkt Y | int16 BE |
| 0x2C-0x2D | Erkennungs-ID | int16 BE |

## Code

### Zentrale Erkennungs-Engine

`components/modules/ai/who_color_detection.cpp` — basierend auf dem esp-dl ColorDetector:

```cpp
// Detektor aufbauen; set_bgr(false) stellt die korrekten Farbkanäle sicher
ColorDetector detector;
detector.set_bgr(false);
detector.set_detection_shape({80, 80, 1});
// 7 Farbschwellen registrieren
detector.register_color({h_lo, h_hi, s_lo, s_hi, v_lo, v_hi}, area_min, "red");
// Erkennung
auto &results = detector.detect((uint16_t *)frame->buf,
    {(int)frame->height, (int)frame->width, 3});

// Ergebnisse durchlaufen, Rahmen + Etikett zeichnen
for (int ci = 0; ci < (int)results.size(); ci++) {
    for (int ri = 0; ri < (int)results[ci].size(); ri++) {
        color_detect_result_t &res = results[ci][ri];
        draw_rect(frame, res.box[0], res.box[1], res.box[2], res.box[3], color_lcd);
        fb_gfx_print(frame, lx, ly, color_lcd, color_name);
    }
}
```

## Ergebnis

Objekte in Rot/Grün/Blau → Farberkennung → Rahmen + Etikett → Koordinaten über I2C → nutzbar für Servo-Sortierung.
