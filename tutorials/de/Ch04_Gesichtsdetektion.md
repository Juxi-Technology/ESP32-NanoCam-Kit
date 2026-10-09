# Kapitel 4: Gesichtsdetektion

## Funktionsprinzip

Die Gesichtsdetektion verwendet die ESP-DL-Bibliothek für Deep Learning mit dem leichten MobileNet-Detektionsmodell. Eingabe ist ein 320x240-RGB565-Bild; ausgegeben wird eine Liste von Gesichtsrahmen (Position + Größe + Konfidenz). Die Inferenz läuft direkt auf dem ESP32-S3, ohne Netzwerkverbindung.

### Format der Detektionsergebnisse

- Koordinaten: obere linke Ecke (x,y) + Breite/Höhe (w,h)
- Konfidenz: Gleitkommazahl zwischen 0 und 1
- Bei mehreren Gesichtern werden mehrere Rahmen zurückgegeben

## Schritte

### 4.1 Modus wechseln

```
ai_mode:2
```

### 4.2 Ergebnis beobachten

Im Browser unter `http://<IP>` erscheint der Gesichtsdetektionsrahmen.

### 4.3 Koordinaten abrufen

Format der seriellen Ausgabe:

```
I (xxxxx) detection_result: [ 0]: ( 45,  30, 180, 210)
I (xxxxx) detection_result:       left eye: ( 90,  80), right eye: (150,  80), nose: (120, 120), mouth left: ( 95, 150), mouth right: (145, 150)
```

- Erste Zeile: `[Nummer] (x, y, w, h)` — Koordinaten des Gesichtsrahmens
- Zweite Zeile: 5 Landmarken — linkes Auge, rechtes Auge, Nase, linker Mundwinkel, rechter Mundwinkel

## Code

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

### Auslesen mit Python

```python
ser = serial.Serial("COM3", 115200)
line = ser.readline().decode()
if line.startswith("$face:"):
    parts = line[6:-1].split(",")
    x, y, w, h = map(int, parts)
```

## Ergebnis

Ein Gesicht erscheint vor der Kamera → grüner Rahmen im Bild → Koordinaten werden über die serielle Schnittstelle ausgegeben.
